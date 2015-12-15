#ifndef SESSION_H
#define SESSION_H

#include <netdb.h>

#include <stdio.h>
#include <time.h>

typedef struct tag_session {
	int key;
	FILE *file;
	char *buf;
	size_t buflen;
	char *re;
	size_t relen;

	struct tag_session *next;

} session;

session *create_session(int key);
void free_session(session *s);
void session_eat(session *s, char *data, size_t len);
void session_respond_announce(session *s, struct in_addr host_addr);


session *create_session_writer(int key, const char *filename);
void write_session_data(int key, const void *data, size_t len);
void close_session_writer(int key);

#endif
