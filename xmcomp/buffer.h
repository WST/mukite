#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

#include "common.h"

typedef struct {
	char *data;
	int size;
} Buffer;
#define B_EQ_LIT(literal, b) \
	(sizeof(literal)-1 == (b)->size && \
	 !memcmp((literal), (b)->data, sizeof(literal)-1))

BOOL buffer_serialize(Buffer *, FILE *);
BOOL buffer_deserialize(Buffer *, FILE *, int);

typedef struct {
	char *data, *end;
} BufferPtr;
#define BPT_SIZE(bptr) ((int)((bptr)->end - (bptr)->data))
#define BPT_EQ_LIT(literal, bptr) \
	(sizeof(literal)-1 == BPT_SIZE(bptr) && \
	 !memcmp((literal), (bptr)->data, sizeof(literal)-1))
#define BPT_2_B(bptr) \
	{ .data = (bptr)->data, .size = BPT_SIZE(bptr) }

BOOL buffer_ptr_serialize(BufferPtr *, FILE *);
BOOL buffer_ptr_deserialize(BufferPtr *, FILE *, int);

#endif