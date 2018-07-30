#include <FreeRTOS.h>
#include <task.h>
#include "hvc.h"
#include "hvc_util.h"

/*
 * @see http://components.omron.eu/getattachment/0c6ded2f-6aee-485f-a735-f9a99499c369/HVC-datasheet.pdf.aspx
 *
 * Pin Layout on HVC-P 1.2 CN4
 *
 * 1: unused
 * 2: TX
 * 3: RX
 * 4: VCC
 * 5: unused
 * 6: GND
 */

// Default to 5 retries
int hvc_read_retry = HVC_DEFAULT_READ_RETRY;

int last_response_length = 0;

static bool _hvc_run_command(char cmd, int data_size, char *data)
{
  char send_data[SEND_BUFFER_SIZE];

  send_data[0] = HVC_SYNC_CODE;
  send_data[1] = cmd;
  send_data[2] = util_lsb(data_size);
  send_data[3] = util_msb(data_size);

  hvc_log_debug("Executing command (first 4 bytes): %02x%02x%02x%02x", send_data[0], send_data[1], send_data[2], send_data[3]);

  for (int i = 0; i < data_size; i++)
  {
    send_data[4 + i] = data[i];
  }

  // Execute the command
  hvc_write_bytes(send_data, CMD_SIZE + data_size);

  // Sleep briefly to let data arrive
  vTaskDelay(100 / portTICK_RATE_MS);

  int sleep = 0;
  int avail = 0;

  // Sleep for max X seconds waiting for data.
  while (!(avail = hvc_read_bytes_available()) && sleep < hvc_read_retry)
  {
    hvc_log_debug("Sleeping HVC thread until data arrives...");
    sleep++;

    vTaskDelay(HVC_READ_RETRY_SLEEP / portTICK_RATE_MS);
  }

  // Read buffer has nothing for us, this is unexpected. The HVC
  // might be unavailable at this point.
  if (!avail)
  {
    hvc_log_error("Unable to find any data in the read buffer.");
    return false;
  }

  // Read response headers
  hvc_log_debug("Reading response header...");

  char sync_code;
  char response_code;
  char data_length_bytes[4];

  hvc_read_bytes(&sync_code, 1);
  hvc_read_bytes(&response_code, 1);

  // Not used at this point
  hvc_read_bytes(data_length_bytes, 4);

  // Set the last response length
  last_response_length =
    data_length_bytes[0] +
    (data_length_bytes[1] << 8) +
    (data_length_bytes[2] << 16) +
    (data_length_bytes[3] << 24);

  hvc_log_debug("Header sync_code: %02x", sync_code);
  hvc_log_debug("Header response_code: %02x", response_code);
  hvc_log_debug("Header data length: %d", last_response_length);

  if (sync_code != HVC_SYNC_CODE) {
    hvc_log_error("Header sync code invalid: %02x", sync_code);
    return false;
  }

  if (response_code != 0x00) {
    hvc_log_error("Header response code invalid: %02x", response_code);
    return false;
  }

  return true;
}

void hvc_set_retry(int retry)
{
  hvc_read_retry = retry;
}

struct hvc_get_version_response* hvc_get_version()
{
  if (!_hvc_run_command(HVC_CMD_GET_VERSION, 0, NULL)) return NULL;

  // Now parse
  struct hvc_get_version_response* res = (struct hvc_get_version_response*) malloc(sizeof(struct hvc_get_version_response));

  hvc_read_bytes(res->model, 12);
  hvc_read_bytes((char *) &res->major_version, 1);
  hvc_read_bytes((char *) &res->minor_version, 1);
  hvc_read_bytes((char *) &res->release_version, 1);
  hvc_read_bytes(res->revision, 4);

  return res;
}

bool hvc_set_camera_angle(char angle)
{
  // Do some validation, ensure camera angle within bounds
  if (angle > HVC_CAMERA_ANGLE_270)
  {
    hvc_log_debug("Angle out of range, setting to 0 degrees. (%d)", angle);
    angle = HVC_CAMERA_ANGLE_0;
  }

  hvc_log_info("Setting HVC camera angle -> %d", angle);

  char data[] = { angle };
  return _hvc_run_command(HVC_CMD_SET_CAMERA_ANGLE, sizeof(data), data);
}

struct hvc_get_camera_angle_response* hvc_get_camera_angle()
{
  if (!_hvc_run_command(HVC_CMD_GET_CAMERA_ANGLE, 0, NULL)) return NULL;

  struct hvc_get_camera_angle_response* res = (struct hvc_get_camera_angle_response*) malloc(sizeof(struct hvc_get_camera_angle_response));

  hvc_read_bytes(&res->angle, 1);
  return res;
}

bool hvc_set_threshold_values(int body, int hand, int face, int recognition)
{
  hvc_log_info("Setting HVC threshold values -> %d/%d/%d/%d", body, hand, face, recognition);

  char data[8];
  util_int_into_lsb_msb(data, 0, body);
  util_int_into_lsb_msb(data, 2, hand);
  util_int_into_lsb_msb(data, 4, face);
  util_int_into_lsb_msb(data, 6, recognition);

  return _hvc_run_command(HVC_CMD_SET_THRESHOLD_VALUES, sizeof(data), data);
}

struct hvc_get_threshold_values_response* hvc_get_threshold_values()
{
  if (!_hvc_run_command(HVC_CMD_GET_THRESHOLD_VALUES, 0, NULL)) return NULL;

  struct hvc_get_threshold_values_response* res = (struct hvc_get_threshold_values_response*) malloc(sizeof(struct hvc_get_threshold_values_response));

  char bytes[8];
  hvc_read_bytes(bytes, sizeof(bytes));

  res->hand = util_bytes_to_int(bytes[0], bytes[1]);
  res->body = util_bytes_to_int(bytes[2], bytes[3]);
  res->face = util_bytes_to_int(bytes[4], bytes[5]);
  res->recognition = util_bytes_to_int(bytes[6], bytes[7]);

  return res;
}

bool hvc_set_detection_size(int min_body, int max_body, int min_hand, int max_hand, int min_face, int max_face)
{
  hvc_log_info("Setting HVC detection size -> %d-%d/%d-%d/%d-%d", min_body, max_body, min_hand, max_hand, min_face, max_face);

  char data[12];

  util_int_into_lsb_msb(data, 0, min_body);
  util_int_into_lsb_msb(data, 2, max_body);
  util_int_into_lsb_msb(data, 4, min_hand);
  util_int_into_lsb_msb(data, 6, max_hand);
  util_int_into_lsb_msb(data, 8, min_face);
  util_int_into_lsb_msb(data, 10, max_face);

  return _hvc_run_command(HVC_CMD_SET_DETECTION_SIZE, sizeof(data), data);
}

struct hvc_get_detection_size_response* hvc_get_detection_size()
{
  if (!_hvc_run_command(HVC_CMD_GET_DETECTION_SIZE, 0, NULL)) return NULL;

  struct hvc_get_detection_size_response* res = (struct hvc_get_detection_size_response*) malloc(sizeof(struct hvc_get_detection_size_response));

  char bytes[12];
  hvc_read_bytes(bytes, sizeof(bytes));

  res->min_body = util_bytes_to_int(bytes[0], bytes[1]);
  res->max_body = util_bytes_to_int(bytes[2], bytes[3]);
  res->min_hand = util_bytes_to_int(bytes[4], bytes[5]);
  res->max_hand = util_bytes_to_int(bytes[6], bytes[7]);
  res->min_face = util_bytes_to_int(bytes[8], bytes[9]);
  res->max_face = util_bytes_to_int(bytes[10], bytes[11]);

  return res;
}

bool hvc_set_face_angle(char yaw, char roll)
{
  hvc_log_info("Setting HVC face angles -> yaw:%d roll:%d", yaw, roll);

  char data[2];
  data[0] = yaw;
  data[1] = roll;

  return _hvc_run_command(HVC_CMD_SET_FACE_ANGLE, sizeof(data), data);
}

struct hvc_get_face_angle_response* hvc_get_face_angle()
{
  if (!_hvc_run_command(HVC_CMD_GET_FACE_ANGLE, 0, NULL)) return NULL;

  struct hvc_get_face_angle_response* res = (struct hvc_get_face_angle_response*) malloc(sizeof(struct hvc_get_face_angle_response));

  hvc_read_bytes(&res->yaw, 1);
  hvc_read_bytes(&res->roll, 1);

  return res;
}

struct hvc_execution_response* hvc_execution(int function, int image)
{
  char data[3];
  data[0] = (function & 0xFF);
  data[1] = ((function >> 8) & 0xFF);
  data[2] = (image & 0xFF);

  if (!_hvc_run_command(HVC_CMD_EXECUTE, sizeof(data), data)) return NULL;

  struct hvc_execution_response* res = (struct hvc_execution_response*) malloc(sizeof(struct hvc_execution_response));

  // Size declaration
  int size = last_response_length;

  char header[4];
  size -= hvc_read_bytes(header, sizeof(header));

  res->body_count = header[0];
  res->hand_count = header[1];
  res->face_count = header[2];
  // header[3] reserved and unused

  // Dud char for empty reading
  char c;

  for (int i = 0; i < res->body_count * (sizeof(uint8_t) * 8); i++)
  {
    hvc_read_bytes(&c, 1); // Read into the void
    size--;
  }

  for (int i = 0; i < res->hand_count * (sizeof(uint8_t) * 8); i++)
  {
    hvc_read_bytes(&c, 1); // Read into the void
    size--;
  }

  // TODO only face detection supported for now...
  for (int i = 0; i < res->face_count * (sizeof(uint8_t) * 8); i++)
  {
    hvc_read_bytes(&c, 1); // Read into the void
    size--;
  }

  // Check if image was requested, if yes...save the
  // raw image bytes to a file for external processing.
  if (image != HVC_EX_IMAGE_NONE)
  {
    char xy[4];

    // Read the XY values, first 4 bytes of the image response
    size -= hvc_read_bytes(xy, 4);

    int width = util_bytes_to_int(xy[0], xy[1]);
    int height = util_bytes_to_int(xy[2], xy[3]);
    int image_size = width * height;

    hvc_log_info("Image Detected: %d X %d (bytes: %d)", width, height, image_size);

    // Save image to file
    const char* filepath = "/debug.img";

    FILE *fp;
    if (!(fp = fopen(filepath, "wb")))
    {
      hvc_log_error("Unable to open file path: %d", filepath);
    }

    // Write dimensions as first 4 bytes
    if (fp) fwrite(xy, 1, sizeof(xy), fp);

    // Set up image buffer
    int available = 0;
    int retry = 0;
    char c[HVC_IMAGE_READ_BUFFER];

    // Drain the rest of the buffer and save image
    while (size > 0 && retry < hvc_read_retry)
    {
      // Wait for HVC to send more data, retry a few times before
      // we decide it's pointless and just give up.
      if (!(available = hvc_read_bytes_available()))
      {
        retry++;
        continue;
      }

      // Reset the retry
      retry = 0;

      hvc_log_info("Reading available image data: %d", available);
      int image_read = hvc_read_bytes(c, HVC_IMAGE_READ_BUFFER);
      size -= image_read;

      // Flush to file if we managed to open it.
      if (fp) fwrite(c, 1, HVC_IMAGE_READ_BUFFER, fp);

      vTaskDelay(HVC_IMAGE_READ_SLEEP_MS / portTICK_RATE_MS);
    }

    // Check for missing bytes, this could've happened because of buffer overflow
    // or just slow processing.
    if (size > 0)
    {
      hvc_log_error("Missing bytes from response (%d)", size);
    }

    if (fp) fclose(fp);
  }

  return res;
}