#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Format:
// Type / HTTP/Version
// Header1: value
// Header2: value
// ...
// \r\n
// Body

int main()
{
	int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

	if (socket_fd == -1)
	{
		printf("Failed to create a socket !!");
		return 1;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_len = sizeof(address);
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	int bind_score = bind(socket_fd, (struct sockaddr *)&address, sizeof(address));
	if (bind_score == -1)
	{
		printf("Failed to bind the socket !!");
		return 1;
	}
	listen(socket_fd, 5);

	while(1){
		printf("\nWaiting for a connection...\n");

		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int newsocket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);

		char buffer[1024];
		int bytes_read = read(newsocket_fd, buffer, sizeof(buffer) - 1);

		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
		}
		
		char request_line[26];
		int i = 0;
		while(buffer[i] != '\r'){
			request_line[i] = buffer[i];
			i++;
		}
		request_line[i] = '\0';

		char method[16], path[256], version[16];
		sscanf(request_line, "%s %s %s", method, path, version);

		char headers[512][512];
		int header_count = 0;
		i += 2; // Skip \r\n

		while(buffer[i] != '\r' && buffer[i+2] != '\r'){
			int k = 0;
			while(buffer[i] != '\r'){
				headers[header_count][k] = buffer[i];
				i += 1;
				k += 1;
			}
			headers[header_count][k] = '\0';
			header_count += 1;
			i += 2;
		}

		i += 2;

		// key: value
		char headers_keys[512][512];
		char headers_values[512][512];
		
		for(int j = 0;j < header_count; j++){
			char key[256], value[256];
			sscanf(headers[j], "%[^:]: %[^\r]", key, value);
			strcpy(headers_keys[j], key);
			strcpy(headers_values[j], value);
		}

		char parse_body[256];
		int k = 0;
		while(buffer[i] != '\0'){
			parse_body[k] = buffer[i];
			i += 1;
			k += 1;
		}

		parse_body[k] = '\0';

		if(strcmp(method, "GET") == 0 && strcmp(path, "/health") == 0){
			const char *body = "{\"message\": \"Hello from the server!\"}";
			char response[512];

			sprintf(response,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/json\r\n"
					"Content-Length: %lu\r\n"
					"\r\n"
					"%s",
					strlen(body), body);
			
			write(newsocket_fd, response, strlen(response));
		}
		else if(strcmp(method, "GET") == 0 && strcmp(path, "/home") == 0){
			// return home.html file
			FILE *file = fopen("home.html", "r");
			if (file == NULL) {
				const char *body = "{\"error\": \"File not found\"}";
				char response[512];
				sprintf(response,
						"HTTP/1.1 404 Not Found\r\n"
						"Content-Type: application/json\r\n"
						"Content-Length: %lu\r\n"
						"\r\n"
						"%s",
						strlen(body), body);
				write(newsocket_fd, response, strlen(response));
			} else {
				fseek(file, 0, SEEK_END);
				long file_size = ftell(file);
				fseek(file, 0, SEEK_SET);
				// do static assignment
				char file_content[5024];
				fread(file_content, 1, file_size, file);
				file_content[file_size] = '\0';
				char response[5024];
				sprintf(response,
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"Content-Length: %lu\r\n"
						"\r\n"
						"%s",
						strlen(file_content), file_content);
				write(newsocket_fd, response, strlen(response));
				fclose(file);
			}
		}
		else {
			const char *body = "{\"error\": \"Not Found\"}";
			char response[512];

			sprintf(response,
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Type: application/json\r\n"
					"Content-Length: %lu\r\n"
					"\r\n"
					"%s",
					strlen(body), body);
			
			write(newsocket_fd, response, strlen(response));
		}

		close(newsocket_fd);
	}

	close(socket_fd);
}
