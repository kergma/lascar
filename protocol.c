#include <string.h>
#include "protocol.h"

key keys[]={LP_SOMETHING, LP_SAMPLE_RATE, LP_COMM_INTERVAL, LP_SOURCE_MAC, LP_HOST_TIME, LP_DEVICE_NAME, LP_SEQ_NO, LP_SAMPLES_COUNT, LP_TIME_OFFSET, LP_DEVICE_TIME, LP_ALARM_DELAY, LP_FAHRENHEIT};

field fields[]={LP_BOOL, LP_RECORD, LP_VALUE};

datatype datatypes[]={LP_CHAR, LP_SHORTINT, LP_UNIXTIME, LP_STRING, LP_MACADDR};

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

#include <stdio.h>
int check_tag(void *tag, size_t tag_size, void *possible_tags, int possible_tags_count)
{
	for (int i=0;i<possible_tags_count;i++)
		if (!memcmp(tag,possible_tags+i*tag_size,tag_size)) return 1;
	return 0;
}
void parse_values(char *buf, size_t buflen, int *pos, hash *h)
{
	if (*pos+3 > buflen) return;

	if (!check_tag(&buf[0],sizeof(key),keys,sizeof(keys)/sizeof(keys[0]))) return;
	if (!check_tag(&buf[1],sizeof(field),fields,sizeof(fields)/sizeof(fields[0]))) return;
	if (!check_tag(&buf[2],sizeof(datatype),datatypes,sizeof(datatypes)/sizeof(datatypes[0]))) return;
	printf("here\n");

}
