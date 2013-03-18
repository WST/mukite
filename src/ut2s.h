#ifndef UT2S_H
#define UT2S_H

/*
 * ut2s: utlist, uthash + size & serialization
 */

#include <string.h>

#include "uthash/src/utlist.h"
#include "uthash/src/uthash.h"
#include "xmcomp/src/logger.h"

#include "serializer.h"

#define DLS_DECLARE(entry_type) \
	entry_type *head; \
	int size, max_size;

#define DLS_INIT(list, limit) \
	{ \
		memset((list), 0, sizeof(*list)); \
		(list)->max_size = (limit); \
	}

#define _DLS_EXPAND_WRAPPER(original, list, element) \
	{ \
		original((list)->head, element); \
		++(list)->size; \
	}

#define DLS_PREPEND(list, element) _DLS_EXPAND_WRAPPER(DL_PREPEND, list, element)
#define DLS_APPEND(list, element) _DLS_EXPAND_WRAPPER(DL_APPEND, list, element)

#define DLS_DELETE(list, element) \
	{ \
		DL_DELETE((list)->head, (element)); \
		--(list)->size; \
	}

#define DLS_CLEAR(list, destructor, type) \
	{ \
		type *current = 0, *tmp = 0; \
		DL_FOREACH_SAFE((list)->head, current, tmp) { \
			DLS_DELETE(list, current); \
			free(destructor(current)); \
		} \
	}

#define DLS_FOREACH(list, element) DL_FOREACH((list)->head, element)

#define _CONTAINER_SERIALIZE(container, iterator, type, entry_serializer) \
	{ \
		LDEBUG("serializing Container<" #type ">."); \
		type *__current__ = 0; \
		void *__presence_mark__ = (void*)1; \
		int __serialized_elements__ = 0; \
		iterator { \
			if (++__serialized_elements__ > (container)->max_size) { \
				LERROR("serializer: container size %d exceeds maximum size of %d. " \
						"Not serializing the rest to avoid deserialization problems.", \
						(container)->size, (container)->max_size); \
				break; \
			} \
			if (!SERIALIZE_BASE(__presence_mark__) || \
					!(entry_serializer(__current__, output))) { \
				LERROR("serializer: cannot write container item."); \
				return FALSE; \
			} \
		} \
		__presence_mark__ = 0; \
		if (!SERIALIZE_BASE(__presence_mark__)) { \
			LERROR("serializer: cannot write container finish mark."); \
			return FALSE; \
		} \
	}

#define DLS_SERIALIZE(list, type, entry_serializer) \
	_CONTAINER_SERIALIZE(list, DLS_FOREACH(list, __current__), type, entry_serializer)

#define HASHS_SERIALIZE(hash, type, entry_serializer) \
	_CONTAINER_SERIALIZE(hash, type *__tmp__ = 0; HASHS_ITER(hash, __current__, __tmp__), type, entry_serializer)

#define _L_DESERIALIZE(list, element, properties, backref) \
	if (!DESERIALIZE_BASE((list)->head)) { \
		LERROR("deserializer: cannot read list presence mark"); \
		return FALSE; \
	} \
	if (!(list)->head) { \
		LDEBUG("deserializer: the list is empty"); \
	} else { \
		(list)->head = element = 0; \
		(list)->size = 0; \
		do { \
			if (++(list)->size > (list)->max_size) { \
				LERROR("deserializer: list size limit %d exceeded, aborting", (list)->max_size); \
				return FALSE; \
			} \
			if (element) { \
				element->next = malloc(sizeof(*element)); \
				memset(element->next, 0, sizeof(*element)); \
				backref; \
				element = element->next; \
			} else { \
				(list)->head = element = malloc(sizeof(*element)); \
				memset(element, 0, sizeof(*element)); \
			} \
			if (!(properties) || !DESERIALIZE_BASE(element->next)) { \
				LERROR("deserializer: cannot read list item %d", (list)->size); \
				return FALSE; \
			} \
		} while (element->next); \
	}

#define DLS_DESERIALIZE(list, element, properties) \
	_L_DESERIALIZE(list, element, properties, element->next->prev = element)

#define HASHS_DESERIALIZE(hash, element, key, key_size, properties) \
	{ \
		void *__presence_mark__ = (void *)1; \
		if (!DESERIALIZE_BASE((hash)->head)) { \
			LERROR("deserializer: cannot read hash presence mark"); \
			return FALSE; \
		} \
		if (!(hash)->head) { \
			LDEBUG("deserializer: the hash is empty"); \
		} else { \
			(hash)->head = element = 0; \
			(hash)->size = 0; \
			do { \
				if (++(hash)->size > (hash)->max_size) { \
					LERROR("deserializer: hash size limit %d exceeded, aborting", (hash)->max_size); \
					return FALSE; \
				} \
				element = memset(malloc(sizeof(*element)), 0, sizeof(*element)); \
				if (!(properties) || !DESERIALIZE_BASE(__presence_mark__)) { \
					LERROR("deserializer: cannot read hash item %d", (hash)->size); \
					return FALSE; \
				} \
				HASH_ADD(hh, (hash)->head, key, key_size, element); \
			} while (__presence_mark__); \
		} \
	}

#define HASHS_ITER(hash, element, tmp) HASH_ITER(hh, (hash)->head, element, tmp)

#define HASHS_ADD(hash, key, key_size, element) \
	{ \
		HASH_ADD(hh, (hash)->head, key, key_size, element); \
		++(hash)->size; \
	}

#define HASHS_DEL(hash, element) \
	{ \
		HASH_DEL((hash)->head, element); \
		--(hash)->size; \
	}

#define HASHS_FIND(hash, key, size, element) HASH_FIND(hh, (hash)->head, key, size, element)

#endif
