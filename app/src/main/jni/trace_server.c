#include <jni.h>
#include <android/log.h>
#include <android/trace.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define TAG "TraceServer"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static int server_socket = -1;
static int is_running = 0;
static pthread_t server_thread;

void* server_loop(void* arg) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        LOGE("Socket creation failed: %s", strerror(errno));
        return NULL;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOGE("Setsockopt failed: %s", strerror(errno));
        close(server_socket);
        return NULL;
    }

    // Bind socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOGE("Bind failed: %s", strerror(errno));
        close(server_socket);
        return NULL;
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        LOGE("Listen failed: %s", strerror(errno));
        close(server_socket);
        return NULL;
    }

    LOGI("Server started on port %d", PORT);

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

            // Remove newline if present
            if (buffer[bytes_read - 1] == '\n') {
                buffer[bytes_read - 1] = '\0';
            }

            if (strcmp(buffer, "begin") == 0) {
                ATrace_beginSection("TraceSection");
                LOGI("Trace section started");
            } else if (strcmp(buffer, "end") == 0) {
                ATrace_endSection();
                LOGI("Trace section ended");
            }
        }

        close(client_socket);
        LOGI("Client disconnected");
    }

    close(server_socket);
    return NULL;
}

JNIEXPORT jboolean JNICALL
Java_com_example_traceserver_TraceServer_startServer(JNIEnv* env, jobject thiz) {
    if (is_running) {
        return JNI_TRUE;
    }

    is_running = 1;
    if (pthread_create(&server_thread, NULL, server_loop, NULL) != 0) {
        LOGE("Failed to create server thread");
        is_running = 0;
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_example_traceserver_TraceServer_stopServer(JNIEnv* env, jobject thiz) {
    if (!is_running) {
        return;
    }

    is_running = 0;
    if (server_socket != -1) {
        shutdown(server_socket, SHUT_RDWR);
        close(server_socket);
    }

    pthread_join(server_thread, NULL);
    LOGI("Server stopped");
} 