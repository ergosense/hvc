#ifndef HVC_RESPONSE_H
#define HVC_RESPONSE_H

#include <stdint.h>
#include "hvc_util.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// TODO use uint8_t instead int

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

struct hvc_get_camera_angle_response
{
  char angle;
};

struct hvc_get_threshold_values_response
{
  int body;
  int hand;
  int face;
  int recognition;  
};

struct hvc_get_detection_size_response
{
  int min_body;
  int max_body;
  int min_hand;
  int max_hand;
  int min_face;
  int max_face;
};

struct hvc_get_face_angle_response
{
  char yaw;
  char roll;
};

struct hvc_execution_response
{
  uint8_t body_count;
  uint8_t hand_count;
  uint8_t face_count;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
