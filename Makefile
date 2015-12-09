test: test.c session.o protocol.o
	gcc -o test test.c session.o protocol.o
