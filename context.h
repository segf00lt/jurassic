#ifndef JLIB_CONTEXT_H
#define JLIB_CONTEXT_H


// TODO these files need to be separated in to .h .c
#include "arena.h"
#include "str.h"


void context_init(void);
void context_close(void);

void scratch_clear(void);

#define scratch_pos() arena_pos(context_scratch_arena)
#define scratch_pop_to(amount) arena_pop_to(context_scratch_arena, (pos))
#define scratch_pop(amount) arena_pop(context_scratch_arena, (amount))

#define scratch_push(size, align) arena_push(context_scratch_arena, (size), (align))
#define scratch_push_array_no_zero_aligned(T, n, align) (T *)arena_push(context_scratch_arena, sizeof(T)*(n), (align))
#define scratch_push_array_aligned(T, n, align) (T *)memory_zero(push_array_no_zero_aligned(context_scratch_arena, T, n, align), sizeof(T)*(n))
#define scratch_push_array_no_zero(T, n) push_array_no_zero_aligned(context_scratch_arena, T, n, MAX(8, align_of(T)))
#define scratch_push_array(T, n) push_array_aligned(context_scratch_arena, T, n, MAX(8, align_of(T)))

#define scratch_push_struct_no_zero(T) push_array_no_zero_aligned(context_scratch_arena, T, 1, MAX(8, align_of(T)))
#define scratch_push_struct(T) push_array_aligned(context_scratch_arena, T, 1, MAX(8, align_of(T)))

#define scratch_push_str8_copy(str) push_str8_copy(context_scratch_arena, (str))
#define scratch_push_str8_copy_cstr(cstr) push_str8_copy_cstr(context_scratch_arena, (char*)(cstr))

Str8 scratch_push_str8f(char *fmt, ...);
char* scratch_push_cstrf(char *fmt, ...);

#define scratch_push_cstr_copy_str8(str) push_cstr_copy_str8(context_scratch_arena, (str))

#define scratch_scope_begin() scope_begin(context_scratch_arena)
#define scratch_scope_end(scope) scope_end((scope))
#define scratch_scope() for(Arena_scope __scope__ = scope_begin(context_scratch_arena), __dummy_scope__ = {0}; !__dummy_scope__.pos; __dummy_scope__.pos += 1, scope_end(__scope__))

#define scratch_arr_init(arr) arr_init((arr), context_scratch_arena)

#endif


#if defined(JLIB_CONTEXT_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_CONTEXT_IMPL
#endif

thread_static Arena *context_scratch_arena;

force_inline void scratch_clear(void) {
  arena_clear(context_scratch_arena);
}

void context_init(void) {
  Arena_params arena_params = { .size = ARENA_DEFAULT_SIZE, .cannot_chain = 0, };
  context_scratch_arena = arena_alloc_(&arena_params);
}

void context_close(void) {
  arena_free(context_scratch_arena);
}

char* scratch_push_cstrf(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  Str8 result = push_str8fv(context_scratch_arena, fmt, args);
  va_end(args);
  return (char*)(result.s);
}

Str8 scratch_push_str8f(char *fmt, ...){
  va_list args;
  va_start(args, fmt);
  Str8 result = push_str8fv(context_scratch_arena, fmt, args);
  va_end(args);
  return result;
}

#endif

