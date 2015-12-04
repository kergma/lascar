#include <stdlib.h>
#include "session.h"

static session *first=NULL;
static session *last=NULL;

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