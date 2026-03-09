#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Client Code
int main() {
    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0){
        perror("socket");
        return 1;   
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    char buffer[5024];
    printf("Enter a message to send to the server: ");
    fgets(buffer, sizeof(buffer), stdin);

    if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        return 1;
    }

    if(send(socket_fd, buffer, sizeof(buffer), 0) < 0){
        perror("send");
        return 1;
    }

    int size = recv(socket_fd, buffer, sizeof(buffer), 0);
    if(size < 0){
        perror("recv");
        return 1;
    }
    
    buffer[size] = '\0';
    printf("Received from server: %s\n", buffer);

    close(socket_fd);
    return 0;
}