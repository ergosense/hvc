#ifndef HVC_H
#define HVC_H

#include "mgos.h" // TODO get rid of this dependency
#include "response.h"

// TODO should not be defined here
#define HVC_UART_NUM 2

/*
 * Function sync code, all functions must return this
 * code to indicate valid responses
 */
#define HVC_SYNC_CODE 0xFE

/*
 * List of available camera angles
 */
#define HVC_CAMERA_ANGLE_0    0x00
#define HVC_CAMERA_ANGLE_90   0x01
#define HVC_CAMERA_ANGLE_180  0x02
#define HVC_CAMERA_ANGLE_270  0x03

/*
 * List of face profile angle recognition
 */
#define HVC_YAW_ANGLE_30  0x00
#define HVC_YAW_ANGLE_60  0x01
#define HVC_YAW_ANGLE_90  0x02

/*
 * List of head angle recognition
 */
#define HVC_ROLL_ANGLE_15 0x00
#define HVC_ROLL_ANGLE_45 0x01

/*
 * List of supported execution flags
 */
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

/*
 * List of supported image options
 */
#define HVC_EX_IMAGE_NONE      0x00
#define HVC_EX_IMAGE_QVGA      0x01
#define HVC_EX_IMAGE_QVGA_HALF 0x02

/*
 * List of supported commands
 */
#define HVC_CMD_GET_VERSION           0x00
#define HVC_CMD_SET_CAMERA_ANGLE      0x01
#define HVC_CMD_GET_CAMERA_ANGLE      0x02
#define HVC_CMD_EXECUTE               0x04
#define HVC_CMD_SET_THRESHOLD_VALUES  0x05
#define HVC_CMD_GET_THRESHOLD_VALUES  0x06
#define HVC_CMD_SET_DETECTION_SIZE    0x07
#define HVC_CMD_GET_DETECTION_SIZE    0x08
#define HVC_CMD_SET_FACE_ANGLE        0x09
#define HVC_CMD_GET_FACE_ANGLE        0x0A

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

QueueHandle_t command_queue;

typedef void (*hvc_command_parser)();
typedef void (*hvc_command_callback)(void* response);

struct hvc_command
{
  char cmd;
  hvc_command_parser parser;
  void* response;
  hvc_command_callback fn;
};

void hvc_read_bytes(char* data, int length);

void hvc_write_bytes(char* data, int length);

void hvc_handle_response();

void hvc_get_version(hvc_command_callback fn);

/*
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
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
