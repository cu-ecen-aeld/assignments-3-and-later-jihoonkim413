#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include <sys/queue.h>
#include <sys/time.h>
#define PORT 9000

int main(int argc, char const* argv[])
{
    int status;
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
    
    if (status = getaddrinfo(NULL, "9000",&hints, &servinfo) !=0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // Creating socket file descriptor
        // freeaddrinfo(servinfo);
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket failed");
            exit(1);
        }
    
        // Forcefully attaching socket to the port 9000
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            perror("failed to set socket options");
            exit(1);
        }    

        // Forcefully attaching socket to the port 9000
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind failed");
            exit(1);
        }


        break;
    }
    freeaddrinfo(servinfo);

    if (listen(sockfd, 10) == -1) {
        perror("listen");
        exit(1);
    }    

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clientfd == -1 &&
            errno != EINTR) {
            close(sockfd);
            perror("accept");
            exit(1);
        }

        ssize_t valread;
        char buffer[1024] = { 0 };
        valread = read(clientfd, buffer,1024 - 1); // subtract 1 for the null
                                // terminator at the end
        printf("%s\n", buffer);
        size_t file_size = sizeof(buffer);
        send(clientfd, buffer, file_size,0);

    }

    // ssize_t valread;
    // char buffer[1024] = { 0 };
    // valread = read(clientfd, buffer,1024 - 1); // subtract 1 for the null
    //                           // terminator at the end
    // printf("%s\n", buffer);

    // freeaddrinfo(servinfo); //free the linked-list

    return 0;
}