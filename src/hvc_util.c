#include "hvc_util.h"

#define DEBUG_BUFFER_SIZE 50

char debug_buffer[DEBUG_BUFFER_SIZE];

char* util_terminate_string(char *arr, int arr_size)
{
  for (int i = 0; i < arr_size; i++)
  {
    debug_buffer[i] = arr[i];
  }

  debug_buffer[arr_size] = '\0';
  return debug_buffer;
}

void util_slice(char *arr, char *dest, int start, int offset)
{
  for (int i = start; i < start + offset; i++)
  {
    dest[i - start] = arr[i];
  }
}

int util_bytes_to_int(char lsb, char msb)
{
  return lsb + (msb << 8);
}

void util_int_into_lsb_msb(char* arr, int index, int val)
{
  arr[index] = util_lsb(val);
  arr[index + 1] = util_msb(val);
}

int util_lsb(int val)
{
  return (val & 0xFF); 
}

int util_msb(int val)
{
  return ((val >> 8) & 0xFF);
}

