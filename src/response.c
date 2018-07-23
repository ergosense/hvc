#include "response.h"
#include "util.h"
#include "esp_log.h"
#include "hvc.h"

static const char *TAG = "HVC-RESPONSE";

void HvcResponse::populate(HvcResponseHeader *header, Stream *port)
{
  this->header = header;

  // TODO need to deal with this
  this->bytesToRead = header->data_length;
   
  this->extract(port); 
};

boolean HvcResponse::ok()
{
  return this->header->responseCode == 0x00;
}

void HvcResponse::debug()
{
  ESP_LOGD(TAG, "Response Code: %02x", this->header->responseCode);
  this->dump();
};

void HvcResponse::dump()
{
  //noop
};

void HvcResponse::extract(Stream *port)
{
  //noop
};

int HvcResponse::read(char *var, int length, Stream *port)
{
  int read = port->readBytes(var, length);

  // TODO compare read vs length
  this->bytesToRead -= read;
  
  return read;
};

void HvcGetVersionResponse::extract(Stream *port)
{     
  int read = 0;
  
  read += port->readBytes(this->model, 12);
  read += port->readBytes(&this->majorVersion, 1);
  read += port->readBytes(&this->minorVersion, 1);
  read += port->readBytes(&this->releaseVersion, 1);
  read += port->readBytes(this->revision, 4);

  //ESP_LOGE("Header provided data length does not match read bytes: %d/%d", 5, read);
};

void HvcGetVersionResponse::dump()
{
  ESP_LOGD(TAG, "Model: %s", Util::terminatedString(this->model, 12));
  ESP_LOGD(TAG, "Major Version: %d", this->majorVersion);
  ESP_LOGD(TAG, "Minor Version: %d", this->minorVersion);
  ESP_LOGD(TAG, "Release Version: %d", this->releaseVersion);
  ESP_LOGD(TAG, "Revision: %02x%02x%02x%02x", this->revision[0], this->revision[1], this->revision[2], this->revision[3]);
}

void HvcGetCameraAngleResponse::extract(Stream *port)
{
  port->readBytes(&this->angle, 1);
};

void HvcGetCameraAngleResponse::dump()
{
  ESP_LOGD(TAG, "Angle: %d", this->angle);  
};

void HvcGetThresholdValuesResponse::extract(Stream *port)
{ 
  char bytes[8];
  int read = port->readBytes(bytes, sizeof(bytes));
  
  this->hand = Util::bytesToInt(bytes[0], bytes[1]);
  this->body = Util::bytesToInt(bytes[2], bytes[3]);
  this->face = Util::bytesToInt(bytes[4], bytes[5]);
  this->recognition = Util::bytesToInt(bytes[6], bytes[7]);
  
  //ESP_LOGE("Header provided data length does not match read bytes: %d/%d", 5, read);
};

void HvcGetThresholdValuesResponse::dump()
{
  ESP_LOGD(TAG, "Threshold Values: %d %d %d %d", this->hand, this->body, this->face, this->recognition);
};

void HvcGetDetectionSizeResponse::extract(Stream *port)
{
  char bytes[12];
  int read = port->readBytes(bytes, sizeof(bytes));
  
  this->minBody = Util::bytesToInt(bytes[0], bytes[1]);
  this->maxBody = Util::bytesToInt(bytes[2], bytes[3]);
  this->minHand = Util::bytesToInt(bytes[4], bytes[5]);
  this->maxHand = Util::bytesToInt(bytes[6], bytes[7]);
  this->minFace = Util::bytesToInt(bytes[8], bytes[9]);
  this->maxFace = Util::bytesToInt(bytes[10], bytes[11]);
  
  //ESP_LOGE("Header provided data length does not match read bytes: %d/%d", 5, read);
};

void HvcGetDetectionSizeResponse::dump()
{
  ESP_LOGD(TAG, "Detection Size: %d-%d %d-%d %d-%d", this->minBody, this->maxBody, this->minHand, this->maxHand, this->minFace, this->maxFace);
};

void HvcGetFaceAngleResponse::extract(Stream *port)
{
  int read;
  
  read += port->readBytes(&this->yaw, 1);
  read += port->readBytes(&this->roll, 1);
  
  //ESP_LOGE("Header provided data length does not match read bytes: %d/%d", 5, read);
};

void HvcGetFaceAngleResponse::dump()
{
  ESP_LOGD(TAG, "Face Angle: %02x %02x", this->yaw, this->roll);
};

void HvcExecutionResponse::extract(Stream *port)
{  
  char header[4];
  this->read(header, sizeof(header), port);

  this->bodyCount = header[0];
  this->handCount = header[1];
  this->faceCount = header[2];
  // header[3] reserved and unused

  // empty out
  while (port->available())
  {
    char c = port->read();
  }

  // Read remaining bytes into image var
  //this->read(this->image, this->imageWidth * this->imageHeight, port);
};

void HvcExecutionResponse::dump()
{
  ESP_LOGD(TAG, "Detections: %d/%d/%d", this->bodyCount, this->handCount, this->faceCount);
  ESP_LOGD(TAG, "Image Size: %d x %d", this->imageWidth, this->imageHeight);
};

