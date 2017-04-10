#ifndef __VECTOR__
#define __VECTOR__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_DEFAULT_SIZE       4
#define vector_t(type)            struct { size_t n, m; type *a; }
#define vector_init(v)            ((v).n = (v).m = 0, (v).a = 0)
#define vector_destroy(v)         if ((v).a) free((v).a)
#define vector_get(v, i)          ((v).a[(i)])
#define vector_set(v, i, x)       (v).a[i] = (x)
#define vector_size(v)            ((v).n)
#define vector_pop(v)             ((v).a[--(v).n])
#define vector_peek(v)            ((v).a[(v).n - 1])

#define vector_push(type, v, x)   do {                                                  \
                     if ((v).n == (v).m)                                                \
                     {                                                                  \
                         (v).m = (v).m ? (v).m << 1 : VECTOR_DEFAULT_SIZE;              \
                         (v).a = (type*) realloc((v).a, sizeof(type) * (v).m);          \
                     }                                                                  \
                     (v).a[(v).n++] = (x);                                              \
                 } while (0)               

#define vector_copy(type, v, dest) do {                                                 \
                     (dest).n = (v).n;                                                  \
                     (dest).m = (v).m;                                                  \
                     (dest).a = calloc((v).m, sizeof(type));                            \
                     memcpy((dest).a, (v).a, (v).m * sizeof(type));                     \
                 } while (0)

typedef vector_t(uint8_t) byte_r;

#endif