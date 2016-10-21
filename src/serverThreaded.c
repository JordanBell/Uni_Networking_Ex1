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
#include "linkedlist.h"

//#define DEBUG

pthread_mutex_t g_log_lock;
char* g_s_arg_filename_output = 0;
int g_i_message_counter = 0;
int g_socket_fd;	// Descriptor for the client's connection, established upon accept()
list g_all_connections;

void* handle_connection(void* p_context)
{
	c_connection_data* p_conn_data = (c_connection_data*)p_context;

	while(1)
	{
		// Wait for message
		memset(p_conn_data->m_received_message, 0, sizeof(p_conn_data->m_received_message));
		int result = 0;
		while((result = recv(p_conn_data->m_client_fd, p_conn_data->m_received_message, sizeof(p_conn_data->m_received_message), 0)) < 0);

		if(result <= 0)
		{
		#ifdef DEBUG
			printf("Connection to client lost.\n\n");
		#endif
			break; // Exit the while loop
		}

		// Message received. Remove any trailing newline
		int message_len = strlen(p_conn_data->m_received_message);
		if(message_len > 0 && p_conn_data->m_received_message[message_len-1] == '\n')
		{
			p_conn_data->m_received_message[message_len-1] = '\0';
		}

		// Echo the received message
#ifdef DEBUG
		printf("%d %s\n", g_i_message_counter, p_conn_data->m_received_message);
#endif

		// Write to file and inc the message counter (Critical section)
		pthread_mutex_lock(&g_log_lock);

		FILE* p_file_out_log = fopen(g_s_arg_filename_output, "a");
		if(p_file_out_log == 0)
		{
			perror("Failed to open log file.");
		}
		else
		{
			if(fprintf(p_file_out_log, "%d %s\n", g_i_message_counter, p_conn_data->m_received_message) <= 0)
			{
				perror("Failed to write to the log file.");
			}

			// Close the log file
			fclose(p_file_out_log);
			++g_i_message_counter;
			p_file_out_log = 0;
		}
		pthread_mutex_unlock(&g_log_lock);

		// Send an acknowledgment character to the client
		char* ack = "a";
		send(p_conn_data->m_client_fd, ack, sizeof(char), 0);
	}

	return 0;
}

void signal_handler(int signal_id)
{
	switch(signal_id)
	{
	case SIGINT:
	case SIGTERM:
	case SIGKILL:
		pthread_mutex_destroy(&g_log_lock);
		list_destroy(&g_all_connections);
		close(g_socket_fd);
		exit(0);
		break;
	default:
		break;
	}
}

void init_signal_handlers()
{
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
}

int init_server(char *argv[])
{
	// Get the arguments, port number and output filepath
	char* s_arg_port_no = argv[1];
	g_s_arg_filename_output = argv[2];
	int i_port_no = atoi(s_arg_port_no); // Convert the port number string into an int
	struct sockaddr_in server;

	// Clear the contents of the logfile
	FILE* p_file_log_overwritten = fopen(g_s_arg_filename_output, "w");
	if(p_file_log_overwritten)
	{
		fclose(p_file_log_overwritten);
	}

	g_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(g_socket_fd < 0)
	{
		perror("Failed to establish socket");
		return 1;
	}

	// Set SO_REUSEADDR to true
	int b_is_reuse_address = 1;
	socklen_t flag_len = sizeof(b_is_reuse_address);
	if(setsockopt(g_socket_fd, SOL_SOCKET, SO_REUSEADDR, &b_is_reuse_address, flag_len) < 0)
	{
		perror("Failed to setup socket options");
		return 1;
	}

	// Set up the the server at localhost, with the argument port number
  memset(&server, '0', sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(i_port_no);

	// Bind and listen
	if(bind(g_socket_fd, (struct sockaddr*) &server, sizeof(server)) < 0)
	{
		// Failed to bind directly
		perror("Failed to bind server to the socket");
		return 1;
	}

  if(listen(g_socket_fd, 10) < 0)
	{
		perror("Server failed to listen to the socket");
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	// Check for valid argument count
	if(argc != 3)
	{
		perror("Invalid number of arguments. Must be: <port> <output_filepath>");
		return 1;
	}

	// Initialisation
	init_signal_handlers();
	if(init_server(argv) != 0)
	{
		return 1;
	}

	init(&g_all_connections);
	if(pthread_mutex_init(&g_log_lock, 0))
	{
		perror("pthread_mutex_init");
		return 1;
	}

	while(1)
	{
		// Prepare new connection data
		c_connection_data* p_conn_data = (c_connection_data*)malloc(sizeof(c_connection_data));

		// Add the data to the list of connections
		append(&g_all_connections, p_conn_data); // Note: The list is now responsible for memory deallocation

		// Wait for a connection
		p_conn_data->m_client_fd = accept(g_socket_fd, (struct sockaddr*)NULL, NULL);
		if(p_conn_data->m_client_fd < 0)
		{
			perror("Error upon accepting client");
			return 0;
		}
		else // Connection established
		{
			// Create a thread to handle the connection
			if(pthread_create(&p_conn_data->m_thread, NULL, handle_connection, p_conn_data))
			{
				perror("pthread_create");
			}
		}
	}

	// Destroy
	pthread_mutex_destroy(&g_log_lock);
	list_destroy(&g_all_connections);

	return 0;
}
