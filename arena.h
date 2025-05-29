#ifndef JLIB_ARENA_H
#define JLIB_ARENA_H

#include "basic.h"

#define JLIB_ARENA_HEADER_SIZE 128

typedef struct Arena_params Arena_params;
struct Arena_params {
  u64 size;
  b32 cannot_chain;
  void *optional_backing_buffer;
};

typedef struct Arena Arena;
struct Arena {
  Arena *prev;
  Arena *cur;
  b32 cannot_chain;
  b32 has_backing_buffer;
  // TODO virtual memory
  //u64 reserve_size; // virtual memory reserved
  u64 size;  // actual memory committed
  u64 base_pos;
  u64 pos;
  u64 free_size;
  Arena *free_last;
};

STATIC_ASSERT(sizeof(Arena) <= JLIB_ARENA_HEADER_SIZE, arena_header_size_check);

typedef struct Arena_scope Arena_scope;
struct Arena_scope {
  Arena *arena;
  u64 pos;
};


global read_only u64 ARENA_DEFAULT_SIZE = KB(64);

Arena* arena_alloc_(Arena_params *params);
#define arena_alloc(...) arena_alloc_(&(Arena_params){ .size = ARENA_DEFAULT_SIZE, .cannot_chain = 0, __VA_ARGS__ })

void arena_free(Arena *arena);

void *arena_push(Arena *arena, u64 size, u64 align);
u64   arena_pos(Arena *arena);
void  arena_pop_to(Arena *arena, u64 pos);

void arena_clear(Arena *arena);
void arena_pop(Arena *arena, u64 amount);

Arena_scope scope_begin(Arena *arena);
void scope_end(Arena_scope scope);

#define push_array_no_zero_aligned(a, T, n, align) (T*)arena_push((a), sizeof(T)*(n), (align))
#define push_array_aligned(a, T, n, align) (T*)memory_zero(push_array_no_zero_aligned(a, T, n, align), sizeof(T)*(n))
#define push_array_no_zero(a, T, n) push_array_no_zero_aligned(a, T, n, MAX(8, align_of(T)))
#define push_array(a, T, n) push_array_aligned(a, T, n, MAX(8, align_of(T)))
#define push_struct(a, T) push_array(a, T, 1)
#define push_struct_no_zero(a, T) push_array_no_zero(a, T, 1)



#endif

#if defined(JLIB_ARENA_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_ARENA_IMPL
#endif


#include "os.h"


Arena* arena_alloc_(Arena_params *params) {
  u64 size = ALIGN_UP(params->size, align_of(void*));
  b32 cannot_chain = params->cannot_chain;
  b32 has_backing_buffer = 0;
  void *base = params->optional_backing_buffer;

  if(base) {
    cannot_chain = 1;
    has_backing_buffer = 1;
  } else {
    base = os_alloc(size);
    ASSERT(base);
  }

  Arena *arena = (Arena*)base;
  arena->cur = arena;
  arena->prev = 0;
  arena->cannot_chain = cannot_chain;
  arena->has_backing_buffer = has_backing_buffer;
  arena->size = size;
  arena->base_pos = 0;
  arena->pos = JLIB_ARENA_HEADER_SIZE;
  arena->free_size = 0;
  arena->free_last = 0;

  return arena;
}

void arena_free(Arena *arena) {
  ASSERT(arena);

  if(arena->has_backing_buffer) return;

  for(Arena *a = arena->free_last, *prev = 0; a != 0; a = prev) {
    prev = a->prev;
    os_free((void*)a);
  }

  for(Arena *a = arena->cur, *prev = 0; a != 0; a = prev) {
    prev = a->prev;
    os_free((void*)a);
  }

}

void *arena_push(Arena *arena, u64 size, u64 align) {
  ASSERT(arena);

  Arena *cur = arena->cur;
  u64 pos = ALIGN_UP(cur->pos, align);
  u64 new_pos = pos + size;

  if(cur->size < new_pos && !cur->cannot_chain) {
    Arena *new_arena = 0;

    Arena *prev_arena;

    for(new_arena = arena->free_last, prev_arena = 0; new_arena != 0; prev_arena = new_arena, new_arena = new_arena->prev) {

      if(new_arena->size >= ALIGN_UP(size, align)) {
        if(prev_arena) {
          prev_arena->prev = new_arena->prev;
        } else {
          arena->free_last = new_arena->prev;
        }
        break;
      }

    }

    if(new_arena == 0) {
      u64 new_arena_size = cur->size;

      if(size + JLIB_ARENA_HEADER_SIZE > new_arena_size) {
        new_arena_size = ALIGN_UP(size + JLIB_ARENA_HEADER_SIZE, align);
      }

      Arena_params params = { .size = new_arena_size };
      new_arena = arena_alloc_(&params);
    }

    new_arena->base_pos = cur->base_pos + cur->size;

    sll_stack_push_n(arena->cur, new_arena, prev);

    cur = new_arena;
    pos = ALIGN_UP(cur->pos, align);
    new_pos = pos + size;

  }

  void *result = (u8*)cur + pos;
  cur->pos = new_pos;

  return result;
}

u64 arena_pos(Arena *arena) {
  ASSERT(arena);

  Arena *cur = arena->cur;
  u64 pos = cur->base_pos + cur->pos;
  return pos;
}

void arena_pop_to(Arena *arena, u64 pos) {
  ASSERT(arena);

  u64 big_pos = CLAMP_BOT(JLIB_ARENA_HEADER_SIZE, pos);
  Arena *cur = arena->cur;

  for(Arena *prev = 0; cur->base_pos >= big_pos; cur = prev) {
    prev = cur->prev;
    cur->pos = JLIB_ARENA_HEADER_SIZE;
    sll_stack_push_n(arena->free_last, cur, prev);
  }

  arena->cur = cur;
  u64 new_pos = big_pos - cur->base_pos;
  ASSERT(new_pos <= cur->pos);
  cur->pos = new_pos;
}

void arena_clear(Arena *arena) {
  arena_pop_to(arena, 0);
}

void arena_pop(Arena *arena, u64 amount) {
  u64 old_pos = arena_pos(arena);
  u64 new_pos = old_pos;
  if(amount < old_pos) {
    new_pos = old_pos - amount;
  }
  arena_pop_to(arena, new_pos);
}

Arena_scope scope_begin(Arena *arena) {
  u64 pos = arena_pos(arena);
  Arena_scope scope = { arena, pos };
  return scope;
}

void scope_end(Arena_scope scope) {
  arena_pop_to(scope.arena, scope.pos);
}



#endif
