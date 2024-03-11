#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>

#define DAEMON_KEY          "-d"
#define PORT                "9000"
#define DATA_FILE_NAME      "/var/tmp/aesdsocketdata"
#define RECV_BUFFER_LEN     512
#define BACKLOG 10   // how many pending connections queue will hold

bool is_active = true;
int sockfd = -1;
int client_fd = -1;

static void remove_data_file(void){
    if(remove(DATA_FILE_NAME) == 0){
        syslog(LOG_INFO, "Removed data file");
    } else {
        syslog(LOG_ERR, "Error removing data file: %s", strerror(errno));
    }
}

static void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void signal_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

int main(int argc, char **argv){

    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa = {0};
    int res;
    int return_val = 0;
    char addr_str[INET6_ADDRSTRLEN];
    char recv_buffer[RECV_BUFFER_LEN] = {0};
    int rv;
    int yes=1;
    bool start_in_daemon = false;

    openlog(argv[0], LOG_PID, LOG_USER);

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    bool daemon_mode = 0;

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    if (daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS); // Parent process exits
        }
        // Child process continues
        umask(0); // Unmask the file mode
    }

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }


    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        freeaddrinfo(servinfo);
        close(sockfd);
        sockfd = -1;
        remove_data_file();
        closelog();
        return return_val;
    }

    while(is_active){
        struct sockaddr_storage client_addr;

        client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &(socklen_t){sizeof(client_addr)});
        if (client_fd == -1){
            syslog(LOG_ERR, "accept error: %s", strerror(errno));
            continue;
        }

        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), addr_str, sizeof(addr_str));

        syslog(LOG_INFO, "Accepted connection from %s", addr_str);

        FILE *data_file = fopen(DATA_FILE_NAME, "a+");

        if(data_file == NULL){
            syslog(LOG_ERR, "Error opening data file: %s", strerror(errno));
                    close(client_fd);
        client_fd = -1;

        syslog(LOG_INFO, "Closed connection from %s", addr_str);
        continue;
        }

        while((res = recv(client_fd, recv_buffer, sizeof(recv_buffer), 0)) > 0){
            syslog(LOG_DEBUG, "Received %d bytes", res);

            fwrite(recv_buffer, sizeof(*recv_buffer), res, data_file);

            if(memchr(recv_buffer, '\n', res) != NULL){
                syslog(LOG_DEBUG, "Newline detected. Packet fully received");
                break;
            }

            memset(recv_buffer, 0, sizeof(recv_buffer));
        }

        if(res == 0){
            syslog(LOG_INFO, "Connection closed by client");
            fclose(data_file);
        } else if(res == -1){
            syslog(LOG_ERR, "recv error: %s", strerror(errno));
            fclose(data_file);
        }

        if(fseek(data_file, 0, SEEK_SET) == -1){
            syslog(LOG_ERR, "Error seeking data file: %s", strerror(errno));
            fclose(data_file);
        }

        while((res = fread(recv_buffer, sizeof(*recv_buffer), sizeof(recv_buffer), data_file)) > 0){
            syslog(LOG_DEBUG, "Sending %d bytes", res);
            res = send(client_fd, recv_buffer, res, 0);
            if(res == -1){
                syslog(LOG_ERR, "send error: %s", strerror(errno));
                fclose(data_file);
            }
            memset(recv_buffer, 0, sizeof(recv_buffer));
        }
    }

}

