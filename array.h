#ifndef JLIB_ARRAY_H
#define JLIB_ARRAY_H


#include "basic.h"
#include "arena.h"


typedef struct __Arr_header __Arr_header;
struct __Arr_header {
  void *d;
  s64 count;
  s64 cap;

  Arena *arena;
};

typedef struct __Slice_header __Slice_header;
struct __Slice_header {
  void *d;
  s64 count;
};

#define DECL_ARR_TYPE(T) \
  typedef struct Arr_##T Arr_##T; \
  struct Arr_##T {                 \
    T *d;                           \
    s64 count;                      \
    s64 cap;                        \
    Arena *arena;                   \
  };                                \

#define DECL_SLICE_TYPE(T) \
  typedef struct Slice_##T Slice_##T; \
  struct Slice_##T {                   \
    T *d;                               \
    s64 count;                          \
  };                                    \


#define ARRAY_DEFAULT_CAP 64


#define Arr(T)   Arr_##T
#define Slice(T) Slice_##T

#define header_from_arr(arr)         (*(__Arr_header*)(void*)(&(arr)))
#define header_from_arr_ptr(arr)     (*(__Arr_header*)(void*)(arr))
#define header_ptr_from_arr(arr)     ((__Arr_header*)(void*)(&(arr)))
#define header_ptr_from_arr_ptr(arr) ((__Arr_header*)(void*)(arr))

#define header_from_slice(slice)         (*(__Slice_header*)(void*)(&(slice)))
#define header_from_slice_ptr(slice)     (*(__Slice_header*)(void*)(slice))
#define header_ptr_from_slice(slice)     ((__Slice_header*)(void*)(&(slice)))
#define header_ptr_from_slice_ptr(slice) ((__Slice_header*)(void*)(slice))

#define arr_stride(array) ((s64)sizeof(*((array).d)))
#define arr_ptr_stride(array) ((s64)sizeof(*((array)->d)))

#define arr_init(array, arena) arr_init_(header_ptr_from_arr((array)), arena, arr_stride(array), ARRAY_DEFAULT_CAP)
#define arr_init_ex(array, arena, cap) arr_init_(header_ptr_from_arr((array)), arena, arr_stride(array), cap)

#define arr_push(array, elem) ((arr_push_no_zero_(header_ptr_from_arr((array)), arr_stride(array), 1)), (array).d[(array).count-1] = (elem))
#define arr_push_n_ptr(array, n) ((arr_push_no_zero_(header_ptr_from_arr((array)), arr_stride(array), (n))), &((array).d[(array).count - (n)]))
#define arr_push_n_index(array, n) ((arr_push_no_zero_(header_ptr_from_arr((array)), arr_stride(array), (n))), (s64)((array).count - (n)))

#define arr_pop(array)        ( ( ((array).count > 0) ? ((array).count--) : (0) ), (array).d[(array).count] )
#define arr_last(array) ((array).d[(array).count-1])

#define arr_to_slice(T, array) (*(Slice(T)*)(&(array)))
#define carray_to_slice(T, carray) ((Slice(T)){ .d = (T*)carray, .count = (sizeof(carray)/sizeof(T)) })

#define slice_last arr_last
#define slice_stride arr_stride

void  arr_init_(__Arr_header *arr, Arena *arena, s64 stride, s64 cap);
void* arr_push_no_zero_(__Arr_header *arr, s64 stride, s64 push_count);

// TODO
//
// I want to be able to declare something with Arr(T) and have the meta program scan all my source files and generate
// DECL_ARR(T) invocations in a generated file that will get included. I should also be able to specify some DECL_ARR(T)'s
// manually, that way the metaprogram could use our dynamic array as well.
//
// We need to set up a JAI style thread local context, containing allocators and other usefull stuff.
// That way these arrays could just use the context Arena allocator.
//
// Arrays are easier to reason about for certain applications than linked lists

#endif

#if defined(JLIB_ARRAY_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_ARRAY_IMPL
#endif


void arr_init_(__Arr_header *arr, Arena *arena, s64 stride, s64 cap) {
  arr->count = 0;
  arr->cap = cap;
  arr->arena = arena;
  arr->d = arena_push(arena, cap * stride, 1);
}

void* arr_push_no_zero_(__Arr_header *arr, s64 stride, s64 push_count) {
  ASSERT(arr->d && arr->cap && arr->arena);

  if(arr->count + push_count >= arr->cap) {
    s64 new_cap = arr->cap << 1;

    while(new_cap < arr->count + push_count) {
      new_cap <<= 1;
    }

    void *new_d = arena_push(arr->arena, new_cap * stride, 1);
    memory_copy(new_d, arr->d, stride * (arr->count + push_count));
    arr->d = new_d;
    arr->cap = new_cap;
  }

  void *result = (u8*)(arr->d) + arr->count;
  arr->count += push_count;

  return result;
}

/*

arrpop:
T arrpop(T* a)
Removes the final element of the array and returns it.

arrput:
T arrput(T* a, T b);
Appends the item b to the end of array a. Returns b.

arrins:
T arrins(T* a, int p, T b);
Inserts the item b into the middle of array a, into a[p],
moving the rest of the array over. Returns b.

arrinsn:
void arrinsn(T* a, int p, int n);
Inserts n uninitialized items into array a starting at a[p],
moving the rest of the array over.

arraddnptr:
T* arraddnptr(T* a, int n)
Appends n uninitialized items onto array at the end.
Returns a pointer to the first uninitialized item added.

arraddnindex:
size_t arraddnindex(T* a, int n)
Appends n uninitialized items onto array at the end.
Returns the index of the first uninitialized item added.

arrdel:
void arrdel(T* a, int p);
Deletes the element at a[p], moving the rest of the array over.

arrdeln:
void arrdeln(T* a, int p, int n);
Deletes n elements starting at a[p], moving the rest of the array over.

arrdelswap:
void arrdelswap(T* a, int p);
Deletes the element at a[p], replacing it with the element from
the end of the array. O(1) performance.

*/


#endif
