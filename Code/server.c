#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080

int main(int argc, char const *argv[]) {
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

    // Set server address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Receive destination directory from client
    memset(buffer2, 0, sizeof(buffer));
    if ((valread = recv(new_socket, buffer, sizeof(buffer), 0)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    printf("Source directory received.\n");
    
    // Create destination directory
    char backup_dir[2048];

    if (mkdir("../Backup/", 0777) == -1) {

    }
    snprintf(backup_dir, sizeof(backup_dir), "../Backup/%s/", buffer);

    if (mkdir(backup_dir, 0777) == -1) {
        printf("Backup directory found!\n");
    }

    // Receive file name from client
    memset(buffer, 0, sizeof(buffer));
    if ((valread = recv(new_socket, buffer, sizeof(buffer), 0)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    printf("Backup file name received: %s\n", buffer);

    // Receive file contents from client and write to file
    char file_buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = recv(new_socket, file_buffer, sizeof(file_buffer), 0)) > 0) {
        // Create file with read and write permissions for all users
        int fd = open(strcat(backup_dir, buffer), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (fd == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        if (write(fd, file_buffer, bytes_read) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        memset(file_buffer, 0, sizeof(file_buffer));
        close(fd);
        return 0;
    }
    }

