#include <string.h>
#include "protocol.h"

void parse_header(char *buf, int *pos, unsigned char *dest_mac, unsigned char *seqno, unsigned char *battery_status)
{
	struct {
		unsigned char signature[2];
		unsigned char packet_type[2];
		unsigned char dest_mac[6];
		unsigned char unk0a;
		unsigned char sn;
		unsigned char bs;
		unsigned char unk0;
	} __attribute__((__packed__)) *h = (void*)(buf+(*pos));

	memmove(dest_mac,h->dest_mac,sizeof(h->dest_mac));
	*seqno=h->sn;
	*battery_status=h->bs;
	*pos+=sizeof(*h);
}

void parse_value(char *buf, int *pos, value_descriptor **desc, void **value, int *value_size)
{
	*desc=(void*)(buf+(*pos));



}
