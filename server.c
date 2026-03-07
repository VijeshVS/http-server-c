#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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
		printf("Waiting for a connection...\n");

		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int newsocket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);

		char buffer[1024];
		int bytes_read = read(newsocket_fd, buffer, sizeof(buffer) - 1);

		if (bytes_read > 0)
		{
			buffer[bytes_read] = '\0';
			printf("Received data: %s\n", buffer);
		}

		printf("\n");

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
		close(newsocket_fd);
	}

	close(socket_fd);
}
