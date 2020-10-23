#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

char* username;

// Compile and link with ptread
void *listener(void *socket) {

	int socketfd = *(int*)socket;

	// Send the user name
	write(socketfd, username, strlen(username));

	char message[256];
	int rb;
	while ((rb =read(fileno(stdin), message, sizeof(message) - 1))) {
		// The buffer has trash from previous messages from this point
		message[rb] = '\0';
		write(socketfd, message, strlen(message));
	}
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("Invalid number of argument");
		return 1;
	}

	// Basic connection stuff
	struct hostent *serverinfo = gethostbyname(argv[1]);
	struct sockaddr_in serverAddress;

	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(argv[2]));

	bcopy((char *)serverinfo->h_addr, (char *)&serverAddress.sin_addr, serverinfo->h_length);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Connection
	if(connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == 0) {

		// If connection is successful create the thread that listen the keyboard
		username = argv[3];
		pthread_t lis;
		pthread_create(&lis, NULL, listener, (void*)&sockfd);

		char message[256];

		// Receive message from the server
		int rb;
		while ((rb = read(sockfd, message, sizeof(message) - 1))) {
			// The buffer has trash from previous messages from this point
			message[rb] = '\0';
			printf(">%s", message);
		}
	}

	return 0;
}
