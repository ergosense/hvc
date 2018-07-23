/**
 * Mongoose OS specific logic for dealing with the HVC-P sensor. Registers
 * listeners and executes setup commands to do face/body recognition.
 */
#include "mgos.h"
#include "mgos_hvc.h"
#include "mgos_uart.h"
#include "hvc.h"
#include "hvc_response.h"

/*
 * Mongoose OS specific implementation of the write function
 *
 * @param char* data
 * @param int   length
 */
void hvc_read_bytes(char* data, int length)
{
  int read = mgos_uart_read(HVC_UART_NUM, data, length);

  if (read != length)
  {
    LOG(LL_ERROR, ("Could not read requested bytes %d/%d", read, length));
  }
}

/*
 * Mongoose OS specific implementation of the write function
 *
 * @param char* data
 * @param int   length
 */
void hvc_write_bytes(char* data, int length)
{
  int written = mgos_uart_write(HVC_UART_NUM, data, length);

  if (written != length)
  {
    LOG(LL_ERROR, ("Unable to write requested bytes %d/%d", written, length));
  }
}

static void _hvc_setup()
{
  // Read the version
  struct hvc_get_version_response* version = hvc_get_version();
  LOG(LL_INFO, ("HVC Version: %d.%d", version->major_version, version->minor_version));

  // Configure the HVC component with values read from config
  hvc_set_camera_angle(HVC_CAMERA_ANGLE_0);
  hvc_set_threshold_values(600, 500, 500, 500);
  hvc_set_detection_size(30, 8192, 40, 8192, 60, 8192);
  hvc_set_face_angle(HVC_YAW_ANGLE_30, HVC_ROLL_ANGLE_15);

  // We shouldn't have to read the values here unless. Failures will
  // be listed in the logs.

  // TODO reboot device if we can't initialize HVC correctly

  while(1)
  {
    struct hvc_execution_response* res = hvc_execution(HVC_EX_BODY_DETECTION | HVC_EX_FACE_DETECTION, HVC_EX_IMAGE_NONE);

    LOG(LL_INFO, ("Execution Result: %d/%d", res->body_count, res->face_count));

    // TODO use configurable interval
    vTaskDelay(1000 / portTICK_RATE_MS);
  }

  vTaskDelete(NULL);
}

void mgos_hvc_init()
{
  struct mgos_uart_config ucfg;
  mgos_uart_config_set_defaults(HVC_UART_NUM, &ucfg);

  // TODO make configurable
  ucfg.baud_rate = 921600;
  
  // Configure UART channel for HVC-P
  if (!mgos_uart_configure(HVC_UART_NUM, &ucfg)) 
  {
    LOG(LL_ERROR, ("Failed to configure HVC-P comms (UART%d)", HVC_UART_NUM));
  } 

  LOG(LL_INFO, ("HVC-P comm settings (baud: %d)", ucfg.baud_rate));

  // Enable receiving data
  mgos_uart_set_rx_enabled(HVC_UART_NUM, true);

  // Run setup task in different vTask since we don't want to block
  // the UART task from pushing and pulling from the socket.
  xTaskCreate(&_hvc_setup, "_hvc_setup", 10000, NULL, 1, NULL);
}