#ifndef JLIB_JSON_H
#define JLIB_JSON_H


#include "basic.h"
#include "arena.h"
#include "str.h"


#define JSON_VALUE_KINDS         \
  X(NULL)                        \
  X(OBJECT)                      \
  X(ARRAY)                       \
  X(STRING)                      \
  X(NUMBER)                      \
  X(BOOL)                        \


typedef struct JSON_parser JSON_parser;
typedef struct JSON_value JSON_value;

typedef enum JSON_value_kind {
  JSON_VALUE_KIND_INVALID = -1,
#define X(kind) JSON_VALUE_KIND_##kind,
  JSON_VALUE_KINDS
#undef X
    JSON_VALUE_KIND_MAX,
} JSON_value_kind;

char *JSON_value_kind_strings[JSON_VALUE_KIND_MAX] = {
#define X(kind) #kind,
  JSON_VALUE_KINDS
#undef X
};


struct JSON_value {
  JSON_value_kind kind;

  Str8        name;
  // TODO store first and last instead of value
  JSON_value *value;

  Str8 str;
  b32  boolean;
  s64  integer;
  f64  floating;
  s64  array_length;
  s64  object_child_count;

  JSON_value *parent;
  JSON_value *next;
  JSON_value *prev;
};

struct JSON_parser {
  Arena *arena;
  u8    *src;
  u8    *pos;
  u8    *end;
  s64    src_len;
  int    err;

  JSON_value *root;
};


void        json_init_parser(JSON_parser *p, Arena *arena, u8 *src, s64 src_len);
JSON_value* json_alloc_value(JSON_parser *p);
Str8        json_dump_to_str8(Arena *arena, JSON_value *root);
JSON_value* json_parse(JSON_parser *p);
void        json_parse_skip_whitespace(JSON_parser *p);
JSON_value* json_parse_object(JSON_parser *p);
JSON_value* json_parse_array(JSON_parser *p);
JSON_value* json_parse_value(JSON_parser *p);
JSON_value* json_parse_string(JSON_parser *p);
JSON_value* json_parse_number(JSON_parser *p);
JSON_value* json_parse_true(JSON_parser *p);
JSON_value* json_parse_false(JSON_parser *p);
JSON_value* json_parse_null(JSON_parser *p);
Str8        json_parse_raw_string(JSON_parser *p);


#endif

#if defined(JLIB_JSON_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_JSON_IMPL
#endif

void json_init_parser(JSON_parser *p, Arena *arena, u8 *src, s64 src_len) {
  p->arena = arena;
  p->src = src;
  p->src_len = src_len;
  p->pos = src;
  p->end = src + src_len;
  p->err = 0;
  p->root = 0;
}

force_inline JSON_value* json_alloc_value(JSON_parser *p) {
  ASSERT(p->arena);
  return push_array_no_zero(p->arena, JSON_value, 1);
}

//Str8 json_dump_to_str8(Arena *arena, JSON_value *root) {
//  Arena_save arena_save = arena_to_save(arena);
//
//
//}

JSON_value* json_parse(JSON_parser *p) {
  p->root = json_parse_object(p);
  return p->root;
}

force_inline void json_parse_skip_whitespace(JSON_parser *p) {
  while(p->pos < p->end) {
    switch(*p->pos) {
      default:
        return;
      case ' ':
      case '\n':
      case '\r':
      case '\t':
        break;
    }
    p->pos++;
  }
}

JSON_value* json_parse_object(JSON_parser *p) {
  if(*p->pos != '{') {
    return NULL;
  }
  p->pos++;

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_OBJECT;

  if(*p->pos == '}') {
    p->pos++;
    return result;
  } 

  JSON_value head;
  JSON_value *list = &head;


  s64 object_child_count = 0;
  for(;p->pos < p->end && *p->pos != '}'; object_child_count++) {
    json_parse_skip_whitespace(p);

    Str8 pair_name = json_parse_raw_string(p);

    if(p->err) {
      return NULL;
    }

    json_parse_skip_whitespace(p);

    if(*p->pos != ':') {
      p->err = 1;
      return NULL;
    }
    p->pos++;

    JSON_value *pair_value = json_parse_value(p);

    if(!pair_value) {
      p->err = 1;
    }

    if(p->err) {
      return NULL;
    }

    pair_value->name = pair_name;
    pair_value->parent = result;

    if(list == &head) {
      pair_value->prev = NULL;
    } else {
      pair_value->prev = list;
    }

    list->next = pair_value;
    list = list->next;

    if(*p->pos == ',') {
      p->pos++;
    }

  }

  result->value = head.next;
  result->object_child_count = object_child_count;

  if(*p->pos != '}') {
    p->err = 1;
    return NULL;
  }
  p->pos++;

  return result;
}

JSON_value* json_parse_array(JSON_parser *p) {
  if(*p->pos != '[') {
    return NULL;
  }
  p->pos++;

  json_parse_skip_whitespace(p);

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_ARRAY;

  if(*p->pos == ']') {
    p->pos++;
    return result;
  } 

  JSON_value head;
  JSON_value *list = &head;

  s64 array_length = 0;
  for(;p->pos < p->end && *p->pos != ']'; array_length++) {
    JSON_value *element_value = json_parse_value(p);

    if(!element_value) {
      p->err = 1;
    }

    if(p->err) {
      return NULL;
    }

    element_value->parent = result;

    if(list == &head) {
      element_value->prev = NULL;
    } else {
      element_value->prev = list;
    }

    list->next = element_value;
    list = list->next;

    if(*p->pos == ',') {
      p->pos++;
    }

  }

  result->value = head.next;
  result->array_length = array_length;

  if(*p->pos != ']') {
    p->err = 1;
    return NULL;
  }
  p->pos++;

  return result;
}

JSON_value* json_parse_value(JSON_parser *p) {
  json_parse_skip_whitespace(p);

  JSON_value *result = NULL;

  if(!result) result = json_parse_string(p);
  if(!result) result = json_parse_number(p);
  if(!result) result = json_parse_object(p);
  if(!result) result = json_parse_array(p);
  if(!result) result = json_parse_true(p);
  if(!result) result = json_parse_false(p);
  if(!result) result = json_parse_null(p);

  if(!result) {
    p->err = 1;
    return NULL;
  }

  json_parse_skip_whitespace(p);

  return result;
}

JSON_value* json_parse_string(JSON_parser *p) {
  Str8 str = json_parse_raw_string(p);

  if(str.len == 0) {
    return NULL;
  }

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_STRING;
  result->str = str;

  return result;
}

Str8 json_parse_raw_string(JSON_parser *p) {
  if(*p->pos != '"') {
    return (Str8){0};
  }
  p->pos++;

  u8 *begin = p->pos;

  u8 *end = begin;

  while(end < p->end && *end != '"') {
    if(*end == '\\') end++;
    end++;
  }

  if(end >= p->end) {
    p->err = 1;
    return (Str8){0};
  }

  if(begin == end) {
    return (Str8){0};
  }

  s64 len = (s64)(end - begin);
  Str8 result = { .s = (u8*)push_array_no_zero(p->arena, u8, len), .len = len };
  u8 *src = begin;
  s64 r = 0;
  s64 w = 0;
  s64 n = (s64)(end - begin);
  ASSERT(n >= 0);

  while(r < n) {
    if(src[r] == '\\') {
      r++;
      char c = src[r];

      switch(c) {
        case '"':
          result.s[w] = '"';
          break;
        case '\\':
          result.s[w] = '\\';
          break;
        case '/':
          result.s[w] = '/';
          break;
        case 'b':
          result.s[w] = '\b';
          break;
        case 'f':
          result.s[w] = '\f';
          break;
        case 'n':
          result.s[w] = '\n';
          break;
        case 'r':
          result.s[w] = '\r';
          break;
        case 't':
          result.s[w] = '\t';
          break;
        case 'u':
          {
            PANIC("hex code in string unimplemented");
          } break;
      }
    } else {
      result.s[w] = src[r++];
    }

    w++;
  }

  result.len = w;

  end++;
  p->pos = end;

  return result;
}

JSON_value* json_parse_number(JSON_parser *p) {
  u8 *end = NULL;
  f64 floating = strtod((char*)p->pos, (char**)&end);

  if(floating == 0.0f && end == p->pos) {
    return NULL;
  }

  ASSERT(end > p->pos);

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_NUMBER;
  result->integer = (s64)floating;
  result->floating = floating;

  p->pos = end;

  return result;
}

JSON_value* json_parse_true(JSON_parser *p) {
  char s[] = "true";

  for(int i = 0; i < STRLEN(s); i++) {
    if(p->pos[i] != s[i]) {
      return NULL;
    }
  }

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_BOOL;
  result->boolean = 1;

  p->pos += STRLEN(s);

  return result;
}

JSON_value* json_parse_false(JSON_parser *p) {
  char s[] = "false";

  for(int i = 0; i < STRLEN(s); i++) {
    if(p->pos[i] != s[i]) {
      return NULL;
    }
  }

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_BOOL;
  result->boolean = 0;

  p->pos += STRLEN(s);

  return result;
}

JSON_value* json_parse_null(JSON_parser *p) {
  char s[] = "null";

  for(int i = 0; i < STRLEN(s); i++) {
    if(p->pos[i] != s[i]) {
      return NULL;
    }
  }

  JSON_value *result = json_alloc_value(p);
  result->kind = JSON_VALUE_KIND_NULL;

  p->pos += STRLEN(s);

  return result;
}

#endif
