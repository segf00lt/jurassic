#ifndef JLIB_STR_H
#define JLIB_STR_H


#include "basic.h"
#include "arena.h"
//#include "context.h"


typedef struct Str8 Str8;
struct Str8 {
  u8 *s;
  s64 len;
};

typedef struct Str8_node Str8_node;
struct Str8_node {
  Str8 str;
  Str8_node *next;
};

typedef struct Str8_list Str8_list;
struct Str8_list {
  Str8_node *first;
  Str8_node *last;
  s64 count;
  s64 total_len;
};

#define str8_lit(strlit) ((Str8){ .s = (u8*)(strlit), .len = sizeof(strlit) - 1 })

b32 str8_match(Str8 a_str, Str8 b_str);
#define str8_match_lit(a_lit, b) str8_match(str8_lit(a_lit), b)
b32 str8_starts_with(Str8 str, Str8 start);
b32 str8_ends_with(Str8 str, Str8 end);
b32 str8_contains(Str8 str, Str8 substr);
s64 str8_find(Str8 haystack, Str8 needle);

Str8_list str8_split_by_string(Arena *a, Str8 str, Str8 sep);
#define str8_split_by_string_lit(a, str, sep) str8_split_by_string(a, str, str8_lit(sep))
Str8_list str8_split_by_chars(Arena *a, Str8 str, u8 *sep_chars, s64 n_sep_chars);
#define str8_split_by_chars_lit(a, str, sep_chars_lit) str8_split_by_chars(a, str, (u8*)sep_chars_lit, (s64)sizeof(sep_chars_lit))
Str8_list str8_split_by_char(Arena *a, Str8 str, u8 sep_char);

void str8_list_append_node_(Str8_list *list, Str8_node *node);
#define str8_list_append_node(list, node) str8_list_append_node_(&(list), node)

void str8_list_append_string_(Arena *a, Str8_list *list, Str8 str);
#define str8_list_append_string(a, list, str) str8_list_append_string_(a, &(list), str)

Str8_list push_str8_list_copy(Arena *a, Str8_list list);

Str8  push_str8_copy(Arena *a, Str8 str);
Str8  push_str8_copy_cstr(Arena *a, char *cstr);
Str8  push_str8fv(Arena *a, char *fmt, va_list args);
Str8  push_str8f(Arena *a, char *fmt, ...);
char* push_cstr_copy_str8(Arena *a, Str8 str);

b32 str8_is_cident(Str8 str);
b32 str8_is_alpha(Str8 str);
b32 str8_is_numeric(Str8 str, int base);
b32 str8_is_decimal(Str8 str);

Str8 str8_to_upper(Arena *a, Str8 str);
Str8 str8_to_lower(Arena *a, Str8 str);

#define is_upper(c) (!!('A' <= (c) && (c) <= 'Z'))
#define is_lower(c) (!!('a' <= (c) && (c) <= 'z'))
#define to_lower(c) (is_upper(c) ? ((c) - 'A' + 'a') : (c))
#define to_upper(c) (is_lower(c) ? ((c) - 'a' + 'A') : (c))
#define is_alpha(c) ('a' <= to_lower(c) && to_lower(c) <= 'z')
#define is_decimal(c) (!!('0' <= (c) && (c) <= '9'))
#define letter_index(c) ((s64)(to_lower(c) - 'a'))
#define hexdigit_to_int(c) ((s64)(is_alpha(c) ? (to_lower(c) - 'a' + 0xa) : (c - '0')))

#endif

#if defined(JLIB_STR_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_STR_IMPL
#endif

#include "stb_sprintf.h"
#define jlib_str_vsnprintf stbsp_vsnprintf

//#if defined(OS_WEB)
//#include <stdio.h>
//#define jlib_str_vsnprintf vsnprintf
//#else
//#endif

force_inline void str8_list_append_node_(Str8_list *list, Str8_node *node) {
  sll_queue_push(list->first, list->last, node);
  list->count++;
  list->total_len += node->str.len;
}

void str8_list_append_string_(Arena *a, Str8_list *list, Str8 str) {
  Str8_node *node = push_array_no_zero(a, Str8_node, 1);
  node->str = str;
  node->next = 0;
  sll_queue_push(list->first, list->last, node);
  list->count++;
  list->total_len += str.len;
}

Str8_list push_str8_list_copy(Arena *a, Str8_list list) {
  Str8_list result = {0};

  for(Str8_node *node = list.first; node; node = node->next) {
    Str8_node *new_node = push_array_no_zero(a, Str8_node, 1);
    new_node->str = node->str;
    new_node->next = 0;
    str8_list_append_node_(&result, new_node);
  }

  return result;
}

b32 str8_match(Str8 a, Str8 b) {
  if(a.len != b.len) {
    return 0;
  } else {
    return (b32)(memory_compare(a.s, b.s, a.len) == 0);
  }
}

b32 str8_contains(Str8 str, Str8 substr) {
  s64 found = str8_find(str, substr);
  b32 result = (found >= 0);
  return result;
}

s64 str8_find(Str8 haystack, Str8 needle) {
  s64 found = -1;

  for(s64 i = 0; i < haystack.len - needle.len; i++) {
    for(s64 j = 0; j < needle.len; j++) {
      if(haystack.s[i+j] != needle.s[j]) {
        goto end;
      }
    }

    found = i;
  }

end:

  return found;
}

b32 str8_starts_with(Str8 str, Str8 start) {
  b32 result = 0;

  if(str.len >= start.len) {
    Str8 str_start = str;
    str_start.len = start.len;
    result = str8_match(str_start, start);
  }

  return result;
}

b32 str8_ends_with(Str8 str, Str8 end) {
  b32 result = 0;

  if(str.len >= end.len) {
    Str8 str_end =
    {
      .s = str.s + str.len - end.len,
      .len = end.len,
    };
    result = str8_match(str_end, end);
  }

  return result;
}

b32 str8_is_cident(Str8 str) {
  b32 result = 1;

  if(!is_alpha(str.s[0]) && str.s[0] != '_') {
    result = 0;
  } else {

    for(int i = 1; i < str.len; i++) {
      if(!is_alpha(str.s[i]) && str.s[i] != '_' && !is_decimal(str.s[i])) {
        result = 0;
        break;
      }
    }

  }

  return result;
}

b32 str8_is_decimal(Str8 str) {
  b32 result = 1;

  for(int i = 0; i < str.len; i++) {
    if(!is_decimal(str.s[i])) {
      result = 0;
      break;
    }
  }

  return result;
}

Str8 push_str8_copy(Arena *a, Str8 str) {
  u8 *s = push_array_no_zero(a, u8, str.len + 1);
  memory_copy(s, str.s, str.len);
  s[str.len] = 0;
  return (Str8){ .s = s, .len = str.len };
}

force_inline Str8 push_str8_copy_cstr(Arena *a, char *cstr) {
  Str8 str = { .s = (u8*)cstr, .len = memory_strlen(cstr) };
  return push_str8_copy(a, str);
}

force_inline char* push_cstr_copy_str8(Arena *a, Str8 str) {
  Str8 s_ = push_str8_copy(a, str);
  char *s = (char*)s_.s;
  return s;
}

Str8 push_str8fv(Arena *a, char *fmt, va_list args) {
  va_list args2;
  va_copy(args2, args);
  u32 needed_bytes = jlib_str_vsnprintf(0, 0, fmt, args) + 1;
  Str8 result = {0};
  result.s = (u8*)arena_push(a, sizeof(u8) * needed_bytes, align_of(u8));
  result.len = jlib_str_vsnprintf((char*)result.s, needed_bytes, fmt, args2);
  result.s[result.len] = 0;
  va_end(args2);
  return result;
}

Str8 push_str8f(Arena *a, char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Str8 result = push_str8fv(a, fmt, args);
  va_end(args);
  return result;
}

Str8 str8_to_lower(Arena *a, Str8 str) {
  Str8 lower_str = push_str8_copy(a, str);

  for(int i = 0; i < lower_str.len; i++) {
    u8 c = lower_str.s[i];
    lower_str.s[i] = to_lower(c);
  }

  return lower_str;
}

Str8 str8_to_upper(Arena *a, Str8 str) {
  Str8 upper_str = push_str8_copy(a, str);

  for(int i = 0; i < upper_str.len; i++) {
    u8 c = upper_str.s[i];
    upper_str.s[i] = to_upper(c);
  }

  return upper_str;
}

Str8_list str8_split_by_chars(Arena *a, Str8 str, u8 *sep_chars, s64 n_sep_chars) {
  Str8_list result = {0};
  Str8_node head = {0};
  Str8_node *node = &head;

  s64 begin = 0;

  s64 i = 0;
  for(;i < str.len;) {
    b8 found_match = 0;

    for(s64 j = 0; j < n_sep_chars; j++) {
      if(str.s[i] == sep_chars[j]) {
        found_match = 1;
        break;
      }
    }

    if(found_match) {
      if(i == 0) {
        i += 1;
        begin = i;
      } else {
        node->next = push_array_no_zero(a, Str8_node, 1);
        node = node->next;
        node->str.s = str.s + begin;
        node->str.len = i - begin;
        node->next = 0;
        i += 1;
        begin = i;

        result.count++;
        result.total_len += node->str.len;
      }

    } else {
      i++;
    }

  }

  if(begin < i) {
    node->next = push_array_no_zero(a, Str8_node, 1);
    node = node->next;
    node->str.s = str.s + begin;
    node->str.len = i - begin;
    node->next = 0;

    result.count++;
    result.total_len += node->str.len;
  }

  result.first = head.next;
  result.last = node;

  return result;
}

force_inline Str8_list str8_split_by_char(Arena *a, Str8 str, u8 sep_char) {
  return str8_split_by_chars(a, str, &sep_char, 1);
}

Str8_list str8_split_by_string(Arena *a, Str8 str, Str8 sep) {
  Str8_list result = {0};
  Str8_node head = {0};
  Str8_node *node = &head;

  s64 begin = 0;

  s64 i = 0;
  for(;i < str.len;) {
    s64 j = 0;
    b8 found_match = 1;

    for(;j < sep.len && i+j < str.len; j++) {
      if(str.s[i+j] != sep.s[j]) {
        found_match = 0;
        break;
      }
    }

    if(found_match) {
      if(i == 0) {
        i += j;
        begin = i;
      } else {
        node->next = push_array_no_zero(a, Str8_node, 1);
        node = node->next;
        node->str.s = str.s + begin;
        node->str.len = i - begin;
        node->next = 0;
        i += j;
        begin = i;

        result.count++;
        result.total_len += node->str.len;
      }

    } else {
      i++;
    }

  }

  if(begin < i) {
    node->next = push_array_no_zero(a, Str8_node, 1);
    node = node->next;
    node->str.s = str.s + begin;
    node->str.len = i - begin;
    node->next = 0;

    result.count++;
    result.total_len += node->str.len;
  }

  result.first = head.next;
  result.last = node;

  return result;
}


#endif
