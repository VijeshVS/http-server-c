#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Format:
// Type Path HTTP/Version
// Header1: value
// Header2: value
// ...
// \r\n
// Body

#define REQUEST_BUFFER_SIZE 1024
#define PORT 8080

struct Client_Request {
	char buffer[REQUEST_BUFFER_SIZE];
	struct sockaddr_in client_addr;
	int client_socket_fd;
};

struct Parsed_Request {
	char method[16];
	char path[256];
	char version[16];
	char headers_keys[512][512];
	char headers_values[512][512];
	int header_count;
	char body[256];
};

// TC: O(1)
int intialize_socket(){
	int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

	if (socket_fd == -1)
	{
		perror("Socket creation failed");
		exit(1);
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_len = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int bind_score = bind(socket_fd, (struct sockaddr *)&address, sizeof(address));
	if (bind_score == -1)
	{
		perror("Bind failed");
		exit(1);
	}
	listen(socket_fd, 5);

	return socket_fd;
}

// TC: O(n) where n is bytes read
struct Client_Request connect_client(int socket_fd){
	struct Client_Request client_req;
	socklen_t client_len = sizeof(client_req.client_addr);
	int newsocket_fd = accept(socket_fd, (struct sockaddr *)&client_req.client_addr, &client_len);

	if(newsocket_fd < 0){
		perror("accept failed");
		exit(1);
	}

	client_req.client_socket_fd = newsocket_fd;
	int bytes_read = read(newsocket_fd, client_req.buffer, sizeof(client_req.buffer) - 1);

	if (bytes_read > 0)
	{
		client_req.buffer[bytes_read] = '\0';
	}

	return client_req;
}

// TC: O(n) where n is request buffer size
struct Parsed_Request parse_request(struct Client_Request client_req){
	struct Parsed_Request parsed_req;

	char request_line[26];
	int i = 0;
	while(client_req.buffer[i] != '\r'){
		request_line[i] = client_req.buffer[i];
		i++;
	}
	request_line[i] = '\0';

	sscanf(request_line, "%s %s %s", parsed_req.method, parsed_req.path, parsed_req.version);

	int header_count = 0;
	i += 2; // Skip \r\n
	char headers[512][512];
	while(client_req.buffer[i] != '\r' && client_req.buffer[i+2] != '\r'){
		int k = 0;
		while(client_req.buffer[i] != '\r'){
			headers[header_count][k] = client_req.buffer[i];
			i += 1;
			k += 1;
		}
		headers[header_count][k] = '\0';
		header_count += 1;
		i += 2;
	}

	i += 2;
	
	for(int j = 0;j < header_count; j++){
		char key[256], value[256];
		sscanf(headers[j], "%[^:]: %[^\r]", key, value);
		strcpy(parsed_req.headers_keys[j], key);
		strcpy(parsed_req.headers_values[j], value);
	}

	int k = 0;
	while(client_req.buffer[i] != '\0'){
		parsed_req.body[k] = client_req.buffer[i];
		i += 1;
		k += 1;
	}

	parsed_req.body[k] = '\0';

	return parsed_req;
}

// TC: O(m) where m is file size for /home, O(1) for other paths
int serve_request(int client_socket_fd, char path[], char method[]){
	char response[5024];

	if(strcmp(path, "/") == 0 && strcmp(method, "GET") == 0){
		strcpy(response, "HTTP/1.1 200 OK\r\n"
						"Content-Type: text/plain\r\n"
						"\r\nHello, World!");
		write(client_socket_fd, response, strlen(response));
		return 1;
	}
	else if(strcmp(path, "/home") == 0 && strcmp(method, "GET") == 0){
		FILE *file = fopen("home.html", "r");
		if (file == NULL) {
			strcpy(response, "HTTP/1.1 500 Internal Server Error\r\n"
							"Content-Type: text/plain\r\n"
							"\r\nFailed to read home.html");
			write(client_socket_fd, response, strlen(response));
			return 0;
		}
		char file_content[5024];
		fread(file_content, 1, sizeof(file_content), file);
		file_content[sizeof(file_content) - 1] = '\0';
		fclose(file);

		strcpy(response, "HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"\r\n");
		strcat(response, file_content);
		write(client_socket_fd, response, strlen(response));
		return 1;
	}
	else {
		strcpy(response, "HTTP/1.1 404 Not Found\r\n"
						"Content-Type: text/plain\r\n"
						"\r\nPage Not Found");
		write(client_socket_fd, response, strlen(response));
		return 0;
	}
	return 0;
}

// TC: O(n) where n is total requests handled
int main()
{
	int socket_fd = intialize_socket();

	while(1){
		struct Client_Request client_req = connect_client(socket_fd);
		struct Parsed_Request parsed_req = parse_request(client_req);
		serve_request(client_req.client_socket_fd,parsed_req.path, parsed_req.method);
		close(client_req.client_socket_fd);
	}

	close(socket_fd);
}