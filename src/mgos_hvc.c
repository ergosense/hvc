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


#include "driver/uart.h"

/*
 * Proxy logging, we don't want to redefine log functions
 * so we use a buffer we will use to forward information to MGOS
 */
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
  int length;
  uart_get_buffered_data_len(HVC_UART_NUM, (size_t*) &length);
  return length;
}

/*
 * Mongoose OS specific implementation of the write function
 *
 * @param char* data
 * @param int   length
 */
int hvc_read_bytes(char* data, int length)
{
  // TODO configure timeout
  int read = uart_read_bytes(HVC_UART_NUM, (unsigned char*) data, length, 100);

  if (read != length)
  {
    LOG(LL_ERROR, ("Could not read requested bytes %d/%d", read, length));
  }

  return read;
}

/*
 * Mongoose OS specific implementation of the write function
 *
 * @param char* data
 * @param int   length
 */
int hvc_write_bytes(char* data, int length)
{
  int written = uart_write_bytes(HVC_UART_NUM, data, length);

  if (written != length)
  {
    LOG(LL_ERROR, ("Unable to write requested bytes %d/%d", written, length));
  }

  return written;
}

static void _hvc_exec()
{
  bool debug = mgos_sys_config_get_hvc_debug();

  // TODO NULL protection
  struct hvc_get_version_response* version_res = hvc_get_version();
  mgos_event_trigger(MGOS_HVC_EVENT_INIT, version_res);
  free(version_res);

  // Delay initialization before we start running executions. This
  // will give the event handler code time to register the init event.
  vTaskDelay(100 / portTICK_RATE_MS);

  while(1)
  {
    struct hvc_execution_response* res = hvc_execution(
      HVC_EX_BODY_DETECTION | HVC_EX_FACE_DETECTION,
      debug ? HVC_EX_IMAGE_QVGA_HALF : HVC_EX_IMAGE_NONE
    );

    if (res != NULL)
    {
      int matches = res->body_count + res->face_count;

      if (matches)
      {
        mgos_event_trigger(MGOS_HVC_EVENT_DETECTION, res);
      }
    }

    // Unset debug. We don't want images to keep dumping, just
    // when the device boots up and does the first recognition.
    debug = false;

    // Free the resource
    free(res);

    vTaskDelay(HVC_EXECUTION_INTERVAL / portTICK_RATE_MS);
  }

  vTaskDelete(NULL);
}

void mgos_hvc_init()
{
  // Configure parameters of an UART driver,
  // communication pins and install the driver
  uart_config_t uart_config = {
    .baud_rate = mgos_sys_config_get_hvc_baudrate(),
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };

  int rx = mgos_sys_config_get_hvc_rx();
  int tx = mgos_sys_config_get_hvc_tx();

  ESP_ERROR_CHECK(uart_param_config(HVC_UART_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(HVC_UART_NUM, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(HVC_UART_NUM, HVC_UART_BUFFER_SIZE, 0, 0, NULL, 0));

  // Don't attempt to retry these first connections, if they can't complete
  // quickly then something is wrong and we should probably reset sooner rather
  // than later.
  hvc_set_retry(0);

  // Drain the buffer before executing more. The reason we do this is
  // that there might be previous command responses still flowing in even after
  // the system restarted.
  ESP_ERROR_CHECK(uart_flush(HVC_UART_NUM));

  // Flushing doesn't seem sufficient, lets manually drain the buffer if
  // anything is left.
  char c;

  while (hvc_read_bytes_available())
  {
    hvc_read_bytes(&c, 500);
  }

  // Configuration variables
  int angle = mgos_sys_config_get_hvc_camera_angle();
  int thres_b = mgos_sys_config_get_hvc_thresholds_body();
  int thres_h = mgos_sys_config_get_hvc_thresholds_hand();
  int thres_f = mgos_sys_config_get_hvc_thresholds_face();
  int thres_r  = mgos_sys_config_get_hvc_thresholds_recognition();
  int dmin_b = mgos_sys_config_get_hvc_detection_size_min_body();
  int dmax_b = mgos_sys_config_get_hvc_detection_size_max_body();
  int dmin_h = mgos_sys_config_get_hvc_detection_size_min_hand();
  int dmax_h = mgos_sys_config_get_hvc_detection_size_max_hand();
  int dmin_f = mgos_sys_config_get_hvc_detection_size_min_face();
  int dmax_f = mgos_sys_config_get_hvc_detection_size_max_face();

  // Setup HVC
  MGOS_HVC_ERROR_CHECK(hvc_set_camera_angle(angle));
  MGOS_HVC_ERROR_CHECK(hvc_set_threshold_values(thres_b, thres_h, thres_f, thres_r));
  MGOS_HVC_ERROR_CHECK(hvc_set_detection_size(dmin_b, dmax_b, dmin_h, dmax_h, dmin_f, dmax_f));

  // Don't think the face angle matching needs to be configurable at this point
  MGOS_HVC_ERROR_CHECK(hvc_set_face_angle(HVC_YAW_ANGLE_30, HVC_ROLL_ANGLE_15));

  // Reset the retry before our normal procedures commence.
  hvc_set_retry(HVC_DEFAULT_READ_RETRY);

  // Run setup task in different vTask since we don't want to block
  // the UART task from pushing and pulling from the socket.
  // TODO check stack size please
  xTaskCreate(&_hvc_exec, "_hvc_exec", 5000, NULL, 1, NULL);
}