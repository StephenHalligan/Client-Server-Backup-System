#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pwd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Incorrect usage!\nUsage: [source directory] [filename]\n");
        return -1;
    }

    // Get the user ID
    uid_t uid = getuid();

    char *src_dir = argv[1];
    char *filename = argv[2];
    char dst_dir[1024] = "";
    char curr_dir[1024];

    // Get current directory
    getcwd(curr_dir, strlen(curr_dir));

    // Concatenate source directory to destination directory
    strcat(dst_dir, curr_dir);
    strcat(dst_dir, "/.Backup/");
    strcat(dst_dir, src_dir);


    // Get username
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL) {
        perror("getpwuid");
        exit(1);
    }
    char username[50];
    if (pw) {
        strcpy(username, pw->pw_name);
    }
    else {
        printf("Failed to get username\n");
        return EXIT_FAILURE;
    }

    // Check if user is correct user to access distribution
    if ((strcmp(username, "distribution") == 0) && (strcmp(src_dir, "Distribution") == 0)) {
        printf("\nUser authenticated successfully\n");
    }

    // Check if user is correct user to access manufacturing
    else if ((strcmp(username, "manufacturing") == 0) && (strcmp(src_dir, "Manufacturing") == 0)) {
        printf("\nUser authenticated successfully\n");
    }
    
    else if (strcmp(username, "admin") == 0) {
        printf("\nUser authenticated successfully\n");
    }
    // Else to end program if user is not valid
    else {
        printf("\nError: You do not have permission to access this server!\n\n");
        return 0;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    // Get time
    time_t now = time(NULL);
    char date[50];

    FILE *file;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    // Set the server address family and port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // Address error handling
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n\n");
        return -1;
    }

    // Connection success/failure error handling
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n*** Connection failed: check server status ***\n\n");
        return -1;
    }

    // Open log file
    file = fopen("../Logs/output.txt", "ab");

    // Write date to log
    strftime(date, sizeof(date), "%a %b %d %T %Y", localtime(&now));

    // Write user info and sent file info 
    fprintf(file, "\n%s\nClient: User '%d' (%s) sent '%s' from /%s/\n", date, uid, username, filename, src_dir);
    fclose(file);

    // Send file info to server via sockets
    send(sock, src_dir, strlen(src_dir), 0);
    send(sock, filename, strlen(filename), 0);
    send(sock, &uid, sizeof(uid), 0);


    // Open source file
    char filepath[2048];
    snprintf(filepath, sizeof(filepath), "%s/%s", src_dir, filename);
    FILE *fp = fopen(filepath, "r");
    // Error checking
    if (fp == NULL) {
        printf("\nError opening source file\n");
        return -1;
    }

    // Send file contents to server
    while (!feof(fp)) {
        int bytes_read = fread(buffer, 1, sizeof(buffer), fp);
        send(sock, buffer, bytes_read, 0);
    }

    printf("\nFile transfer complete\n\n");

    return 0;

}