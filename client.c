// Tiny TCP chat client: sends stdin to server, prints server messages.
// Build: gcc client.c -o client
// Run:   ./client           (connects to 127.0.0.1:5555)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5555
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    printf("Connected to 127.0.0.1:%d\n", PORT);
    printf("Type messages and press Enter. Ctrl+C to quit.\n");

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);
        int max_fd = sockfd;

        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            break;
        }

        // Input from keyboard
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buf[BUF_SIZE];
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                // EOF (Ctrl+D)
                break;
            }
            size_t len = strlen(buf);
            if (len > 0) {
                if (send(sockfd, buf, len, 0) < 0) {
                    perror("send");
                    break;
                }
            }
        }

        // Data from server
        if (FD_ISSET(sockfd, &readfds)) {
            char buf[BUF_SIZE];
            ssize_t n = recv(sockfd, buf, sizeof(buf) - 1, 0);
            if (n <= 0) {
                if (n < 0) perror("recv");
                printf("Server closed connection\n");
                break;
            }
            buf[n] = '\0';
            printf("Peer: %s", buf);
            fflush(stdout);
        }
    }

    close(sockfd);
    return 0;
}
