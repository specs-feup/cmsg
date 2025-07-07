#include <stdio.h>
#include <stdlib.h>

#include "cmsg.h"

#define SRC_ADDRESS_EXPECTED_VALUE 30

void
insert_data_map (cmsg *msg)
{
  cmsg_insert_nested_map (msg, "data", 4);
  cmsg_insert_pair_i32 (msg, "src_address", SRC_ADDRESS_EXPECTED_VALUE);
  cmsg_insert_pair_i32 (msg, "dst_address", 20);
  cmsg_insert_pair_i32 (msg, "threshold", 10);
  cmsg_insert_pair_i32 (msg, "data_size", 0);
}

void
insert_memory_map (cmsg *msg)
{
  cmsg_insert_nested_map (msg, "memory", 4);
  cmsg_insert_pair_i32 (msg, "read_req_gnt", 0);
  cmsg_insert_pair_i32 (msg, "read_req_rvalid", 5);
  cmsg_insert_pair_i32 (msg, "read_req_rdata", 0);
  cmsg_insert_pair_i32 (msg, "write_req_gnt", 0);
}

int
main (void)
{

  cmsg *pack_msg = cmsg_create ();
  if (!pack_msg)
    {
      fprintf (stderr, "Failed to create packing cmsg object!\n");
      return EXIT_FAILURE;
    }

  cmsg_insert_map (pack_msg, 3);
  cmsg_insert_pair_i32 (pack_msg, "control", 0);
  insert_data_map (pack_msg);
  insert_memory_map (pack_msg);

  char *packed_data = pack_msg->sbuf->data;
  size_t packed_size = pack_msg->sbuf->size;

  cmsg *unpack_msg = cmsg_from_buf (packed_data, packed_size);
  if (!unpack_msg)
    {
      fprintf (stderr,
               "Failed to create unpacking cmsg object from buffer!\n");
      cmsg_free (pack_msg);
      return EXIT_FAILURE;
    }

  cmsg_err err;
  cmsg *data = cmsg_get_nested_map_from_key (unpack_msg, "data", &err);
  if (err != CMSG_OK)
    {
      fprintf (stderr, "Failed to get nested map! Error: %d\n", err);
      cmsg_free (pack_msg);
      cmsg_free (unpack_msg);
      return EXIT_FAILURE;
    }

  int32_t src_address_actual_value
      = cmsg_get_i32_from_key (data, "src_address", &err);
  if (err != CMSG_OK)
    {
      fprintf (stderr, "Failed to get I32 from key! Error: %d\n", err);
      cmsg_free (pack_msg);
      cmsg_free (unpack_msg);
      cmsg_free (data);
      return EXIT_FAILURE;
    }

  if (src_address_actual_value != SRC_ADDRESS_EXPECTED_VALUE)
    {
      fprintf (stderr, "Values do not match: expected %d, actual %d\n",
               SRC_ADDRESS_EXPECTED_VALUE, src_address_actual_value);
      cmsg_free (pack_msg);
      cmsg_free (unpack_msg);
      cmsg_free (data);
      return EXIT_FAILURE;
    }

  fprintf (stdout, "Success, values match!\n");

  cmsg_free (pack_msg);
  cmsg_free (unpack_msg);
  cmsg_free (data);

  return EXIT_SUCCESS;
}
