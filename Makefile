all:
	gcc src/serverThreaded.c src/linkedlist.c -o serverThreaded -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread

run:
	fuser -k 8808/tcp; ./serverThreaded 8808
