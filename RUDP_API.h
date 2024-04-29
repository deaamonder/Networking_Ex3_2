
#ifndef RUDP_API_H
#define RUDP_API_H

typedef struct {
    int socket_fd;
    struct sockaddr_in addr;
} RUDP_Socket;

RUDP_Socket* rudp_socket_and_handshake(char* ip, int port);
void rudp_send(RUDP_Socket* rudp_socket, char* data, size_t size);
void rudp_receive(RUDP_Socket* rudp_socket, char* buffer, size_t buffer_size);
void rudp_close(RUDP_Socket* rudp_socket);

#endif 
