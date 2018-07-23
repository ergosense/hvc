#include <FreeRTOS.h>
#include <task.h>
#include "hvc.h"
#include "hvc_util.h"
#include "esp_log.h"
// TODO abstract
#include "mgos.h"
#include "mgos_uart.h"


#define SEND_BUFFER_SIZE    32
#define HEADER_BUFFER_SIZE  32
#define DATA_BUFFER_SIZE    64
#define HEADER_SIZE         6
#define CMD_SIZE            4

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


static const char *TAG = "hvc";

static void _hvc_retrieve_header(struct hvc_response_header* header)
{
  ESP_LOGI(TAG, "Read response header");

  hvc_read_bytes(&header->sync_code, 1);
  hvc_read_bytes(&header->response_code, 1);
  hvc_read_bytes(header->data_length_bytes, 4);
  
  ESP_LOGI(TAG, "Header sync_code: %02x", header->sync_code);
  ESP_LOGI(TAG, "Header response_code: %02x", header->response_code);
  ESP_LOGI(TAG, "Header data_length: %d", header->data_length);
}

static void _hvc_run_command(char cmd, int data_size, char *data)
{
  char send_data[SEND_BUFFER_SIZE];
  
  send_data[0] = HVC_SYNC_CODE;
  send_data[1] = cmd;
  send_data[2] = util_lsb(data_size);
  send_data[3] = util_msb(data_size);

  ESP_LOGI(TAG, "Executing command (first 4 bytes): %02x%02x%02x%02x", send_data[0], send_data[1], send_data[2], send_data[3]);

  for (int i = 0; i < data_size; i++)
  {
    send_data[4 + i] = data[i];  
  }

  // Execute the command
  hvc_write_bytes(send_data, CMD_SIZE + data_size);

  int sleep = 0;

  // Sleep for max 5 seconds waiting for data.
  // TODO configurable please
  while (!mgos_uart_read_avail(HVC_UART_NUM) && sleep < 5)
  {
    vTaskDelay(1000 / portTICK_RATE_MS);
  }

  // TODO come on, make this better...this is meh

  // Read response headers
  struct hvc_response_header header = {};

  _hvc_retrieve_header(&header);


  if (header.sync_code != HVC_SYNC_CODE) {
    ESP_LOGE(TAG, "Header sync code invalid: %02x", header.sync_code);
    return;
  }

  if (header.response_code != 0x00) {
    ESP_LOGE(TAG, "Header response code invalid: %02x", header.response_code);
    return;  
  }
}

struct hvc_get_version_response* hvc_get_version()
{
  _hvc_run_command(HVC_CMD_GET_VERSION, 0, NULL);

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
  char data[] = { angle };
  _hvc_run_command(HVC_CMD_SET_CAMERA_ANGLE, sizeof(data), data);
  return true;
}

struct hvc_get_camera_angle_response* hvc_get_camera_angle()
{
  _hvc_run_command(HVC_CMD_GET_CAMERA_ANGLE, 0, NULL);

  struct hvc_get_camera_angle_response* res = (struct hvc_get_camera_angle_response*) malloc(sizeof(struct hvc_get_camera_angle_response));

  hvc_read_bytes(&res->angle, 1);
  return res;
}

bool hvc_set_threshold_values(int body, int hand, int face, int recognition)
{
  char data[8];
  util_int_into_lsb_msb(data, 0, body);
  util_int_into_lsb_msb(data, 2, hand);
  util_int_into_lsb_msb(data, 4, face);
  util_int_into_lsb_msb(data, 6, recognition);

  _hvc_run_command(HVC_CMD_SET_THRESHOLD_VALUES, sizeof(data), data);
  return true;
}

struct hvc_get_threshold_values_response* hvc_get_threshold_values()
{
  _hvc_run_command(HVC_CMD_GET_THRESHOLD_VALUES, 0, NULL);

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
  char data[12];

  util_int_into_lsb_msb(data, 0, min_body);
  util_int_into_lsb_msb(data, 2, max_body);
  util_int_into_lsb_msb(data, 4, min_hand);
  util_int_into_lsb_msb(data, 6, max_hand);
  util_int_into_lsb_msb(data, 8, min_face);
  util_int_into_lsb_msb(data, 10, max_face);

  _hvc_run_command(HVC_CMD_SET_DETECTION_SIZE, sizeof(data), data);
  return true; 
}

struct hvc_get_detection_size_response* hvc_get_detection_size()
{
  _hvc_run_command(HVC_CMD_GET_DETECTION_SIZE, 0, NULL);

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
  char data[2];
  data[0] = yaw;
  data[1] = roll;

  _hvc_run_command(HVC_CMD_SET_FACE_ANGLE, sizeof(data), data);
  return true;
}

struct hvc_get_face_angle_response* hvc_get_face_angle()
{
  _hvc_run_command(HVC_CMD_GET_FACE_ANGLE, 0, NULL);

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
  data[2] = (0 & 0xFF); // TODO no images for now

  _hvc_run_command(HVC_CMD_EXECUTE, sizeof(data), data);
  
  struct hvc_execution_response* res = (struct hvc_execution_response*) malloc(sizeof(struct hvc_execution_response));

  char header[4];
  hvc_read_bytes(header, sizeof(header));

  res->body_count = header[0];
  res->hand_count = header[1];
  res->face_count = header[2];
  // header[3] reserved and unused

  // Empty out the read buffer. TODO perhaps we can use flush?
  char c;

  while (mgos_uart_read_avail(HVC_UART_NUM))
  {
    hvc_read_bytes(&c, 1);
  }

  return res;
}