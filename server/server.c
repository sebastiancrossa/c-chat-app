#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <stdbool.h>

int sockets[5];
int sockfdin;


void sendAll(char* message, int skip);
void disconnect(int index);
void closeAll(int signum);
void *listener(void *data);


int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Invalid number of arguments\n");
		return 1;
	}

	// C-c handler
	signal(SIGINT, closeAll);

	//basic connection stuff
	struct sockaddr_in serverAddress;
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(argv[1]));
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	sockfdin = socket(AF_INET, SOCK_STREAM, 0);
	bind(sockfdin, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

	listen(sockfdin, 5);

	pthread_t newThread;
	while (true) {

		// Accept connection
		struct sockaddr_in clientAddress;
		int clientSize = sizeof(clientAddress);
		int clientSocket = accept(sockfdin, (struct sockaddr *)&clientAddress, (unsigned int *)&clientSize);

		bool success = false;
		// Search for an empty socket
		for (int i = 0; i < 5; i++) {
			if (sockets[i] == 0) {

			// Store the index to send it to the thread
			int* index = malloc(sizeof(int));
			*index = i;

			// Update the socket array
			sockets[i] = clientSocket;

			// Create thread that handles the connection
			pthread_create(&newThread, NULL, listener, (void *)index);
			success = true;

			break;
			}
		}

		if (!success) {
			char* m = "Chat full, try again later\n";
			write(clientSocket, m, strlen(m));
			close(clientSocket);
		}
	}

	pthread_exit(NULL);
}

void *listener(void *data) {

	// Cast and store the index
	int index = *(int *)data;
	free(data);

	// Get the socket of this connection
	int socketfd = sockets[index];

	//Read the username and greet the client
	char username[32];
	char send[255+64];
	read(socketfd, username, sizeof(username)-1);
	printf("%s se conecto\n", username);
	sprintf(send, "bienvenido %s\n", username);
	write(socketfd, send, strlen(send));

	sprintf(send, "%s se conecto\n", username);
	sendAll(send, index);
	char message[256];
	int rb;
	bool active = true;

	while (active && (rb = read(socketfd, message, sizeof(message) - 1))) {
		if (rb == 0) {
			break;
		}
		// The buffer could have trash beyond this
		message[rb] = '\0';

		// Check if the user write a special keyword
		if (strcmp(message, "bye\n") == 0) {
			printf("%s se desconecto\n", username);
			sprintf(send, "%s se desconecto\n", username);
			active = false;

		} else {
			printf("%s dijo %s",username,  message);
			sprintf(send, "%s: %s", username, message);
		}

		sendAll(send, index);
	}
	// Close the connection and clean the socket array
	disconnect(index);
	pthread_exit(NULL);
}

void sendAll(char* message, int skip) {
	int sizeM = strlen(message);
	for (int i = 0; i < 5; i++) {
		if (i == skip) {
			continue;
		}
		// If the socket is active write to it
		if (sockets[i] > 0) {
			write(sockets[i], message, sizeM);
		}
	}
}

void disconnect(int index) {

	// Send goodbye message
	char* message = "bye from server\n\0";
	write(sockets[index], message, strlen(message));
	// Close and clean the connection
	close(sockets[index]);
	sockets[index] = 0;

}

void closeAll(int signum) {

	for (int i = 0; i < 5; i++) {
		// If the socket is active close it
		if (sockets[i] > 0) {
			disconnect(i);
		}
	}
	// Close the server socket and exit
	close(sockfdin);
	exit(0);
}
