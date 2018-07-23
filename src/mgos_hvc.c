/**
 * Mongoose OS specific logic for dealing with the HVC-P sensor. Registers
 * listeners and executes setup commands to do face/body recognition.
 */
#include "mgos.h"
#include "mgos_hvc.h"
#include "mgos_uart.h"
#include "hvc.h"

static void _mgos_hvc_debug_function(void* response)
{
  struct hvc_get_version_response* res = (struct hvc_get_version_response*) response;

  LOG(LL_INFO, ("AT RESPONSE"));
  LOG(LL_INFO, ("AT %s", util_terminate_string(res->model, 12)));
  LOG(LL_INFO, ("AT %d", res->major_version));
  LOG(LL_INFO, ("AT %d", res->minor_version));
  LOG(LL_INFO, ("AT %d", res->release_version));
  LOG(LL_INFO, ("Revision: %02x%02x%02x%02x", res->revision[0], res->revision[1], res->revision[2], res->revision[3]));
}

static void _mgos_hvc_uart_dispatcher(int uart_no, void *arg) {

  // Read available bytes
  int avail = mgos_uart_read_avail(uart_no);

  if (avail)
  {
    LOG(LL_INFO, ("Available bytes found %d", avail));
    hvc_handle_response();
  }
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

  // Set MGOS UART handler
  mgos_uart_set_dispatcher(HVC_UART_NUM, _mgos_hvc_uart_dispatcher, NULL);

  // TODO this should be somewhere else
  command_queue = xQueueCreate(10, sizeof(struct hvc_command));

  // Initialize
  hvc_get_version(_mgos_hvc_debug_function);
}