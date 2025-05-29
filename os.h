#ifndef JLIB_OS_H
#define JLIB_OS_H


#include "basic.h"
#include "context.h"
#include "arena.h"
#include "str.h"


// NOTE these wrap malloc and free
// in future we'll do a virtual allocation scheme
void* os_alloc(u64 size);
void  os_free(void *ptr);

Str8 os_get_current_dir(void);
b32 os_set_current_dir(Str8 dir_path);
b32 os_set_current_dir_cstr(char *dir_path_cstr);

b32 os_move_file(Str8 old_path, Str8 new_path);
b32 os_remove_file(Str8 path);


#endif


#if defined(JLIB_OS_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_OS_IMPL
#endif

#if defined(OS_LINUX)

#include <limits.h>
#include <unistd.h>

#define OS_PATH_LEN PATH_MAX

#elif defined(OS_WEB)

#include <limits.h>
#include <unistd.h>

#define OS_PATH_LEN PATH_MAX

#elif defined(OS_MAC)

#include <limits.h>
#include <unistd.h>
#include <sys/param.h>

#define OS_PATH_LEN MAXPATHLEN

#elif defined(OS_WINDOWS)

#define OS_PATH_LEN MAX_PATH

#endif

b32 os_set_current_dir(Str8 dir_path) {
  b32 result = 0;

  scratch_scope() {
    char *dir_path_cstr = scratch_push_cstr_copy_str8(dir_path); 
    result = os_set_current_dir_cstr(dir_path_cstr);
  }

  return result;
}

#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_WEB)

void* os_alloc(u64 size) {
  return malloc(size);
}

void os_free(void *ptr) {
  free(ptr);
}


Str8 os_get_current_dir(void) {
  size_t buf_size = OS_PATH_LEN;

  Str8 result = {0};

  u64 pos = scratch_pos();
  char *buf = scratch_push_array_no_zero(char, buf_size);

  char *s = getcwd(buf, buf_size);

  while(s == NULL && buf_size < 4*OS_PATH_LEN) {
    scratch_pop_to(pos);
    buf_size <<= 1;
    buf = scratch_push_array_no_zero(char, buf_size);
    s = getcwd(buf, buf_size);
  }

  result.s = (u8*)s;
  result.len = memory_strlen(s);

  scratch_pop(buf_size - (result.len - 1));

  return result;
}

b32 os_set_current_dir_cstr(char *dir_path_cstr) {
  b32 result = 0;

  result = !chdir(dir_path_cstr);

  return result;
}

b32 os_move_file(Str8 old_path, Str8 new_path) {
  b32 result = 0;

  scratch_scope() {
    const char *old_path_cstr = scratch_push_cstr_copy_str8(old_path); 
    const char *new_path_cstr = scratch_push_cstr_copy_str8(new_path); 

    result = !rename(old_path_cstr, new_path_cstr);
  }

  return result;
}

b32 os_remove_file(Str8 path) {
  b32 result = 1;

  scratch_scope() {
    const char *path_cstr = scratch_push_cstr_copy_str8(path);
    if(remove(path_cstr) < 0) {
      result = 0;
    }
  }

  return result;
}

#elif defined(OS_WINDOWS)

#error "windows support not implemented"

Str8 os_get_current_dir(void) {
  DWORD buf_size = GetCurrentDirectory(0, NULL);
  ASSERT(buf_size > 0);

  char *buf = scratch_push_array_no_zero(char, buf_size);
  ASSERT(GetCurrentDirectory(buf_size, buf) != 0);

  Str8 result = { .s = buf, .s = buf_size - 1 };

  return result;
}

b32 os_set_current_dir_cstr(char *dir_path_cstr) {
  b32 result = 0;

  result = SetCurrentDirectory(dir_path_cstr);

  return result;
}

b32 os_move_file(Str8 old_path, Str8 new_path) {
  b32 result = 0;

  scratch_scope() {
    const char *old_path_cstr = scratch_push_cstr_copy_str8(old_path); 
    const char *new_path_cstr = scratch_push_cstr_copy_str8(new_path); 

    result = MoveFileEx(old_path_cstr, new_path_cstr, MOVEFILE_REPLACE_EXISTING);
  }

  return result;
}

b32 os_remove_file(Str8 path) {
  b32 result = 1;

  scratch_scope() {
    const char *path_cstr = scratch_push_cstr_copy_str8(path);
    if(!DeleteFileA(path_cstr)) {
      result = 0;
    }
  }

  return result;
}

#else

#error "unsupported operating system"

#endif


#endif
