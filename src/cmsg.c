#include "cmsg.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static msgpack_object *
cmsg_get_root_object_for_read (cmsg *msg, cmsg_err *err)
{
  if (!msg || !err)
    {
      if (err)
        *err = CMSG_ERR_INVALID;
      return NULL;
    }
  if (!msg->unpacked_obj)
    {
      *err = CMSG_ERR_PARSE;
      return NULL;
    }
  if (msg->unpacked_obj->data.type != MSGPACK_OBJECT_MAP)
    {
      *err = CMSG_ERR_PARSE;
      return NULL;
    }
  return &(msg->unpacked_obj->data);
}

static msgpack_object *
private_get_map_object_from_key (msgpack_object *obj, const char *key)
{
  if (!obj || obj->type != MSGPACK_OBJECT_MAP)
    return NULL;

  size_t key_len = strlen (key);
  for (size_t i = 0; i < obj->via.map.size; ++i)
    {
      msgpack_object k = obj->via.map.ptr[i].key;
      if (k.type == MSGPACK_OBJECT_STR && k.via.str.size == key_len
          && strncmp (k.via.str.ptr, key, key_len) == 0)
        {
          return &(obj->via.map.ptr[i].val);
        }
    }
  return NULL;
}

cmsg *
cmsg_create ()
{
  cmsg *msg = (cmsg *)malloc (sizeof (cmsg));
  if (!msg)
    {
      return NULL;
    }

  msg->sbuf = (msgpack_sbuffer *)malloc (sizeof (msgpack_sbuffer));
  if (!msg->sbuf)
    {
      free (msg);
      return NULL;
    }
  msgpack_sbuffer_init (msg->sbuf);

  msg->pk = (msgpack_packer *)malloc (sizeof (msgpack_packer));
  if (!msg->pk)
    {
      msgpack_sbuffer_destroy (msg->sbuf);
      free (msg->sbuf);
      free (msg);
      return NULL;
    }
  msgpack_packer_init (msg->pk, msg->sbuf, msgpack_sbuffer_write);

  msg->unpacked_obj = NULL;

  return msg;
}

cmsg *
cmsg_from_buf (const char *buf, size_t size)
{
  if (!buf || size == 0)
    return NULL;

  cmsg *msg = (cmsg *)malloc (sizeof (cmsg));
  if (!msg)
    return NULL;

  msg->sbuf = NULL;
  msg->pk = NULL;

  msg->unpacked_obj = (msgpack_unpacked *)malloc (sizeof (msgpack_unpacked));
  if (!msg->unpacked_obj)
    {
      free (msg);
      return NULL;
    }
  msgpack_unpacked_init (msg->unpacked_obj);

  size_t offset = 0;
  if (!msgpack_unpack_next (msg->unpacked_obj, buf, size, &offset))
    {
      msgpack_unpacked_destroy (msg->unpacked_obj);
      free (msg->unpacked_obj);
      free (msg);
      return NULL;
    }

  if (msg->unpacked_obj->data.type != MSGPACK_OBJECT_MAP)
    {
      msgpack_unpacked_destroy (msg->unpacked_obj);
      free (msg->unpacked_obj);
      free (msg);
      return NULL;
    }

  return msg;
}

void
cmsg_free (cmsg *msg)
{
  if (msg)
    {
      if (msg->sbuf)
        {
          msgpack_sbuffer_destroy (msg->sbuf);
          free (msg->sbuf);
        }
      if (msg->pk)
        {
          free (msg->pk);
        }
      if (msg->unpacked_obj)
        {
          msgpack_unpacked_destroy (msg->unpacked_obj);
          free (msg->unpacked_obj);
        }
      free (msg);
    }
}

void
cmsg_clear (cmsg *msg)
{
  if (msg)
    {
      if (msg->sbuf)
        {
          msgpack_sbuffer_clear (msg->sbuf);
        }
      if (msg->unpacked_obj)
        {
          msgpack_unpacked_destroy (msg->unpacked_obj);
          free (msg->unpacked_obj);
          msg->unpacked_obj = NULL;
        }
    }
}

void
cmsg_print (cmsg *msg)
{
  if (!msg)
    return;

  if (msg->unpacked_obj && msg->unpacked_obj->data.type != MSGPACK_OBJECT_NIL)
    {
      msgpack_object_print (stdout, msg->unpacked_obj->data);
      printf ("\n");
      return;
    }

  if (msg->sbuf && msg->sbuf->data && msg->sbuf->size > 0)
    {
      msgpack_unpacked result;
      msgpack_unpacked_init (&result);
      size_t offset = 0;

      while (msgpack_unpack_next (&result, msg->sbuf->data, msg->sbuf->size,
                                  &offset))
        {
          msgpack_object_print (stdout, result.data);
          printf ("\n");
        }
      msgpack_unpacked_destroy (&result);
    }
  else
    {
      printf ("(Empty cmsg object)\n");
    }
}

cmsg_err
cmsg_insert_map (cmsg *msg, size_t entries)
{
  if (!msg || !msg->pk)
    {
      return CMSG_ERR_INVALID;
    }
  if (msgpack_pack_map (msg->pk, entries) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_nested_map (cmsg *msg, const char *key, size_t entries)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_map (msg->pk, entries) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i8 (cmsg *msg, const char *key, int8_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_int8 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i16 (cmsg *msg, const char *key, int16_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_int16 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i32 (cmsg *msg, const char *key, int32_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_int32 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i64 (cmsg *msg, const char *key, int64_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_int64 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui8 (cmsg *msg, const char *key, uint8_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_uint8 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui16 (cmsg *msg, const char *key, uint16_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_uint16 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui32 (cmsg *msg, const char *key, uint32_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_uint32 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui64 (cmsg *msg, const char *key, uint64_t value)
{
  if (!msg || !msg->pk || !key)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_uint64 (msg->pk, value) != 0)
    return CMSG_ERR_INVALID;
  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_str (cmsg *msg, const char *key, const char *value)
{
  if (!msg || !msg->pk || !key || !value)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str (msg->pk, strlen (key)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, key, strlen (key)) != 0)
    return CMSG_ERR_INVALID;

  if (msgpack_pack_str (msg->pk, strlen (value)) != 0)
    return CMSG_ERR_INVALID;
  if (msgpack_pack_str_body (msg->pk, value, strlen (value)) != 0)
    return CMSG_ERR_INVALID;

  return CMSG_OK;
}

static msgpack_object *
cmsg_get_value_object_from_root (cmsg *msg, const char *key, cmsg_err *err)
{
  *err = CMSG_OK;
  msgpack_object *root_obj = cmsg_get_root_object_for_read (msg, err);
  if (!root_obj)
    return NULL;

  msgpack_object *val_obj = private_get_map_object_from_key (root_obj, key);
  if (!val_obj)
    {
      *err = CMSG_ERR_KEY_NOT_FOUND;
      return NULL;
    }
  return val_obj;
}

int8_t
cmsg_get_i8_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  int64_t raw_value = val->via.i64;
  if (raw_value < INT8_MIN || raw_value > INT8_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      return 0;
    }
  return (int8_t)raw_value;
}

int16_t
cmsg_get_i16_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  int64_t raw_value = val->via.i64;
  if (raw_value < INT16_MIN || raw_value > INT16_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      return 0;
    }
  return (int16_t)raw_value;
}

int32_t
cmsg_get_i32_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  int64_t raw_value = val->via.i64;
  if (raw_value < INT32_MIN || raw_value > INT32_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      return 0;
    }
  return (int32_t)raw_value;
}

int64_t
cmsg_get_i64_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  return val->via.i64;
}

uint8_t
cmsg_get_ui8_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  uint64_t raw_value = val->via.u64;
  if (raw_value > UINT8_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      return 0;
    }
  *err = CMSG_OK;
  return (uint8_t)raw_value;
}

uint16_t
cmsg_get_ui16_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  uint64_t raw_value = val->via.u64;
  if (raw_value > UINT16_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      return 0;
    }
  *err = CMSG_OK;
  return (uint16_t)raw_value;
}

uint32_t
cmsg_get_ui32_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  uint64_t raw_value = val->via.u64;
  if (raw_value > UINT32_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      return 0;
    }
  *err = CMSG_OK;
  return (uint32_t)raw_value;
}

uint64_t
cmsg_get_ui64_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return 0;

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return 0;
    }

  *err = CMSG_OK;
  return val->via.u64;
}

cmsg *
cmsg_get_nested_map_from_key (cmsg *msg, const char *key, cmsg_err *err)
{
  msgpack_object *val = cmsg_get_value_object_from_root (msg, key, err);
  if (!val)
    return NULL;

  if (val->type != MSGPACK_OBJECT_MAP)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      return NULL;
    }

  cmsg *nested = (cmsg *)malloc (sizeof (cmsg));
  if (!nested)
    {
      *err = CMSG_ERR_ALLOC;
      return NULL;
    }

  nested->sbuf = NULL;
  nested->pk = NULL;

  nested->unpacked_obj
      = (msgpack_unpacked *)malloc (sizeof (msgpack_unpacked));
  if (!nested->unpacked_obj)
    {
      free (nested);
      *err = CMSG_ERR_ALLOC;
      return NULL;
    }
  msgpack_unpacked_init (nested->unpacked_obj);
  nested->unpacked_obj->data = *val;

  *err = CMSG_OK;
  return nested;
}
