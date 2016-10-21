#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

//#define DEBUG

int server_fd = 0;

void signal_handler(int signal_id)
{
	switch(signal_id)
	{
	case SIGINT:
	case SIGTERM:
	case SIGKILL:
	close(server_fd);
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	// Check for valid argument count
	if(argc < 3)
	{
		perror("Invalid number of arguments. Must be: <address> <port>");
		return 1;
	}

	// Setup signal handlers
	if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
#ifdef DEBUG
		printf("Warning: Cannot catch SIGINT\n");
#endif
	}

	if(signal(SIGTERM, signal_handler) == SIG_ERR)
	{
#ifdef DEBUG
		printf("Warning: Cannot catch SIGTERM\n");
#endif
	}
	signal(SIGKILL, signal_handler); // Will probably fail, so don't warn the user

	// Get the arguments, port number and output filepath
	char* s_arg_address = argv[1];
	char* s_arg_port_no = argv[2];
	int i_port_no = atoi(s_arg_port_no); // Convert the port number string into an int

	int socket_fd;
  struct sockaddr_in server;
	char* message = (char*)malloc(sizeof(char) * 256);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		perror("Failed to establish socket");
		return 1;
	}

	// Set up the connection to the server with the argument config
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(s_arg_address);
  server.sin_port = htons(i_port_no);

	// Connect
	server_fd = connect(socket_fd, (struct sockaddr*) &server, sizeof(server));
	if(server_fd < 0)
	{
		perror("Failed to connect to the server");
		return 1;
	}

	// Loop through stdin messages and send them.
	while(1)
	{
		do
		{
#ifdef DEBUG
			printf("Message: ");
#endif
			if(fgets(message, 256, stdin) == 0 && argc == 3)
			{
				return 0;
			}
		}
		while(message == 0);

		if(argc == 4 && (strcmp(message, "EXIT") == 0))
		{
			printf("Exit command reached.\n");
			break;
		}

		if(send(socket_fd, message, strlen(message), 0) < 0)
		{
			perror("Failed to send message to the server");
			return 1;
		}

		// Wait for acknowledgment from server
		char recv_value[1];
		//printf("Waiting for response... ");
		int recv_result;
		while((recv_result = recv(socket_fd, recv_value, sizeof(char), 0)) < 0);

		if(recv_result <= 0)
		{
			perror("Server connection lost");
			return 1;
		}
		if(recv_value[0] == 'a')
		{
			//printf("Received\n");
		}
		else if(recv_value[0] == 'c')
		{
			//printf("Server shutdown\n");
		}
	}

	return 0;
}
