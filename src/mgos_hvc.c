/**
 * Mongoose OS specific logic for dealing with the HVC-P sensor. Registers
 * listeners and executes setup commands to do face/body recognition.
 */
#include "mgos.h"
#include "mgos_event.h"
#include "mgos_hvc.h"
#include "mgos_uart.h"
#include "mgos_sys_config.h"
#include "hvc.h"
#include "hvc_response.h"
#include "hvc_util.h"

#define HVC_EXECUTION_INTERVAL 1000

#define MGOS_HVC_LOG_BUFFER 100

char log_buffer[MGOS_HVC_LOG_BUFFER];

void hvc_log_debug(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);

  vsprintf(log_buffer, format, ap);
  va_end(ap);

  LOG(LL_DEBUG, (util_terminate_string(log_buffer, strlen(log_buffer))));
}

void hvc_log_info(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);

  vsprintf(log_buffer, format, ap);
  va_end(ap);

  LOG(LL_INFO, (util_terminate_string(log_buffer, strlen(log_buffer))));
}

void hvc_log_error(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);

  vsprintf(log_buffer, format, ap);
  va_end(ap);

  LOG(LL_ERROR, (util_terminate_string(log_buffer, strlen(log_buffer))));
}

int hvc_read_bytes_available()
{
  return mgos_uart_read_avail(HVC_UART_NUM);
}

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
  // Don't attempt to retry these first connections, if they can't complete
  // quickly then something is wrong and we should probably reset sooner rather
  // than later.
  hvc_set_retry(0);

  // Setup variable
  bool set = true;

  set &= hvc_set_camera_angle(
    mgos_sys_config_get_hvc_camera_angle()
  );
  
  set &= hvc_set_threshold_values(
    mgos_sys_config_get_hvc_thresholds_body(), 
    mgos_sys_config_get_hvc_thresholds_hand(), 
    mgos_sys_config_get_hvc_thresholds_face(), 
    mgos_sys_config_get_hvc_thresholds_recognition()
  );

  set &= hvc_set_detection_size(
    mgos_sys_config_get_hvc_detection_size_min_body(), 
    mgos_sys_config_get_hvc_detection_size_max_body(), 
    mgos_sys_config_get_hvc_detection_size_min_hand(), 
    mgos_sys_config_get_hvc_detection_size_max_hand(), 
    mgos_sys_config_get_hvc_detection_size_min_face(), 
    mgos_sys_config_get_hvc_detection_size_max_face()
  );

  // Don't think the face angle matching needs to be configurable at this point
  set &= hvc_set_face_angle(HVC_YAW_ANGLE_30, HVC_ROLL_ANGLE_15);

  // Reset the retry before our normal procedures commence. 
  hvc_set_retry(HVC_DEFAULT_READ_RETRY);

  struct hvc_get_version_response* version_res = hvc_get_version();

  if (!set || version_res == NULL)
  {
    LOG(LL_ERROR, ("Unable to initialize HVC properly, restart..."));
    mgos_system_restart();
  }

  LOG(LL_INFO, ("HVC intialized..."));
  mgos_event_trigger(MGOS_HVC_EVENT_INIT, version_res); 

  free(version_res);

  // We shouldn't have to read the values here. Failures will
  // be listed in the logs.

  while(1)
  {
    struct hvc_execution_response* res = hvc_execution(HVC_EX_BODY_DETECTION | HVC_EX_FACE_DETECTION, HVC_EX_IMAGE_NONE);

    if (res != NULL)
    {
      int matches = res->body_count + res->face_count;

      if (matches)
      {
        mgos_event_trigger(MGOS_HVC_EVENT_DETECTION, res); 
      }
    }

    // Free the resource
    free(res);

    vTaskDelay(HVC_EXECUTION_INTERVAL / portTICK_RATE_MS);
  }

  vTaskDelete(NULL);
}

void mgos_hvc_init()
{
  mgos_event_register_base(MGOS_HVC_EVENT_BASE, "HVC mongoose module");

  struct mgos_uart_config ucfg;
  mgos_uart_config_set_defaults(HVC_UART_NUM, &ucfg);

  // Set the connection rate
  ucfg.baud_rate = mgos_sys_config_get_hvc_baudrate();
    
  // TODO buffer size?
    
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
  // TODO check stack size please
  xTaskCreate(&_hvc_setup, "_hvc_setup", 10000, NULL, 1, NULL);
}