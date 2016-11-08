#include <sys/socket.h>
#include <sys/stat.h>
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

pthread_mutex_t g_mutex;
int g_i_message_counter = 0;
int g_socket_fd;	// Descriptor for the client's connection, established upon accept()
list g_all_connections;

// buf needs to store 30 characters
int timespec_tostr(char *buf, uint len, struct timespec *ts)
{
    int ret;
    struct tm t;

    tzset();
    if (localtime_r(&(ts->tv_sec), &t) == NULL)
        return 1;

    ret = strftime(buf, len, "%F %T", &t);
    if (ret == 0)
        return 2;
    len -= ret - 1;

    ret = snprintf(&buf[strlen(buf)], len, ".%09ld", ts->tv_nsec);
    if (ret >= len)
        return 3;

    return 0;
}

int split_tochar(char** head, char** src, char c)
{
  char* chrreturn = strchr(*src, '\r');
  char* newline = strchr(*src, '\n');

	*head = *src;
	char* p_char_at = strchr(*src, c);

  // Find up to the nearest newline/carriage return
  if((newline != NULL) && (newline < p_char_at))
  {
    p_char_at = newline;
  }
  if((chrreturn != NULL) && (chrreturn < p_char_at))
  {
    p_char_at = chrreturn;
  }

	if(p_char_at != NULL)
	{
		*p_char_at = '\0';
		*src = p_char_at + 1;
    return 1;
	}
	else
	{
    return 0;
	}
}

char send_queue[2048];
void qsend_flush(int fd)
{
  #ifdef DEBUG
  printf("qsend_flush::%s\n", send_queue);
  #endif
	send(fd, send_queue, strlen(send_queue), 0);
  memset(send_queue, 0, 1024);
}

void qsend(int fd, char* message)
{
  #ifdef DEBUG
  printf("qsend::%s\n", message);
  #endif
  strncat(send_queue, message, strlen(message));
}

void* handle_connection(void* p_context)
{
	c_connection_data* p_conn_data = (c_connection_data*)p_context;

	while(1)
	{
    // Send any queued message
    if(strlen(send_queue) > 0)
    {
      qsend_flush(p_conn_data->m_client_fd);
    }

		// Wait for message
		memset(p_conn_data->m_received_message, 0, sizeof(p_conn_data->m_received_message));
		int result = 0;
		while((result = recv(p_conn_data->m_client_fd, p_conn_data->m_received_message, sizeof(p_conn_data->m_received_message), 0)) < 0);

		if(result == 0)
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

		// Parse the request
		char* word1;
		char* word2;
		char* word3;

		// Make a copy of the message for us to modify and point to
		char* message = (char*)malloc(sizeof(char) * 256);
		strncpy(message, p_conn_data->m_received_message, 256);

		int i;
		for(i = 0; message != 0 && i < 3; ++i)
		{
			switch(i)
			{
				case 0:
				{
					// Copy words of the message into the buffers
					split_tochar(&word1, &message, ' ');
				}
				break;
				case 1:
				{
					// Copy words of the message into the buffers
					split_tochar(&word2, &message, ' ');
				}
				break;
				case 2:
				{
					// Copy words of the message into the buffers
					if(split_tochar(&word3, &message, ' ') == 0)
          {
            if(split_tochar(&word3, &message, '\n') == 0)
            {
              split_tochar(&word3, &message, '\r');
            }
          }
				}
				break;
			}
		}

		if(i < 3)
		{
			// Terminate if there were fewer than 3 words copied over
			char* errmsg = "Invalid number of words\n";
			send(p_conn_data->m_client_fd, errmsg, strlen(errmsg), 0);
			continue;
		}

		if(strcmp(word1, "GET") != 0)
		{
			char* errmsg = "Not a get request\n";
			send(p_conn_data->m_client_fd, errmsg, strlen(errmsg), 0);
			continue;
		}

    char* str_filetype = strrchr(word2, '.');
		if((strcmp(word3, "HTTP/1.1") != 0) || (word2[0] != '/') || (str_filetype == NULL))
		{
      if((strcmp(word3, "HTTP/1.1") != 0))
      {
        printf("No http/1.1 : Found \"%s\" instead\n", word3);
      }
      if(word2[0] != '/')
      {
        printf("No /\n");
      }
      if(str_filetype == NULL)
      {
        printf("No filetype found\n");
      }
			qsend(p_conn_data->m_client_fd, "HTTP/1.1 400 Bad Request\n");
			continue;
		}

		// Forbid access to parent folders
		if(strstr(word2, "..") != 0)
		{
			// The user may be trying to access a parent folder
			qsend(p_conn_data->m_client_fd, "HTTP/1.1 403 Forbidden\n");
			continue;
		}

		// Get the file at the requested URL
		char* requested_filepath = (char*)malloc(sizeof(char) * 256);
		strcpy(requested_filepath, "files");
		strncat(requested_filepath, word2, 256);

		// Lock mutex for accessing files
		pthread_mutex_lock(&g_mutex);

		struct stat file_stat;
		int stat_result = stat(requested_filepath, &file_stat);

		pthread_mutex_unlock(&g_mutex);

		// Return 404 if the filepath has an error
		if(stat_result == -1)
		{
			// File does not exist
      printf("Failed to find %s\n", requested_filepath);
			qsend(p_conn_data->m_client_fd, "HTTP/1.1 404 Not Found\n");
			continue;
		}

		// TODO Lock mutex when accessing file!
		FILE* p_file_requested = fopen(requested_filepath, "r");

		if(p_file_requested == 0)
		{
			// File does not exist
			qsend(p_conn_data->m_client_fd, "HTTP/1.1 404 Not Found\n");
			continue;
		}

		// Send the OK message
		qsend(p_conn_data->m_client_fd, "HTTP/1.1 200 OK\n");

		// Send headers
		qsend(p_conn_data->m_client_fd, "Server: Localhost\n");
		qsend(p_conn_data->m_client_fd, "Strict-Transport-Security: max-age=31536000\n");

		// Get the date of modification
		char modification_date_str[30];
		timespec_tostr(modification_date_str, 30, &file_stat.st_mtim);
		char header_modification[128];
		sprintf(header_modification, "Last-Modified: %s\n", modification_date_str);
		qsend(p_conn_data->m_client_fd, header_modification);

		// Send file-type specific information
		if((strcmp(str_filetype, ".jpg") == 0) || (strcmp(str_filetype, ".png") == 0) || (strcmp(str_filetype, ".bmp") == 0))
    {
      char str_type[32];
      sprintf(str_type, "Content-Type: text/%s\n", str_filetype+1);
      qsend(p_conn_data->m_client_fd, str_type);

			char header_length[128];
			sprintf(header_length, "Content-Length: %jd\n", (intmax_t)file_stat.st_size);
			qsend(p_conn_data->m_client_fd, header_length);

      qsend_flush(p_conn_data->m_client_fd);

			char* str_contents = (char*)malloc(file_stat.st_size);
			pthread_mutex_lock(&g_mutex);
			while(fgets(str_contents, file_stat.st_size, p_file_requested))
			{
				send(p_conn_data->m_client_fd, str_contents, file_stat.st_size, 0);
				memset(str_contents, 0, file_stat.st_size);
			}
      fclose(p_file_requested);
      pthread_mutex_unlock(&g_mutex);
    }
    else
		{
			qsend(p_conn_data->m_client_fd, "Content-Type: text/html\n");

			char header_length[128];
			sprintf(header_length, "Content-Length: %jd\n", (intmax_t)file_stat.st_size);
			qsend(p_conn_data->m_client_fd, header_length);

      qsend_flush(p_conn_data->m_client_fd);

			char* str_contents = (char*)malloc(file_stat.st_size);
			pthread_mutex_lock(&g_mutex);
			while(fgets(str_contents, file_stat.st_size, p_file_requested))
			{
				qsend(p_conn_data->m_client_fd, str_contents);
				memset(str_contents, 0, file_stat.st_size);
			}
      fclose(p_file_requested);
      pthread_mutex_unlock(&g_mutex);
		}
#ifdef DEBUG
    printf("Sent\n");
#endif
	}

  qsend_flush(p_conn_data->m_client_fd);

	return 0;
}

void signal_handler(int signal_id)
{
	switch(signal_id)
	{
	case SIGINT:
	case SIGTERM:
	case SIGKILL:
		pthread_mutex_destroy(&g_mutex);
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
	int i_port_no = atoi(s_arg_port_no); // Convert the port number string into an int
	struct sockaddr_in server;

  // Open the socket
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
	if(argc != 2)
	{
		perror("Invalid number of arguments. Must be: <port>");
		return 1;
	}

	// Initialisation
	init_signal_handlers();
	if(init_server(argv) != 0)
	{
		return 1;
	}

  // Initialise the connection handler list
	init(&g_all_connections);

  // Init the file-read mutex
	if(pthread_mutex_init(&g_mutex, 0))
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

		// Wait for the next connection
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
	pthread_mutex_destroy(&g_mutex);
	list_destroy(&g_all_connections);

	return 0;
}
