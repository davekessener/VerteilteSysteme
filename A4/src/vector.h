#ifndef VS_VECTOR_H
#define VS_VECTOR_H

#include <stdint.h>

typedef struct
{
	uint8_t *data;
	int i, c;
	size_t s;
} vector_t;

typedef void (*vector_cb)(void *);

void vector_init(vector_t *, size_t);
void vector_delete(vector_t *);
void vector_add(vector_t *, const void *);
void *vector_get(vector_t *, int);
void vector_clear(vector_t *);
void vector_foreach(vector_t *, vector_cb);

static inline int vector_size(const vector_t *self) { return self->i; }

#endif

