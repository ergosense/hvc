#ifndef HVC_H
#define HVC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "hvc_response.h"

// TODO should be configurable
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

/*
 * Retry definitions
 */
#define HVC_READ_RETRY_SLEEP    1000
#define HVC_DEFAULT_READ_RETRY  5

/*
 * Image settings
 */
#define HVC_IMAGE_READ_BUFFER 200
#define HVC_IMAGE_READ_SLEEP_MS 20

extern int hvc_read_retry;

void hvc_log_debug(const char* format, ...);

void hvc_log_info(const char* format, ...);

void hvc_log_error(const char* format, ...);

int hvc_read_bytes(char* data, int length);

int hvc_write_bytes(char* data, int length);

int hvc_read_bytes_available();

void hvc_set_retry(int retry);

struct hvc_get_version_response* hvc_get_version();

bool hvc_set_camera_angle(char angle);

struct hvc_get_camera_angle_response* hvc_get_camera_angle();

bool hvc_set_threshold_values(int body, int hand, int face, int recognition);

struct hvc_get_threshold_values_response* hvc_get_threshold_values();

bool hvc_set_detection_size(int min_body, int max_body, int min_hand, int max_hand, int min_face, int max_face);

struct hvc_get_detection_size_response* hvc_get_detection_size();

bool hvc_set_face_angle(char yaw, char roll);

struct hvc_get_face_angle_response* hvc_get_face_angle();

struct hvc_execution_response* hvc_execution(int function, int image);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
