#ifndef PACKET_H
#define PACKET_H

#include "xmcomp/src/xmlfsm.h"

#include "jid.h"

typedef struct {
	char name, type;
	Jid real_from, proxy_to;
	BufferPtr header, inner;

	XmlAttr from_attr,
			to_attr,
			id_attr,
			type_attr;

	union {
		struct {
			BufferPtr muc, muc_user;
		} presence;
/*		struct {
			BufferPtr body;
		} message;*/
	} inner_nodes;
} IncomingPacket;

#define PACKET_CLEANUP_FROM 1
#define PACKET_CLEANUP_TO (1 << 1)
#define PACKET_CLEANUP_SWITCH_FROM_TO (PACKET_CLEANUP_FROM | (1 << 2))
#define PACKET_CLEANUP_ID (1 << 3)

void packet_cleanup(IncomingPacket *, int mode);
BOOL packet_parse(IncomingPacket *, BufferPtr *);

#endif
