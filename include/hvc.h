#ifndef HVC_H
#define HVC_H

#include "response.h"

#define SYNC_CODE 0xFE

#define CAMERA_ANGLE_0    0x00
#define CAMERA_ANGLE_90   0x01
#define CAMERA_ANGLE_180  0x02
#define CAMERA_ANGLE_270  0x03

#define YAW_ANGLE_30  0x00
#define YAW_ANGLE_60  0x01
#define YAW_ANGLE_90  0x02

#define ROLL_ANGLE_15 0x00
#define ROLL_ANGLE_45 0x01

#define HVC_EX_BODY_DETECTION        0x00000001
#define HVC_EX_HAND_DETECTION        0x00000002
#define HVC_EX_FACE_DETECTION        0x00000004
#define HVC_EX_FACE_DIRECTION        0x00000008
#define HVC_EX_AGE_ESTIMATION        0x00000010
#define HVC_EX_GENDER_ESTIMATION     0x00000020
#define HVC_EX_GAZE_ESTIMATION       0x00000040
#define HVC_EX_BLINK_ESTIMATION      0x00000080
#define HVC_EX_EXPRESSION_ESTIMATION 0x00000100
#define HVC_EX_FACE_RECOGNITION      0x00000200

#define HVC_EX_IMAGE_NONE      0x00
#define HVC_EX_IMAGE_QVGA      0x01
#define HVC_EX_IMAGE_QVGA_HALF 0x02

#define CMD_GET_VERSION           0x00
#define CMD_SET_CAMERA_ANGLE      0x01
#define CMD_GET_CAMERA_ANGLE      0x02
#define CMD_EXECUTE               0x04
#define CMD_SET_THRESHOLD_VALUES  0x05
#define CMD_GET_THRESHOLD_VALUES  0x06
#define CMD_SET_DETECTION_SIZE    0x07
#define CMD_GET_DETECTION_SIZE    0x08
#define CMD_SET_FACE_ANGLE        0x09
#define CMD_GET_FACE_ANGLE        0x0A

void raw_debug(char * arr, int arr_size);

char* hex_array_to_string(char * arr, int arr_size);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

class HVC
{
  private:
    Stream *port;
    struct HvcResponseHeader retrieve_header();
    void run_command(char cmd, int data_size, char *data, struct HvcResponse *response);
  public:
    HVC(Stream *port);
    //
    struct HvcGetVersionResponse getVersion();
    //
    struct HvcResponse setCameraAngle(char angle);
    //
    struct HvcGetCameraAngleResponse getCameraAngle();
    //
    struct HvcResponse setThresholdValues(int body, int hand, int face, int recognition);
    //
    struct HvcGetThresholdValuesResponse getThresholdValues();
    //
    struct HvcResponse setDetectionSize(int minBody, int maxBody, int minHand, int maxHand, int minFace, int maxFace);
    //
    struct HvcGetDetectionSizeResponse getDetectionSize();
    //
    struct HvcResponse setFaceAngle(char yaw, char roll);
    //
    struct HvcGetFaceAngleResponse getFaceAngle();
    //
    struct HvcExecutionResponse execute(int function, int image);
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
