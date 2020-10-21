#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

// Compile and link with ptread
void *listener(void *socket) {

	int socketfd = *(int*)socket;

	char message[255];
	while (read(socketfd, &message, sizeof(message))) {
		/* write(fileno(stdout), &message, sizeof(message)); */
		printf("%s", message);
	}
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("Invalid number of argumet");
		return 1;
	}

	struct hostent *serverinfo = gethostbyname(argv[1]);
	struct sockaddr_in serverAddress;

	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(argv[2]));

	bcopy((char *)serverinfo->h_addr, (char *)&serverAddress.sin_addr, serverinfo->h_length);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

	char *username = argv[3];
	write(sockfd, &username, sizeof(username));

	pthread_t lis;
	pthread_create(&lis, NULL, listener, (void*)&sockfd);

	char message[255];

	while (read(fileno(stdin), &message, sizeof(message))) {
		write(sockfd, &message, sizeof(message));
		if (strcmp(message, "bye")) {
			close(sockfd);
			return 0;
		}
	}

	pthread_exit(NULL);
}
