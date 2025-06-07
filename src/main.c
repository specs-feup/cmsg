#include <stdio.h>

#include "cmsg.h"

int
main (void)
{
  cmsg *msg = cmsg_create ();

  cmsg_insert_map(msg, 2);
  cmsg_insert_pair_i32 (msg, "value1", 10);
  cmsg_insert_pair_i32 (msg, "value2", 10);

  cmsg_err err;
  int32_t v1 = cmsg_get_i32_from_key(msg, "value1", &err);

  printf("v1: %d\n", v1);

  cmsg_print (msg);

  cmsg_free (msg);

  return 0;
}
