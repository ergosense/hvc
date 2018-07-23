#ifndef HVC_PARSER_H
#define HVC_PARSER_H

#include "util.h"
#include "hvc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 
 * Parse the version response from HVC
 *
 * model            = bytes 0 - 12
 * major version    = bytes 13
 * minor version    = bytes 14
 * release version  = bytes 15
 * revision         = bytes 16 - 19
 *
 * @param hvc_get_version_reaponse* response
 */
void hvc_get_version_parser(void* response);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
