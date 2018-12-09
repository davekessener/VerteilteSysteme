#include <string.h>

#include "avg.h"

void avg_init(avg_t *self, size_t n)
{
	memset(self, 0, sizeof(*self));

	self->n = n;
}

void avg_reset(avg_t *self, AVG_TYPE v)
{
	self->v = v;
}

void avg_add(avg_t *self, AVG_TYPE v)
{
	unsigned t = (self->n - 1) * self->v + v;

	self->v = (AVG_TYPE) (t / self->n);
}

