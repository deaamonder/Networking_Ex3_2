#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "RUDP_API.h"

#define FILENAME "sample.txt"
#define RECEIVER_IP "10.0.2.15"
#define PORT 12345
char *util_generate_random_data(int size) {
    char *buffer = (char *)calloc(size, sizeof(char));
    if (buffer == NULL) {
        perror("calloc");
        exit(1);
    }
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

int main() {
    // Create sample file
    FILE *file = fopen(FILENAME, "wb"); // Open in write mode to create a new file
    if (file == NULL) {
        perror("Error creating file");
        exit(EXIT_FAILURE);
    }
    // Write random content to the file
    const char *file_content = util_generate_random_data(1024);
    size_t content_length = strlen(file_content);
    if (fwrite(file_content, 1, content_length, file) != content_length) {
        perror("Error writing to file");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    // Close the file
    fclose(file);

    // Open file for reading
    file = fopen(FILENAME, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer for file content
    char *file_buffer = (char *)malloc(file_size);
    if (file_buffer == NULL) {
        perror("Error allocating memory for file buffer");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Read file content into buffer
    if (fread(file_buffer, 1, file_size, file) != file_size) {
        perror("Error reading file");
        fclose(file);
        free(file_buffer);
        exit(EXIT_FAILURE);
    }

    // Close file
    fclose(file);

    // Create RUDP socket and perform handshake
    RUDP_Socket *rudp_socket = rudp_socket_and_handshake(RECEIVER_IP, PORT);

    // Send file data
    rudp_send(rudp_socket, file_buffer, file_size);

    // Close RUDP connection
    rudp_close(rudp_socket);

    // Free memory
    free(file_buffer);
    return 0;
}
