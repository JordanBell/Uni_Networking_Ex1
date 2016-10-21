all:
	gcc src/client.c -o client -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread
	gcc src/serverSingle.c -o serverSingle -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread
	gcc src/serverThreaded.c src/linkedlist.c -o serverThreaded -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread
