#ifndef HVC_UTIL_H
#define HVC_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

class Util
{
  public:
    static char* hexArrayToString(char * arr, int arr_size);
    static char* terminatedString(char *arr, int arrSize);
    static void slice(char *arr, char *dest, int start, int offset);
    static int bytesToInt(char lsb, char msb);
    static void intIntoLSBMSB(char* arr, int index, int val);
    static int LSB(int val);
    static int MSB(int val);
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
