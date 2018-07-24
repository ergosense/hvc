#ifndef MGOS_HVC_H
#define MGOS_HVC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mgos.h"

#define MGOS_HVC_EVENT_BASE MGOS_EVENT_BASE('H', 'V', 'C')

enum {
  MGOS_HVC_EVENT_DETECTION = MGOS_HVC_EVENT_BASE,
  MGOS_HVC_EVENT_INIT
};

void mgos_hvc_init();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif