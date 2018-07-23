#include "util.h"

#define DEBUG_BUFFER_SIZE 50

char debug_buffer[DEBUG_BUFFER_SIZE];

void clear_debug_buffer()
{
  memset(debug_buffer, 0, DEBUG_BUFFER_SIZE);   
};

char* Util::hexArrayToString(char * arr, int arr_size)
{
  clear_debug_buffer();

  for (int i = 0; i < arr_size; i++)
  {
    sprintf(debug_buffer + (i * 2), "%02x", arr[i]);
  }

  return debug_buffer;
};

char* Util::terminatedString(char *arr, int arrSize)
{
  clear_debug_buffer();

  for (int i = 0; i < arrSize; i++)
  {
    debug_buffer[i] = arr[i];
  }

  debug_buffer[arrSize] = '\0';
  return debug_buffer;
}

void Util::slice(char *arr, char *dest, int start, int offset)
{
  for (int i = start; i < start + offset; i++)
  {
    dest[i - start] = arr[i];
  }
}

int Util::bytesToInt(char lsb, char msb)
{
  return lsb + (msb << 8);
}

void Util::intIntoLSBMSB(char* arr, int index, int val)
{
  arr[index] = Util::LSB(val);
  arr[index + 1] = Util::MSB(val);
}

int Util::LSB(int val)
{
  return (val & 0xFF); 
}

int Util::MSB(int val)
{
  return ((val >> 8) & 0xFF);
}

