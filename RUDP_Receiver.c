#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "RUDP_API.h"

#define PORT 12345
#define BUFFER_SIZE 1024

double get_elapsed_time(struct timeval start_time, struct timeval end_time)
{
    return (end_time.tv_sec - start_time.tv_sec) * 1000.0 +
           (end_time.tv_usec - start_time.tv_usec) / 1000.0;
}

int main() {
    int total_bytes_received = 0;
    int num_files_received = 0;
    double total_time = 0.0;

    printf("starting the server.... \n");

    // Create buffer for received data
    char buffer[BUFFER_SIZE];

    // Create RUDP socket and perform handshake
    RUDP_Socket *rudp_socket = rudp_socket_and_handshake("0.0.0.0", PORT);

    struct timeval startTime,endTime;
    gettimeofday(&startTime,NULL);
    
    // Receive data
    rudp_receive(rudp_socket, buffer, BUFFER_SIZE);

    gettimeofday(&endTime,NULL);
    total_time += get_elapsed_time(startTime,endTime);
    total_bytes_received += BUFFER_SIZE;
    num_files_received++;

    double average_time = total_time / num_files_received;
    double total_bandwidth = (total_time / total_bytes_received)*1000;
    printf("Average time to receive: %.6f milliseconds\n", average_time);
    printf("Total average bandwidth: %.2f bytes/ms\n", total_bandwidth);
    printf("------------------- \n");

    // Close RUDP connection
    rudp_close(rudp_socket);
    return 0;
}
