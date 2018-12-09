#ifndef VS_AVG_H
#define VS_AVG_H

#include <stdint.h>

#define AVG_TYPE uint16_t

typedef struct
{
	AVG_TYPE v;
	size_t n;
} avg_t;

void avg_init(avg_t *, size_t);
void avg_reset(avg_t *, AVG_TYPE);
void avg_add(avg_t *, AVG_TYPE);

static inline AVG_TYPE avg_get(const avg_t *self) { return self->v; }

#endif

