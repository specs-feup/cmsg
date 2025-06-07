#include "cmsg.h"

cmsg *
cmsg_create ()
{
  cmsg *msg = (cmsg *)malloc (sizeof (cmsg));
  msg->sbuf = (msgpack_sbuffer *)malloc (sizeof (msgpack_sbuffer));
  msg->pk = (msgpack_packer *)malloc (sizeof (msgpack_packer));

  msgpack_sbuffer_init (msg->sbuf);
  msgpack_packer_init (msg->pk, msg->sbuf, msgpack_sbuffer_write);

  return msg;
}

void
cmsg_free (cmsg *msg)
{
  if (msg)
    {
      msgpack_sbuffer_destroy (msg->sbuf);
      msgpack_packer_free (msg->pk);

      free (msg->sbuf);
      free (msg->pk);

      free (msg);
    }
}

void
cmsg_clear (cmsg *msg)
{
  if (msg)
    msgpack_sbuffer_clear (msg->sbuf);
}

void
cmsg_print (cmsg *msg)
{
  if (!msg)
    return;

  msgpack_unpacked result;
  msgpack_unpacked_init (&result);

  size_t offset = 0;

  while (
      msgpack_unpack_next (&result, msg->sbuf->data, msg->sbuf->size, &offset))
    {
      msgpack_object_print (stdout, result.data);
    }
  
  printf("\n");

  msgpack_unpacked_destroy (&result);
}

cmsg_err
cmsg_insert_map (cmsg *msg, size_t entries)
{
  msgpack_pack_map (msg->pk, entries);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_nested_map (cmsg *msg, char *key, size_t entries)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_map (msg->pk, entries);
}

cmsg_err
cmsg_insert_pair_i8 (cmsg *msg, char *key, int8_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_int8 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i16 (cmsg *msg, char *key, int16_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_int16 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i32 (cmsg *msg, char *key, int32_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_int32 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_i64 (cmsg *msg, char *key, int64_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_int64 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui8 (cmsg *msg, char *key, uint8_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_uint8 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui16 (cmsg *msg, char *key, uint16_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_uint16 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui32 (cmsg *msg, char *key, uint32_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_uint32 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_ui64 (cmsg *msg, char *key, uint64_t value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));
  msgpack_pack_uint64 (msg->pk, value);

  return CMSG_OK;
}

cmsg_err
cmsg_insert_pair_str (cmsg *msg, char *key, char *value)
{
  msgpack_pack_str (msg->pk, strlen (key));
  msgpack_pack_str_body (msg->pk, key, strlen (key));

  msgpack_pack_str (msg->pk, strlen (value));
  msgpack_pack_str_body (msg->pk, key, strlen (value));

  return CMSG_OK;
}

static msgpack_object *
private_get_map_object_from_key (msgpack_object *obj, const char *key)
{
  if (obj->type != MSGPACK_OBJECT_MAP)
    return NULL;

  for (size_t i = 0; i < obj->via.map.size; ++i)
    {
      msgpack_object k = obj->via.map.ptr[i].key;
      if (k.type == MSGPACK_OBJECT_STR && k.via.str.size == strlen (key)
          && strncmp (k.via.str.ptr, key, k.via.str.size) == 0)
        return &(obj->via.map.ptr[i].val);
    }

  return NULL;
}

int8_t
cmsg_get_i8_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  if (!msg || !key || !err)
    {
      if (err)
        *err = CMSG_ERR_INVALID;
      return 0;
    }

  msgpack_unpacked result;
  msgpack_unpacked_init (&result);
  bool ok
      = msgpack_unpack_next (&result, msg->sbuf->data, msg->sbuf->size, NULL);

  if (!ok || result.data.type != MSGPACK_OBJECT_MAP)
    {
      *err = CMSG_ERR_PARSE;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  msgpack_object *val = private_get_map_object_from_key (&result.data, key);
  if (!val)
    {
      *err = CMSG_ERR_KEY_NOT_FOUND;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  int64_t raw_value = val->via.i64;
  if (raw_value < INT8_MIN || raw_value > INT8_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  *err = CMSG_OK;
  msgpack_unpacked_destroy (&result);
  return (int8_t)raw_value;
}

int16_t
cmsg_get_i16_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  if (!msg || !key || !err)
    {
      if (err)
        *err = CMSG_ERR_INVALID;
      return 0;
    }

  msgpack_unpacked result;
  msgpack_unpacked_init (&result);
  bool ok
      = msgpack_unpack_next (&result, msg->sbuf->data, msg->sbuf->size, NULL);

  if (!ok || result.data.type != MSGPACK_OBJECT_MAP)
    {
      *err = CMSG_ERR_PARSE;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  msgpack_object *val = private_get_map_object_from_key (&result.data, key);
  if (!val)
    {
      *err = CMSG_ERR_KEY_NOT_FOUND;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  int64_t raw_value = val->via.i64;
  if (raw_value < INT16_MIN || raw_value > INT16_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  *err = CMSG_OK;
  msgpack_unpacked_destroy (&result);
  return (int16_t)raw_value;
}

int32_t
cmsg_get_i32_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  if (!msg || !key || !err)
    {
      if (err)
        *err = CMSG_ERR_INVALID;
      return 0;
    }

  msgpack_unpacked result;
  msgpack_unpacked_init (&result);
  bool ok
      = msgpack_unpack_next (&result, msg->sbuf->data, msg->sbuf->size, NULL);

  if (!ok || result.data.type != MSGPACK_OBJECT_MAP)
    {
      *err = CMSG_ERR_PARSE;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  msgpack_object *val = private_get_map_object_from_key (&result.data, key);
  if (!val)
    {
      *err = CMSG_ERR_KEY_NOT_FOUND;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  if (val->type != MSGPACK_OBJECT_POSITIVE_INTEGER
      && val->type != MSGPACK_OBJECT_NEGATIVE_INTEGER)
    {
      *err = CMSG_ERR_TYPE_MISMATCH;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  int64_t raw_value = val->via.i64;
  if (raw_value < INT32_MIN || raw_value > INT32_MAX)
    {
      *err = CMSG_ERR_OUT_OF_RANGE;
      msgpack_unpacked_destroy (&result);
      return 0;
    }

  *err = CMSG_OK;
  msgpack_unpacked_destroy (&result);
  return (int32_t)raw_value;
}

uint8_t
cmsg_get_ui8_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  return 0;
}

uint16_t
cmsg_get_ui16_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  return 0;
}

uint32_t
cmsg_get_ui32_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  return 0;
}

cmsg *
cmsg_get_nested_map_from_key (cmsg *msg, char *key, cmsg_err *err)
{
  if (!msg || !key || !err)
    {
      if (err)
        *err = CMSG_ERR_INVALID;
      return NULL;
    }

  msgpack_unpacked result;
  msgpack_unpacked_init (&result);

  bool ok
      = msgpack_unpack_next (&result, msg->sbuf->data, msg->sbuf->size, NULL);
  if (!ok || result.data.type != MSGPACK_OBJECT_MAP)
    {
      msgpack_unpacked_destroy (&result);
      *err = CMSG_ERR_PARSE;
      return NULL;
    }

  msgpack_object *val = private_get_map_object_from_key (&result.data, key);
  if (!val)
    {
      msgpack_unpacked_destroy (&result);
      *err = CMSG_ERR_KEY_NOT_FOUND;
      return NULL;
    }

  if (val->type != MSGPACK_OBJECT_MAP)
    {
      msgpack_unpacked_destroy (&result);
      *err = CMSG_ERR_TYPE_MISMATCH;
      return NULL;
    }

  cmsg *nested = cmsg_create ();
  if (!nested)
    {
      msgpack_unpacked_destroy (&result);
      *err = CMSG_ERR_ALLOC;
      return NULL;
    }

  msgpack_pack_object (nested->pk, *val);

  msgpack_unpacked_destroy (&result);
  *err = CMSG_OK;
  return nested;
}
