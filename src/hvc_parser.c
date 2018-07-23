#include "hvc.h"
#include "hvc_parser.h"

void hvc_get_version_parser(void* response)
{
  struct hvc_get_version_response* res = (struct hvc_get_version_response*) response;

  hvc_read_bytes(res->model, 12);
  hvc_read_bytes((char *) &res->major_version, 1);
  hvc_read_bytes((char *) &res->minor_version, 1);
  hvc_read_bytes((char *) &res->release_version, 1);
  hvc_read_bytes(res->revision, 4);

  // TODO test what free(res) does here
}