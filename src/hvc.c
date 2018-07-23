#include <FreeRTOS.h>
#include <task.h>
#include "hvc.h"
#include "esp_log.h"
#include "util.h"
// TODO abstract

#include "mgos.h"

#include "hvc_parser.h"

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

static void _hvc_run_command(struct hvc_command *cmd, int data_size, char *data)
{
  char send_data[SEND_BUFFER_SIZE];
  
  send_data[0] = SYNC_CODE;
  send_data[1] = cmd->cmd;
  send_data[2] = util_lsb(data_size);
  send_data[3] = util_msb(data_size);

  ESP_LOGI(TAG, "Executing command (first 4 bytes): %02x%02x%02x%02x", send_data[0], send_data[1], send_data[2], send_data[3]);

  for (int i = 0; i < data_size; i++)
  {
    send_data[4 + i] = data[i];  
  }

  hvc_write_bytes(send_data, CMD_SIZE + data_size);

  if (xQueueSend(command_queue, cmd, 0) != pdTRUE)
  {
    LOG(LL_INFO, ("Unable to queue!"));
  }
    
  /*
  if (header->sync_code != SYNC_CODE) {
    ESP_LOGE(TAG, "Header sync code invalid: %02x", header->sync_code);
    return;
  }

  if (header->response_code != 0x00) {
    ESP_LOGE(TAG, "Header response code invalid: %02x", header->response_code);
    return;  
  }
  */

  //response->populate(&header, this->port);
  //this->load_data(header, response);
}

void hvc_get_version(hvc_command_callback fn)
{
  // Create the response on the heap, this response will be freed
  // by the hvc_handle_response() code after all callbacks have been executed.

  // It is important to note that the response variable will not be available outside
  // or beyond the hvc_command_callback function.
  struct hvc_get_version_response* response = (struct hvc_get_version_response*) malloc(sizeof(struct hvc_get_version_response));

  struct hvc_command cmd = {
    HVC_CMD_GET_VERSION,
    hvc_get_version_parser,
    response,
    fn
  };

  return _hvc_run_command(&cmd, 0, NULL);
}

void hvc_handle_response()
{
  struct hvc_command cmd;

  if (xQueueReceive(command_queue, &cmd, 0) == pdTRUE)
  {
    struct hvc_response_header header = {};

    // Read headers
    _hvc_retrieve_header(&header);

    if (header.sync_code != HVC_SYNC_CODE) {
      ESP_LOGE(TAG, "Header sync code invalid: %02x", header.sync_code);
      return;
    }

    // TODO constant
    if (header.response_code != 0x00) {
      ESP_LOGE(TAG, "Header response code invalid: %02x", header.response_code);
      return;  
    }

    // Parse the object into a "known" response
    ESP_LOGI(TAG, "Run command parser...");
    cmd.parser(cmd.response);

    // Forward the object on to the requested callback
    ESP_LOGI(TAG, "Forward command response...");
    cmd.fn(cmd.response);

    // Release response resource
    ESP_LOGI(TAG, "Free response malloc");
    free(cmd.response);
    return;
  }

  LOG(LL_ERROR, ("Unable to pop command from queue"));
}

/*
struct HvcResponse HVC::setCameraAngle(char angle)
{
  HvcResponse response;
  char data[] = { angle };
  this->run_command(CMD_SET_CAMERA_ANGLE, sizeof(data), data, &response);
  return response;
};

struct HvcGetCameraAngleResponse HVC::getCameraAngle()
{
  HvcGetCameraAngleResponse response;
  this->run_command(CMD_GET_CAMERA_ANGLE, 0, NULL, &response);
  return response;
};

struct HvcResponse HVC::setThresholdValues(int body, int hand, int face, int recognition)
{
  HvcResponse response;

  char data[8];
  util_int_into_lsb_msb(data, 0, body);
  util_int_into_lsb_msb(data, 2, hand);
  util_int_into_lsb_msb(data, 4, face);
  util_int_into_lsb_msb(data, 6, recognition);

  this->run_command(CMD_SET_THRESHOLD_VALUES, sizeof(data), data, &response);
  return response;
}

struct HvcGetThresholdValuesResponse HVC::getThresholdValues()
{
  HvcGetThresholdValuesResponse response;

  this->run_command(CMD_GET_THRESHOLD_VALUES, 0, NULL, &response);
  return response;
}

struct HvcResponse HVC::setDetectionSize(int minBody, int maxBody, int minHand, int maxHand, int minFace, int maxFace)
{
  HvcResponse response;
  char data[12];

  util_int_into_lsb_msb(data, 0, minBody);
  util_int_into_lsb_msb(data, 2, maxBody);
  util_int_into_lsb_msb(data, 4, minHand);
  util_int_into_lsb_msb(data, 6, maxHand);
  util_int_into_lsb_msb(data, 8, minFace);
  util_int_into_lsb_msb(data, 10, maxFace);
  
  this->run_command(CMD_SET_DETECTION_SIZE, sizeof(data), data, &response);
  return response;
}

struct HvcGetDetectionSizeResponse HVC::getDetectionSize()
{
  HvcGetDetectionSizeResponse response;
  this->run_command(CMD_GET_DETECTION_SIZE, 0, NULL, &response);
  return response;
}

struct HvcResponse HVC::setFaceAngle(char yaw, char roll)
{
  HvcResponse response;
  char data[2];
  data[0] = yaw;
  data[1] = roll;
  
  this->run_command(CMD_SET_FACE_ANGLE, sizeof(data), data, &response);
  return response;
}

struct HvcGetFaceAngleResponse HVC::getFaceAngle()
{
  HvcGetFaceAngleResponse response;
  this->run_command(CMD_GET_FACE_ANGLE, 0, NULL, &response);
  return response;
}

struct HvcExecutionResponse HVC::execute(int function, int imageByte)
{
  HvcExecutionResponse response;
  char data[3];
  data[0] = (function & 0xFF);
  data[1] = ((function >> 8) & 0xFF);
  data[2] = (imageByte & 0xFF);
  
  this->run_command(CMD_EXECUTE, sizeof(data), data, &response);
  return response; 
}


*/