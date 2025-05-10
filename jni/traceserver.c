#include <jni.h>
#include <android/log.h>
#include <android/trace.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024
#define TAG "TraceServer"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static int server_socket = -1;
static int is_running = 0;
static pthread_t server_thread;
static int test_mode = 0;
static int server_port = DEFAULT_PORT;

// Test mode function that generates trace events
static void* test_loop(void* arg) {
    int counter = 0;
    char tag[64];
    
    while (is_running) {
        snprintf(tag, sizeof(tag), "test_trace_%d", counter++);
        ATrace_beginSection(tag);
        LOGI("Test trace section started with tag: %s", tag);
        usleep(200000);
        ATrace_endSection();
        LOGI("Test trace section ended for tag: %s", tag);
        usleep(1000000);
    }
    return NULL;
}

void* server_loop(void* arg) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        LOGE("Socket creation failed: %s", strerror(errno));
        return NULL;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOGE("Setsockopt failed: %s", strerror(errno));
        close(server_socket);
        return NULL;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOGE("Bind failed: %s", strerror(errno));
        close(server_socket);
        return NULL;
    }

    if (listen(server_socket, 5) < 0) {
        LOGE("Listen failed: %s", strerror(errno));
        close(server_socket);
        return NULL;
    }

    LOGI("Server started on port %d", server_port);

    while (is_running) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            LOGE("Accept failed: %s", strerror(errno));
            continue;
        }

        LOGI("Client connected");

        while (is_running) {
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_read <= 0) {
                break;
            }
            if (buffer[bytes_read - 1] == '\n') {
                buffer[bytes_read - 1] = '\0';
            }
            char* command = strtok(buffer, "|");
            char* tag = strtok(NULL, "|");
            if (command && tag) {
                if (strcmp(command, "begin") == 0) {
                    ATrace_beginSection(tag);
                    LOGI("Trace section started with tag: %s", tag);
                } else if (strcmp(command, "end") == 0) {
                    ATrace_endSection();
                    LOGI("Trace section ended for tag: %s", tag);
                }
            } else {
                LOGE("Invalid command format. Expected: begin|<tag> or end|<tag>");
            }
        }
        close(client_socket);
        LOGI("Client disconnected");
    }
    close(server_socket);
    return NULL;
}

void print_usage(const char* program_name) {
    LOGE("Usage: %s [-t] [-p port]", program_name);
    LOGE("Options:");
    LOGE("  -t        Run in test mode (generate test traces)");
    LOGE("  -p port   Specify port number (default: %d)", DEFAULT_PORT);
}

int main(int argc, char *argv[]) {
    int opt;
    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }
    while ((opt = getopt(argc, argv, "tp:")) != -1) {
        switch (opt) {
            case 't':
                test_mode = 1;
                LOGI("Test mode enabled");
                break;
            case 'p':
                server_port = atoi(optarg);
                if (server_port <= 0 || server_port > 65535) {
                    LOGE("Invalid port number: %s", optarg);
                    print_usage(argv[0]);
                    return 1;
                }
                LOGI("Using port: %d", server_port);
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    // Daemonize the process
    pid_t pid = fork();
    if (pid < 0) {
        LOGE("Fork failed");
        return 1;
    }
    if (pid > 0) {
        // Parent process exits
        return 0;
    }
    umask(0);
    if (setsid() < 0) {
        LOGE("setsid failed");
        return 1;
    }
    int fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
        return 1;
    }
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
    is_running = 1;
    if (test_mode) {
        if (pthread_create(&server_thread, NULL, test_loop, NULL) != 0) {
            LOGE("Failed to create test thread");
            return 1;
        }
        LOGI("Test mode started - generating traces every 200ms");
        pthread_join(server_thread, NULL);
    } else {
        if (pthread_create(&server_thread, NULL, server_loop, NULL) != 0) {
            LOGE("Failed to create server thread");
            return 1;
        }
        LOGI("Server started successfully on port %d", server_port);
        pthread_join(server_thread, NULL);
    }
    return 0;
} 