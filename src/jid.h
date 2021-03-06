#ifndef JID_H
#define JID_H

#include "xmcomp/src/buffer.h"

typedef struct {
	BufferPtr node, host, resource;
} Jid;

#define MAX_JID_PART_SIZE 1023
#define MAX_JID_SIZE (MAX_JID_PART_SIZE * 3 + 2)

#define JID_NODE 1
#define JID_HOST 2
#define JID_RESOURCE 4
#define JID_FULL (JID_NODE | JID_HOST | JID_RESOURCE)
#define JID_CMP_NULLWC 8

BOOL jid_struct(BufferPtr *jid_string, Jid *jid_struct);

// When using NULLWC mode, jid1 can contain NULL for node/resource to indicate 'wildcard'
int jid_cmp(Jid *jid1, Jid *jid2, int mode);
int jid_strcmp(Jid *jid, Buffer *str, int part);
void jid_set(Jid *dst, Jid *src, int part);
void jid_destroy(Jid *jid);
void jid_init(Jid *jid);
BOOL jid_empty(Jid *jid);

BOOL jid_serialize(Jid *, FILE *);
BOOL jid_deserialize(Jid *, FILE *);

#define JID_STR(jid) ((jid)->node.data ? (jid)->node.data : (jid)->host.data)
#define JID_LEN(jid) ((int)( \
			(BPT_EMPTY(&(jid)->resource) ? (jid)->host.end : (jid)->resource.end) - \
			JID_STR(jid)))
#define JID_EMPTY(jid) (!(jid)->host.data)

#endif
