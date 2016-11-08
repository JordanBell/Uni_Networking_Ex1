typedef struct data_header
{
  char* key;
  char* value;
} data_header;

typedef struct data_response
{

  // Optional pointer data
  data_header* p_content_length;

} data_response;

void init_response_data(data_response* p_data)
{
  // TODO Init data in ALL responses
}

void set_header(data_header* p_header, char* key, char* value)
{
  p_header->key = key;
  p_header->value = value;
}

void set_content_length(data_response* p_data, int length)
{
  o_p_data->content_length = (data_header*)malloc(sizeof(data_header));
  set_header(o_p_data, "content-length", itoa(length));
}

void send_response_data(char* dst, data_response* p_data)
{
  // TODO Build a string of the response data

}
