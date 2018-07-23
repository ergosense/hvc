#ifndef HVC_UTIL_H
#define HVC_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

char* util_terminate_string(char* arr, int arr_size);

void util_slice(char* arr, char* dest, int start, int offset);

int util_bytes_to_int(char lsb, char msb);

int util_lsb(int val);

int util_msb(int val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
