#ifndef MGOS_HVC_H
#define MGOS_HVC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mgos.h"

/*
 * Define the UART RX/TX pins and the RX buffer.
 * TX needs no buffer as we will wait/block until command has sent.
 */
#define MGOS_HVC_RX_PIN 16
#define MGOS_HVC_TX_PIN 17
#define HVC_UART_BUFFER_SIZE 4096

/*
 * Define how often we will attempt to detect humans
 */
#define HVC_EXECUTION_INTERVAL 1000

/*
 * Proxy log buffer size. Messages can't exceed this
 * length or the will miss characters.
 */
#define MGOS_HVC_LOG_BUFFER 100

/*
 * Macro to check if HVC SET calls succeeded or not
 */
#define MGOS_HVC_ERROR_CHECK(x) do {                                      \
    bool res = (x);                                                       \
    if (!res) {                                                           \
      LOG(LL_ERROR, ("Unable to initialize HVC properly, restart..."));   \
      mgos_system_restart();                                              \
    }                                                                     \
  } while(0);                                                             \

/*
 * Define base event and list of supported
 * MGOS events.
 */
#define MGOS_HVC_EVENT_BASE MGOS_EVENT_BASE('H', 'V', 'C')

enum {
  MGOS_HVC_EVENT_DETECTION = MGOS_HVC_EVENT_BASE,
  MGOS_HVC_EVENT_INIT
};

/*
 * Initialize the MGOS plugin used for HVC human
 * detection
 */
void mgos_hvc_init();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif