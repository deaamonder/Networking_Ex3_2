#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "RUDP_API.h"

#define MAX_PACKET_SIZE 1024
#define TIMEOUT_SEC 5
#define MAX_RETRIES 5

// Calculate checksum including header
unsigned short int calculate_checksum(void *data, unsigned int bytes) {
    unsigned short int *data_pointer = (unsigned short int *)data;
    unsigned int total_sum = 0;

    // Add header checksum field to the sum
    total_sum += *((unsigned short int *)data_pointer);
    data_pointer++;

    // Main summing loop
    while (bytes > 1) {
        total_sum += *data_pointer++;
        bytes -= 2;
    }

    // Add left-over byte, if any
    if (bytes > 0)
        total_sum += *((unsigned char *)data_pointer);

    // Fold 32-bit sum to 16 bits
    while (total_sum >> 16)
        total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);

    return (~((unsigned short int)total_sum));
}

// Perform handshake
void perform_handshake(RUDP_Socket* rudp_socket) {
    // Send handshake packet
    char handshake_packet[] = "HANDSHAKE";
    sendto(rudp_socket->socket_fd, handshake_packet, strlen(handshake_packet), 0,
           (struct sockaddr*)&rudp_socket->addr, sizeof(rudp_socket->addr));

    // Wait for acknowledgment
    char ack_packet[MAX_PACKET_SIZE];
    ssize_t bytes_received;
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    while (1) {
        bytes_received = recvfrom(rudp_socket->socket_fd, ack_packet, MAX_PACKET_SIZE, 0,
                                  (struct sockaddr*)&sender_addr, &addr_len);
        if (bytes_received != -1) {
            // Acknowledgment received
            if (strncmp(ack_packet, "ACK", 3) == 0) {
                printf("Handshake successful. ACK received.\n");
                break;
            }
        } else {
            // Error receiving acknowledgment
            perror("Error receiving acknowledgment");
            exit(EXIT_FAILURE);
        }
    }
}

// Function to create RUDP socket and perform handshake
RUDP_Socket* rudp_socket_and_handshake(char* ip, int port) {
    RUDP_Socket* rudp_socket = (RUDP_Socket*)malloc(sizeof(RUDP_Socket));
    if (rudp_socket == NULL) {
        perror("Failed to allocate memory for RUDP socket");
        exit(EXIT_FAILURE);
    }

    // Create UDP socket
    if ((rudp_socket->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Failed to create UDP socket");
        exit(EXIT_FAILURE);
    }

    // Set up receiver address
    memset(&rudp_socket->addr, 0, sizeof(rudp_socket->addr));
    rudp_socket->addr.sin_family = AF_INET;
    rudp_socket->addr.sin_port = htons(port);
    if (inet_aton(ip, &rudp_socket->addr.sin_addr) == 0) {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }

   

    return rudp_socket;
}

// Function to send data over RUDP connection
void rudp_send(RUDP_Socket* rudp_socket, char* data, size_t size) {
    ssize_t bytes_sent;
    int attempt = 0;

    // Calculate checksum
    unsigned short int checksum = calculate_checksum(data, size);

    while (1) {
        // Send data
        bytes_sent = sendto(rudp_socket->socket_fd, data, size, 0,
                            (struct sockaddr*)&rudp_socket->addr,
                            sizeof(rudp_socket->addr));
        if (bytes_sent == -1) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }

        // Check for timeout
        if (++attempt > TIMEOUT_SEC) {
            printf("Timeout occurred. Retransmitting...\n");
            attempt = 0;
        }

        sleep(1); 
    }
}

// Function to receive data over RUDP connection
void rudp_receive(RUDP_Socket* rudp_socket, char* buffer, size_t buffer_size) {
    ssize_t bytes_received;
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    // Loop for handling retransmission attempts
    int attempt = 0;
    while (1) {
        // Receive data
        bytes_received = recvfrom(rudp_socket->socket_fd, buffer, buffer_size, 0,
                                  (struct sockaddr*)&sender_addr, &addr_len);
        if (bytes_received == -1) {
            perror("Error receiving data");
            exit(EXIT_FAILURE);
        }

        // Calculate checksum
        unsigned short int received_checksum = calculate_checksum(buffer, bytes_received);

        
            printf("Checksum verification failed. Packet corrupted. Retransmitting...\n");
            
            if (++attempt > MAX_RETRIES) {
                printf("Maximum retry attempts reached. Exiting...\n");
                exit(EXIT_FAILURE);
            }
           
            sleep(1);
       
    }
}

// Function to close RUDP connection
void rudp_close(RUDP_Socket* rudp_socket) {
    close(rudp_socket->socket_fd);
    free(rudp_socket);
}
