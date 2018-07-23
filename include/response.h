#ifndef HVC_RESPONSE_H
#define HVC_RESPONSE_H

#include <stdint.h>
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct hvc_response_header
{
  char sync_code;
  char response_code;

  union
  {
    char data_length_bytes[4];
    int data_length;
  };
};

struct hvc_get_version_response
{
  char model[12];
  uint8_t major_version;
  uint8_t minor_version;
  uint8_t release_version;
  char revision[4];
};

/*
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
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
