#include "Arduino.h"
#include "hvc.h"
#include "esp_log.h"
#include "util.h"

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

static const char *TAG = "HVC";

HVC::HVC(Stream *p)
{
  port = p;
};

struct HvcResponseHeader HVC::retrieve_header()
{
  char header_buffer[HEADER_BUFFER_SIZE];
  HvcResponseHeader header;
  
  int bytesRead = port->readBytes(header_buffer, HEADER_SIZE);
  ESP_LOGD(TAG, "Bytes read for response headers: %d/%d", bytesRead, HEADER_SIZE);
  ESP_LOGI(TAG, "Header response: %s", Util::hexArrayToString(header_buffer, bytesRead));
  
  memmove(&header, header_buffer, HEADER_SIZE);
  ESP_LOGD(TAG, "Discovered data length: %d", header.data_length);
  
  return header;
};

/*
void HVC::load_data(struct ResponseHeader header, struct Response *response)
{
  char data_buffer[DATA_BUFFER_SIZE];
  
  int bytesRead = port->readBytes(data_buffer, header.data_length);
  ESP_LOGD(TAG, "Bytes read for response data: %d/%d", bytesRead, header.data_length);
  ESP_LOGI(TAG, "Command response: %s", Util::hexArrayToString(data_buffer, bytesRead));

  response->populateFromBytes(header, data_buffer);
};
*/

void HVC::run_command(char cmd, int data_size, char *data, struct HvcResponse *response)
{
  char send_data[SEND_BUFFER_SIZE];
  
  send_data[0] = SYNC_CODE;
  send_data[1] = cmd;
  send_data[2] = Util::LSB(data_size);
  send_data[3] = Util::MSB(data_size);

  for (int i = 0; i < data_size; i++)
  {
    send_data[4 + i] = data[i];  
  }
  
  ESP_LOGI(TAG, "Executing command: %s", Util::hexArrayToString(send_data, CMD_SIZE + data_size));
  port->write(send_data, CMD_SIZE + data_size);
  
  HvcResponseHeader header = this->retrieve_header();
  
  if (header.sync_code != SYNC_CODE) {
    ESP_LOGE(TAG, "Header sync code invalid: %02x", header.sync_code);
    return;
  }

  if (header.responseCode != 0x00) {
    ESP_LOGE(TAG, "Header response code invalid: %02x", header.responseCode);
    return;  
  }

  response->populate(&header, this->port);
  //this->load_data(header, response);
};

struct HvcGetVersionResponse HVC::getVersion()
{
  HvcGetVersionResponse response;
  this->run_command(CMD_GET_VERSION, 0, NULL, &response);
  return response;
};

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
  Util::intIntoLSBMSB(data, 0, body);
  Util::intIntoLSBMSB(data, 2, hand);
  Util::intIntoLSBMSB(data, 4, face);
  Util::intIntoLSBMSB(data, 6, recognition);

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

  Util::intIntoLSBMSB(data, 0, minBody);
  Util::intIntoLSBMSB(data, 2, maxBody);
  Util::intIntoLSBMSB(data, 4, minHand);
  Util::intIntoLSBMSB(data, 6, maxHand);
  Util::intIntoLSBMSB(data, 8, minFace);
  Util::intIntoLSBMSB(data, 10, maxFace);
  
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


