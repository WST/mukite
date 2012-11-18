#include <string.h>

#include "xmcomp/logger.h"

#include "jid.h"

#include "builder.h"

#define BUF_PUSH(data, size) \
	{ \
		chunk_size = (size); \
		if (buffer->data_end + chunk_size > buffer->end) { \
			return FALSE; \
		} \
		memcpy(buffer->data_end, (data), chunk_size); \
		buffer->data_end += chunk_size; \
	}

#define BUF_PUSH_BUF(bptr) \
	BUF_PUSH((bptr).data, (bptr).size)

#define BUF_PUSH_STR(str) \
	BUF_PUSH((str), strlen(str))

#define BUF_PUSH_BPT(bptr) \
	BUF_PUSH((bptr).data, BPT_SIZE(&(bptr)))

#define BUF_PUSH_IFBPT(bptr) \
	{ if ((bptr).data) { BUF_PUSH_BPT(bptr) } }

#define BUF_PUSH_LITERAL(data) \
	BUF_PUSH(data, sizeof(data)-1)

static const char* affiliations[] = {
	"outcast",
	"none",
	"member",
	"admin",
	"owner"
};
static const int affiliation_sizes[] = {
	7,
	4,
	6,
	5,
	5
};

static const char* roles[] = {
	"visitor",
	"participant",
	"moderator"
};
static const int role_sizes[] = {
	7,
	11,
	9
};

BOOL build_presence_mucadm(MucAdmNode *node, BuilderBuffer *buffer) {
	int i, code, chunk_size;
	char code_str[3];

	BUF_PUSH_LITERAL("<x xmlns='http://jabber.org/protocol/muc#user'><item affiliation='");
	BUF_PUSH(affiliations[node->affiliation+1], affiliation_sizes[node->affiliation+1]);
	BUF_PUSH_LITERAL("' role='");
	BUF_PUSH(roles[node->role], role_sizes[node->role]);
	if (node->jid) {
		BUF_PUSH_LITERAL("' jid='");
		BUF_PUSH(JID_STR(node->jid), JID_LEN(node->jid));
	}
	if (node->nick.data) {
		BUF_PUSH_LITERAL("' nick='");
		BUF_PUSH_BPT(node->nick);
	}

	for (i = 0; i < node->status_codes_count && (code = node->status_codes[i]); ++i) {
		BUF_PUSH_LITERAL("'/><status code='");
		code_str[2] = code % 10 + '0';
		code /= 10;
		code_str[1] = code % 10 + '0';
		code /= 10;
		code_str[0] = code + '0';
		BUF_PUSH(code_str, 3);
	}

	BUF_PUSH_LITERAL("'/></x>");
	return TRUE;
}

BOOL build_stats(BuilderBuffer *buffer) {
	/*
<stat name='time/uptime'/>
<stat name='queue/overflows'/>
<stat name='queue/underflows'/>
<stat name='queue/realloc/enlarges'/>
<stat name='queue/realloc/shortens'/>
<stat name='queue/mallocs'/>
<stat name='queue/data/push'/>
<stat name='queue/data/pop'/>
<stat name='queue/free/push'/>
<stat name='queue/free/pop'/>
<stat name='cbuffer/underflows'/>
<stat name='cbuffer/overflows'/>
<stat name='cbuffer/writes'/>
<stat name='muc/rooms'/>
<stat name='muc/users'/>
<stat name='muc/jids'/>


<stat name='time/uptime' unit='seconds' value='1024'/>
	 */
	return FALSE;
}

BOOL build_room_items(BuilderBuffer *buffer, Room *room, Buffer *host) {
	int chunk_size;
	ParticipantEntry *participant = room->participants;

	for (; participant; participant = participant->next) {
		BUF_PUSH_LITERAL("<item name='");
		BUF_PUSH_BUF(participant->nick);
		BUF_PUSH_LITERAL("' jid='");
		BUF_PUSH_BUF(room->node);
		BUF_PUSH_LITERAL("@");
		BUF_PUSH_BUF(*host);
		BUF_PUSH_LITERAL("/");
		BUF_PUSH_BUF(participant->nick);
		BUF_PUSH_LITERAL("'/>");
	}

	return TRUE;
}

BOOL build_room_info(BuilderBuffer *buffer, Room *room, Buffer *host) {
	int chunk_size;

	BUF_PUSH_LITERAL("<identity category='conference' type='text' name='");
	if (room->title.size) {
		BUF_PUSH_BUF(room->title);
	} else {
		BUF_PUSH_BUF(room->node);
	}
	BUF_PUSH_LITERAL("'/><feature var='http://jabber.org/protocol/muc'/>");
	
	// TODO(artem): build items depending on room flags
	/*
		"<feature var='muc_public'/>"
		"<feature var='muc_persistent'/>"
		"<feature var='muc_open'/>"
		"<feature var='muc_semianonymous'/>"
		"<feature var='muc_moderated'/>"
		"<feature var='muc_unsecured'/>"
	*/

	return TRUE;
}

BOOL build_component_items(BuilderBuffer *buffer, Rooms *rooms, Buffer *host) {
	int chunk_size;
	char participants_count[20];
	Room *room = rooms->start;

	for (; room; room = room->next) {
		BUF_PUSH_LITERAL("<item name='");
		if (room->title.size) {
			BUF_PUSH_BUF(room->title);
		} else {
			BUF_PUSH_BUF(room->node);
		}
		sprintf(participants_count, " (%d)", room->participants_count);
		BUF_PUSH_STR(participants_count);
		BUF_PUSH_LITERAL("' jid='");
		BUF_PUSH_BUF(room->node);
		BUF_PUSH_LITERAL("@");
		BUF_PUSH_BUF(*host);
		BUF_PUSH_LITERAL("'/>");
	}

	return TRUE;
}

BOOL build_error(XMPPError *error, BuilderBuffer *buffer) {
	int chunk_size;

	BUF_PUSH_LITERAL("<error code='");
	BUF_PUSH_STR(error->code);
	BUF_PUSH_LITERAL("' type='");
	BUF_PUSH_STR(error->type);
	BUF_PUSH_LITERAL("'><");
	BUF_PUSH_STR(error->name);
	BUF_PUSH_LITERAL(" xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>"
			"<text xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'>");
	BUF_PUSH_STR(error->text);
	BUF_PUSH_LITERAL("</text></error>");

	return TRUE;
}

BOOL builder_build(BuilderPacket *packet, BuilderBuffer *buffer) {
	int chunk_size;

	LDEBUG("building packet: started");

	BUF_PUSH_BPT(packet->header);
	BUF_PUSH_LITERAL(" from='");
	if (packet->from_node.data) {
		BUF_PUSH_BUF(packet->from_node);
		BUF_PUSH_LITERAL("@");
	}
	BUF_PUSH_BUF(packet->from_host);
	if (packet->from_nick.data) {
		BUF_PUSH_LITERAL("/");
		BUF_PUSH_BUF(packet->from_nick);
	}
	BUF_PUSH_LITERAL("' to='");
	BUF_PUSH(JID_STR(&packet->to), JID_LEN(&packet->to));

	if (packet->type) {
		BUF_PUSH_LITERAL("' type='");
		switch (packet->type) {
			case 'g':
				if (packet->name == 'm') {
					BUF_PUSH_LITERAL("groupchat");
				} else {
					// iq
					BUF_PUSH_LITERAL("get");
				}
				break;
			case 'u':
				BUF_PUSH_LITERAL("unavailable");
				break;
			case 'c':
				BUF_PUSH_LITERAL("chat");
				break;
			case 'r':
				BUF_PUSH_LITERAL("result");
				break;
			case 'e':
				BUF_PUSH_LITERAL("error");
				break;
			case 's':
				BUF_PUSH_LITERAL("set");
				break;
		}
	}

	if (!packet->header.data &&
			!packet->user_data.data &&
			(packet->name == 'p')) {
		BUF_PUSH_LITERAL("'/>");
	} else {
		BUF_PUSH_LITERAL("'>");

		BUF_PUSH_IFBPT(packet->user_data);
		if (packet->type == 'e') {
			if (!build_error(packet->error, buffer)) {
				return FALSE;
			}
		} else {
			switch (packet->name) {
				case 'p':
					if (!build_presence_mucadm(&packet->participant, buffer)) {
						return FALSE;
					}
					break;
				case 'i':
					switch (packet->iq_type) {
						case BUILD_IQ_VERSION:
							BUF_PUSH_LITERAL(
									"<query xmlns='jabber:iq:version'>"
										"<name>Mukite http://mukite.org/</name>"
										"<version>git</version>"
										"<os>Windows-XP 5.01.2600</os>"
									"</query>");
							break;
						case BUILD_IQ_LAST:
							BUF_PUSH_LITERAL("<query xmlns='jabber:iq:last' seconds='");
							// We assume buffer is always large enough to hold int64
							buffer->data_end += sprintf(buffer->data_end, "%.0f", packet->iq_last.seconds);
							BUF_PUSH_LITERAL("'/>");
							break;
						case BUILD_IQ_TIME:
							BUF_PUSH_LITERAL("<time xmlns='xmpp:urn:time'><tzo>");
							BUF_PUSH_STR(packet->iq_time.tzo);
							BUF_PUSH_LITERAL("</tzo><utc>");
							BUF_PUSH_STR(packet->iq_time.utc);
							BUF_PUSH_LITERAL("</utc></time>");
							break;
						case BUILD_IQ_DISCO_INFO:
							BUF_PUSH_LITERAL(
									"<query xmlns='http://jabber.org/protocol/disco#info'>"
										"<identity category='conference' type='text' name='Mukite Chatrooms'/>"
										"<identity category='directory' type='chatroom' name='Mukite Chatrooms'/>"
										"<feature var='http://jabber.org/protocol/disco#info'/>"
										"<feature var='http://jabber.org/protocol/disco#items'/>"
										"<feature var='http://jabber.org/protocol/muc'/>"
										"<feature var='jabber:iq:register'/>"
										"<feature var='jabber:iq:last'/>"
										"<feature var='jabber:iq:version'/>"
										"<feature var='xmpp:urn:time'/>"
									"</query>");
							break;
						case BUILD_IQ_DISCO_ITEMS:
							BUF_PUSH_LITERAL("<query xmlns='http://jabber.org/protocol/disco#items'>");
							if (!build_component_items(buffer, packet->rooms, &packet->from_host)) {
								return FALSE;
							}
							BUF_PUSH_LITERAL("</query>");
							break;
						case BUILD_IQ_ROOM_DISCO_INFO:
							BUF_PUSH_LITERAL("<query xmlns='http://jabber.org/protocol/disco#info'>");
							if (!build_room_info(buffer, packet->room, &packet->from_host)) {
								return FALSE;
							}
							BUF_PUSH_LITERAL("</query>");
							break;
						case BUILD_IQ_ROOM_DISCO_ITEMS:
							BUF_PUSH_LITERAL("<query xmlns='http://jabber.org/protocol/disco#items'>");
							if (!build_room_items(buffer, packet->room, &packet->from_host)) {
								return FALSE;
							}
							BUF_PUSH_LITERAL("</query>");
							break;
					}
					break;
			}
		}
		switch (packet->name) {
			case 'm':
				BUF_PUSH_LITERAL("</message>");
				break;
			case 'p':
				BUF_PUSH_LITERAL("</presence>");
				break;
			case 'i':
				BUF_PUSH_LITERAL("</iq>");
				break;
		}
	}

	return TRUE;
}

BOOL builder_push_status_code(MucAdmNode *participant, int code) {
	if (participant->status_codes_count >= MAX_STATUS_CODES) {
		return FALSE;
	}
	participant->status_codes[participant->status_codes_count++] = code;
	return TRUE;
}
