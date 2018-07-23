#ifndef HVC_RESPONSE_H
#define HVC_RESPONSE_H

#include "Arduino.h"
#include "util.h"
#include "hvc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct HvcResponseHeader
{
  char sync_code;
  char responseCode;

  union
  {
    char data_length_bytes[4];
    int data_length;
  } __attribute__((packed));
};

struct HvcResponse
{
  int bytesToRead;
  
  void populate(HvcResponseHeader *header, Stream *port);
  void debug();
  boolean ok();
  
  protected:
    HvcResponseHeader *header;
    int read(char *var, int length, Stream *port);
    virtual void dump();
    virtual void extract(Stream *port);
};

struct HvcGetVersionResponse:HvcResponse
{
  char model[12];
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint8_t releaseVersion;
  char revision[4];

  protected:
    void extract(Stream *port);
    void dump();
};

struct HvcGetCameraAngleResponse:HvcResponse
{
  char angle;

  protected:
    void extract(Stream *port);
    void dump();
};

struct HvcGetThresholdValuesResponse:HvcResponse
{
  int body;
  int hand;
  int face;
  int recognition;  
 
  protected:
    void extract(Stream *port);
    void dump();
};

struct HvcGetDetectionSizeResponse:HvcResponse
{
  int minBody;
  int maxBody;
  int minHand;
  int maxHand;
  int minFace;
  int maxFace;

  protected:
    void extract(Stream *port);
    void dump();
};

struct HvcGetFaceAngleResponse:HvcResponse
{
  char yaw;
  char roll;

  protected:
    void extract(Stream *port);
    void dump();
};

struct HvcExecutionResponse:HvcResponse
{
  uint8_t bodyCount = 0;
  uint8_t handCount = 0;
  uint8_t faceCount = 0;

  int imageWidth;
  int imageHeight;
  
  protected:
    void extract(Stream *port);
    void dump();
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
