#include <stdio.h>

#include "cmsg.h"

int
main (void)
{
  cmsg *msg = cmsg_create ();

  cmsg_insert_map (msg, 3);

  cmsg_insert_pair_i32 (msg, "control", 0);

  cmsg_insert_nested_map (msg, "data", 4);
  cmsg_insert_pair_i32 (msg, "src_address", 30);
  cmsg_insert_pair_i32 (msg, "dst_address", 20);
  cmsg_insert_pair_i32 (msg, "threshold", 10);
  cmsg_insert_pair_i32 (msg, "data_size", 0);

  cmsg_insert_nested_map (msg, "memory", 4);
  cmsg_insert_pair_i32 (msg, "read_req_gnt", 0);
  cmsg_insert_pair_i32 (msg, "read_req_rvalid", 5);
  cmsg_insert_pair_i32 (msg, "read_req_rdata", 0);
  cmsg_insert_pair_i32 (msg, "write_req_gnt", 0);

  cmsg_print (msg);

  cmsg_err err;
  cmsg *data = cmsg_get_nested_map_from_key (msg, "data", &err);

  int32_t src_address = cmsg_get_i32_from_key (data, "src_address", &err);

  printf ("src_address: %d\n", src_address);

  cmsg_free (msg);
  cmsg_free (data);

  return 0;
}
