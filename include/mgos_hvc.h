#ifndef MGOS_HVC_H
#define MGOS_HVC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mgos.h"

#define MGOS_HVC_RX_PIN 16
#define MGOS_HVC_TX_PIN 17

#define HVC_UART_BUFFER_SIZE 4096

#define MGOS_HVC_EVENT_BASE MGOS_EVENT_BASE('H', 'V', 'C')

#define MGOS_HVC_ERROR_CHECK(x) do {                                      \
    bool res = (x);                                                       \
    if (!res) {                                                           \
      LOG(LL_ERROR, ("Unable to initialize HVC properly, restart..."));   \
      mgos_system_restart();                                              \
    }                                                                     \
  } while(0);                                                             \

enum {
  MGOS_HVC_EVENT_DETECTION = MGOS_HVC_EVENT_BASE,
  MGOS_HVC_EVENT_INIT
};

void mgos_hvc_init();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif