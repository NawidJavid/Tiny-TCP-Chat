// Tiny TCP chat server: accepts up to 2 clients and relays messages.
// Build: gcc server.c -o server
// Run:   ./server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5555
#define MAX_CLIENTS 2
#define BUF_SIZE 1024

int main(void) {
    int listen_fd, client_fds[MAX_CLIENTS];
    struct sockaddr_in addr;
    int i;

    for (i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0 (localhost capture works fine)
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        exit(1);
    }

    if (listen(listen_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(listen_fd);
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        fd_set readfds;
        int max_fd = listen_fd;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] != -1) {
                FD_SET(client_fds[i], &readfds);
                if (client_fds[i] > max_fd) {
                    max_fd = client_fds[i];
                }
            }
        }

        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            break;
        }

        // New connection
        if (FD_ISSET(listen_fd, &readfds)) {
            int new_fd = accept(listen_fd, NULL, NULL);
            if (new_fd < 0) {
                perror("accept");
            } else {
                int added = 0;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (client_fds[i] == -1) {
                        client_fds[i] = new_fd;
                        printf("Client %d connected (fd=%d)\n", i, new_fd);
                        added = 1;
                        break;
                    }
                }
                if (!added) {
                    printf("Too many clients, rejecting\n");
                    close(new_fd);
                }
            }
        }

        // Existing clients
        for (i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_fds[i];
            if (fd == -1) continue;

            if (FD_ISSET(fd, &readfds)) {
                char buf[BUF_SIZE];
                ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
                if (n <= 0) {
                    if (n < 0) perror("recv");
                    printf("Client %d disconnected (fd=%d)\n", i, fd);
                    close(fd);
                    client_fds[i] = -1;
                } else {
                    buf[n] = '\0';
                    printf("From client %d: %s", i, buf);

                    // Relay to other clients
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (j == i) continue;
                        if (client_fds[j] != -1) {
                            send(client_fds[j], buf, n, 0);
                        }
                    }
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}
