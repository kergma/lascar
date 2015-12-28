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



size_t value_len(struct hash_item i)
{
	switch (i.t)
	{
		case LP_CHAR: return 1; break;
		case LP_SHORTINT: return 2; break;
		case LP_UNIXTIME: return 4; break;
		case LP_STRING: return strlen((char*)i.v)+1; break;
		case LP_MACADDR: return 6; break;
	};
	return 0;
}

#include <stdio.h>
void print_hash(hash *h)
{

	int k;
	struct hash_item i;
	size_t l;
	int b;
	for (i=(*h)[k=0];k<LP_LAST;i=(*h)[++k]) if (i.v) switch (k)
	{
		case LP_SAMPLE_RATE: printf("sample rate %d\n",*(int*)i.v); break;
		case LP_SOURCE_MAC: printf("source mac address %02x:%02x:%02x:%02x:%02x:%02x\n",*(unsigned char*)i.v,*(unsigned char*)(i.v+1),*(unsigned char*)(i.v+2),*(unsigned char*)(i.v+3),*(unsigned char*)(i.v+4),*(unsigned char*)(i.v+5)); break;
		case LP_DEVICE_NAME: printf("device name %s\n",(char*)i.v); break;
	default:
		l=value_len(i);
		printf("unprintable value 0x%x (%d bytes)",k,l);
		for (b=0;b<l;b++) printf(" %02x",*(char*)(i.v+b));
		printf("\n");
	};
}
int check_tag(void *tag, size_t tag_size, void *possible_tags, int possible_tags_count)
{
	for (int i=0;i<possible_tags_count;i++)
		if (!memcmp(tag,possible_tags+i*tag_size,tag_size)) return 1;
	return 0;
}

int parse_value(char *buf, size_t buflen, int *pos, value_descriptor *desc, void **value, size_t *_value_len)
{
	if (*pos+3 > buflen) return 0;

	char *b=buf+*pos;
	if (!check_tag(&b[0],sizeof(key),keys,sizeof(keys)/sizeof(keys[0]))) return 0;
	if (!check_tag(&b[1],sizeof(field),fields,sizeof(fields)/sizeof(fields[0]))) return 0;
	if (b[1]!=LP_RECORD && !check_tag(&b[2],sizeof(datatype),datatypes,sizeof(datatypes)/sizeof(datatypes[0]))) return 0;

	memcpy(desc, b, sizeof (*desc));
	b+=3;
	struct hash_item i={desc->f, desc->t, b};
	*_value_len=value_len(i);
	if (*pos+3+*_value_len>buflen) return 0;
	*value=(void*)b;
	*pos+=3+*_value_len;

	return 1;

}

void parse_values(char *buf, size_t buflen, int *pos, hash *h)
{
	value_descriptor d;
	void *v;
	size_t vl;
	while (parse_value(buf, buflen, pos, &d, &v, &vl))
	{
		struct hash_item i={d.f, d.t, v};
		(*h)[d.k]=i;
	};
}
