#include <string.h>
#include <malloc.h>

#include "vector.h"

void vector_init(vector_t *self, size_t s)
{
	memset(self, 0, sizeof(*self));

	self->s = s;
}

void vector_delete(vector_t *self)
{
	free(self->data);
	memset(self, 0, sizeof(*self));
}

void vector_add(vector_t *self, const void *e)
{
	if(self->i == self->c)
	{
		self->data = realloc(self->data, (self->c = 2 * self->c + 1) * self->s);
	}

	memcpy(self->data + (self->i++) * self->s, e, self->s);
}

void *vector_get(vector_t *self, int i)
{
	return self->data + i * self->s;
}

void vector_clear(vector_t *self)
{
	free(self->data);
	self->data = NULL;
	self->i = self->c = 0;
}

void vector_foreach(vector_t *self, vector_cb f)
{
	int i;

	for(i = 0 ; i < self->i ; ++i)
	{
		f(self->data + i * self->s);
	}
}

