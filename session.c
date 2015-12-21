#include <stdlib.h>
#include <string.h>
#include "session.h"
#include "protocol.h"

static session *first=NULL;
static session *last=NULL;

session *create_session(int key)
{
	session *s=(session*)calloc(1,sizeof(session));
	s->key=key;
	if (last) last->next=s;
	last=s;
	if (!first) first=s;
	return s;
}

session *get_session(int key, session **prev)
{
	session *s;
	if (prev) *prev=NULL;
	for (s=first;s; s=s->next)
	{
		if (s->next && s->next->key==key && prev) *prev=s->next;
		if (s->key==key) break;
	};

	return s;
}


void free_session(session *s)
{
	session *p;
	get_session(s->key,&p);
	if (s->file) fclose(s->file);
	if (s->buf) free(s->buf);
	if (s->re) free(s->re);
	if (p) p->next=s->next;
	if (s==first) first=s->next;
	if (s==last) last=p;
	free(s);
}

void session_eat(session *s, char *data, size_t len)
{
	char *new_buf=(char*)calloc(1,s->buflen+len);
	if (s->buf) memmove(new_buf,s->buf,s->buflen);
	memmove(new_buf+s->buflen,data,len);
	free(s->buf);
	s->buf=new_buf;
	s->buflen+=len;
}

void session_respond_announce(session *s, struct in_addr host_addr)
{
	if (s->re) free(s->re);
	s->relen=29;
	s->re=(char*)calloc(1,s->relen);

	int i=0;
	unsigned char dest_mac[6];
	unsigned char seqno, battery_status;

	parse_header(s->buf,&i,dest_mac,&seqno,&battery_status);

	hash h;
	parse_values(s->buf, s->buflen, &i, &h);




}

session *create_session_writer(int key, const char *filename)
{
	session *s=(session*)calloc(1,sizeof(session));
	s->key=key;
	s->file=fopen(filename,"wb");
	if (last) last->next=s;
	last=s;
	if (!first) first=s;
	return s;
}

void close_session_writer(int key)
{
	session *p;
	session *s=get_session(key,&p);
	if (!s) return;
	fclose(s->file);
	if (p) p->next=s->next;
	if (s==first) first=s->next;
	if (s==last) last=p;
	free(s);
}

void close_all_writers(void)
{
	while (first)
		close_session_writer(first->key);
}

void write_session_data(int key, const void *data, size_t len)
{
	session *s=get_session(key,NULL);
	if (!s) return;
	fwrite(data,1,len,s->file);
}
