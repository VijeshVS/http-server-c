#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(){
    int socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0){
        perror("socket");
        return 1;   
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        return 1;
    }

    char buffer[5024];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    while(1){
        int size = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if(size < 0){
            perror("recvfrom");
            return 1;
        }
        buffer[size] = '\0';
        printf("Received from client: %s\n", buffer);

        if(sendto(socket_fd, buffer, size, 0, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
            perror("sendto");
            return 1;
        }
    }
    
    close(socket_fd);
    return 0;   
}