#ifndef CMSG_H
#define CMSG_H

#include <msgpack.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
  CMSG_OK,
  CMSG_ERR_KEY_NOT_FOUND,
  CMSG_ERR_INVALID,
  CMSG_ERR_PARSE,
  CMSG_ERR_TYPE_MISMATCH,
  CMSG_ERR_OUT_OF_RANGE,
  CMSG_ERR_ALLOC
} cmsg_err;

typedef struct
{
  msgpack_sbuffer *sbuf;
  msgpack_packer *pk;
  msgpack_unpacked *unpacked_obj;
} cmsg;

cmsg *cmsg_create ();

cmsg *cmsg_from_buf (const char *buf, size_t size);

void cmsg_free (cmsg *msg);
void cmsg_clear (cmsg *msg);

void cmsg_print (cmsg *msg);

cmsg_err cmsg_insert_map (cmsg *msg, size_t entries);
cmsg_err cmsg_insert_nested_map (cmsg *msg, const char *key, size_t entries);

cmsg_err cmsg_insert_pair_i8 (cmsg *msg, const char *key, int8_t value);
cmsg_err cmsg_insert_pair_i16 (cmsg *msg, const char *key, int16_t value);
cmsg_err cmsg_insert_pair_i32 (cmsg *msg, const char *key, int32_t value);
cmsg_err cmsg_insert_pair_i64 (cmsg *msg, const char *key, int64_t value);

cmsg_err cmsg_insert_pair_ui8 (cmsg *msg, const char *key, uint8_t value);
cmsg_err cmsg_insert_pair_ui16 (cmsg *msg, const char *key, uint16_t value);
cmsg_err cmsg_insert_pair_ui32 (cmsg *msg, const char *key, uint32_t value);
cmsg_err cmsg_insert_pair_ui64 (cmsg *msg, const char *key, uint64_t value);

cmsg_err cmsg_insert_pair_str (cmsg *msg, const char *key, const char *value);

int8_t cmsg_get_i8_from_key (cmsg *msg, const char *key, cmsg_err *err);
int16_t cmsg_get_i16_from_key (cmsg *msg, const char *key, cmsg_err *err);
int32_t cmsg_get_i32_from_key (cmsg *msg, const char *key, cmsg_err *err);
int64_t cmsg_get_i64_from_key (cmsg *msg, const char *key, cmsg_err *err);

uint8_t cmsg_get_ui8_from_key (cmsg *msg, const char *key, cmsg_err *err);
uint16_t cmsg_get_ui16_from_key (cmsg *msg, const char *key, cmsg_err *err);
uint32_t cmsg_get_ui32_from_key (cmsg *msg, const char *key, cmsg_err *err);
uint64_t cmsg_get_ui64_from_key (cmsg *msg, const char *key, cmsg_err *err);

cmsg *cmsg_get_nested_map_from_key (cmsg *msg, const char *key, cmsg_err *err);

#ifdef __cplusplus
}
#endif

#endif // CMSG_H