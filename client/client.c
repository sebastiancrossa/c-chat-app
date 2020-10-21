#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

typedef struct td {
	int hid;
	char *msg;
} ThreadData;

// Compile and link with ptread
void *hello(void *data) {
  ThreadData *td = (ThreadData *)data;
  printf("%s %d\n", td->msg, td->hid);
  free(data);
  pthread_exit(NULL);
}

int main() {
	struct hostent *serverinfo = gethostbyname(argv[1]);
	struct sockaddr_in serverAddress;

	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(2727);
	pthread_t newThread;
	for (int i = 0; i < 10; i++) {
		ThreadData *td = (ThreadData *)malloc(sizeof(ThreadData));
		td->msg = "Hello Thread";
		td->hid = i;
		pthread_create(&newThread, NULL, hello, (void *)td);
	}
	pthread_exit(NULL);
}
