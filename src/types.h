#include <pthread.h>

enum e_state
{
	e_state_free,
	e_state_connected,
	e_state_connection_lost,
};

struct c_connection_data
{
	pthread_t m_thread;
	int m_client_fd;
	char m_received_message[256];
};

typedef struct c_connection_data c_connection_data;
