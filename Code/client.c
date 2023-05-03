#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Incorrect usage!\nUsage: %s [source directory] [filename]\n", argv[0]);
        return -1;
    }
    char *src_dir = argv[1];
    char *filename = argv[2];
    char dst_dir[1024] = "";
    char curr_dir[1024];

    // Get current directory
    getcwd(curr_dir, strlen(curr_dir));
    
    // Concatenate source directory to destination directory
    strcat(dst_dir, curr_dir);
    strcat(dst_dir, "/Backups/");
    strcat(dst_dir, src_dir);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    time_t now = time(NULL);
    char date[50];

    FILE *file;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    send(sock, src_dir, strlen(src_dir), 0);
    send(sock, filename, strlen(filename), 0);

    char filepath[2048];
    snprintf(filepath, sizeof(filepath), "%s/%s", src_dir, filename);
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("\nError opening source file\n");
        return -1;
    }

    // Send file contents to server
    while (!feof(fp)) {
        int bytes_read = fread(buffer, 1, sizeof(buffer), fp);
        send(sock, buffer, bytes_read, 0);
    }

    printf("File transfer complete\n");

    uid_t uid = getuid();

    file = fopen("../Logs/output.txt", "ab");
    strftime(date, sizeof(date), "%a %b %d %T %Y", localtime(&now));
    fprintf(file, "%s: %s moved from /%s to ../Backup/%s by user: %d\n", date, filename, src_dir, src_dir, uid);
    fclose(file);

    return 0;

}