#ifndef ROOMS_H
#define ROOMS_H

#include <pthread.h>
#include <stdio.h>

#include "room.h"
#include "jid.h"
#include "acl.h"
#include "packet.h"
#include "registered_nick/registered_nicks.h"

typedef struct {
	struct {
		int min_message_interval,
			min_presence_interval;
	} limits;

	int size, max_size;
	Room *head;
	pthread_rwlock_t sync;

	RegisteredNicksList registered_nicks;
} Rooms;

void rooms_init(Rooms *);
void rooms_destroy(Rooms *);

Room *rooms_create(Rooms *, Jid *);

void rooms_process(Rooms *, IncomingPacket *, ACLConfig *);

BOOL rooms_serialize(Rooms *, FILE *);
BOOL rooms_deserialize(Rooms *, FILE *);

#endif
