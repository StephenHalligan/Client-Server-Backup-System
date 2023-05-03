#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char buffer2[1024] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Get destination directory from client.c
    memset(buffer2, 0, sizeof(buffer));
    if ((valread = recv(new_socket, buffer, sizeof(buffer), 0)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    printf("\nConnection received and accepted");

    printf("\nSource directory received.\n");
    
    // Create destination directory
    char backup_dir[2048];

    // If backup directory doesn't exist, create it
    if (mkdir("../.Backup/", 0777) == -1) {
    }

    // Concat backup dir
    snprintf(backup_dir, sizeof(backup_dir), "../.Backup/%s/", buffer);

    // If final concatenated backup directory doesn't exist, create it
    if (mkdir(backup_dir, 0777) == -1) {
        printf("Backup directory found!\n");
    }
    else {
        printf("Backup directory not found, creating directory...\n");
    }

    // Get filename from client
    memset(buffer, 0, sizeof(buffer));
    if ((valread = recv(new_socket, buffer, sizeof(buffer), 0)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    printf("Backup file name received: %s\n", buffer);

    // Get user ID from client via socket
    int uid;
    if ((valread = recv(new_socket, &uid, sizeof(uid), 0)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    // Create log
    FILE *file;
    file = fopen("../Logs/output.txt", "ab");
    fprintf(file, "Server: File received and saved to %s%s\nServer: Transfer complete, terminating connection.\n", backup_dir, buffer);
    fclose(file);

    // Receive file contents from client and write to file
    char file_buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = recv(new_socket, file_buffer, sizeof(file_buffer), 0)) > 0) {
        // Create file with permissions for all users
        int fd = open(strcat(backup_dir, buffer), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        // Error checking for file write
        if (write(fd, file_buffer, bytes_read) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        memset(file_buffer, 0, sizeof(file_buffer));
        close(fd);

    }
    }
    
    return 0;

}