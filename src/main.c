// ============================================================================
// main.c — TCP Server: loop accept(), kelola 5 layanan, berhenti saat semua
//          layanan nonaktif.
// ============================================================================

#include "../include/server_utils.h"
#include "../include/word_count.h"

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 8080
#define NUM_SERVICES 5

// Nama-nama layanan yang dilacak oleh server
static const char *SERVICE_NAMES[NUM_SERVICES] = {
    "CHARACTER_COUNT",
    "WORD_COUNT",
    "UPPER_CASE",
    "LOWER_CASE",
    "REVERSE"
};

int main(void) {
    // ---- Seed random (untuk fitur pengacak error di server_socket.c) ----
    srand((unsigned int)time(NULL));

    // ---- Array status layanan: true = aktif ----
    bool services_active[NUM_SERVICES];
    for (int i = 0; i < NUM_SERVICES; i++) {
        services_active[i] = true;
    }

    // ---- 1. Buat socket server (TCP) ----
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[Server] socket() failed");
        exit(EXIT_FAILURE);
    }

    // Opsi SO_REUSEADDR agar port bisa langsung dipakai kembali
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[Server] setsockopt() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // ---- 2. Bind ke 127.0.0.1:8080 ----
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("[Server] inet_pton() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[Server] bind() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // ---- 3. Listen ----
    if (listen(server_fd, 5) < 0) {
        perror("[Server] listen() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("========================================\n");
    printf("[Server] Listening on %s:%d\n", SERVER_IP, SERVER_PORT);
    printf("[Server] Services (%d):\n", NUM_SERVICES);
    for (int i = 0; i < NUM_SERVICES; i++) {
        printf("  [%d] %s = ACTIVE\n", i, SERVICE_NAMES[i]);
    }
    printf("========================================\n");

    // ---- 4. Loop accept() — berhenti jika SEMUA layanan nonaktif ----
    while (1) {
        // Cek apakah masih ada layanan yang aktif
        bool any_active = false;
        for (int i = 0; i < NUM_SERVICES; i++) {
            if (services_active[i]) {
                any_active = true;
                break;
            }
        }
        if (!any_active) {
            printf("\n[Server] Semua layanan sudah nonaktif. Mematikan server...\n");
            break;
        }

        // Terima koneksi klien
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        printf("\n[Server] Menunggu koneksi klien...\n");

        int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("[Server] accept() failed");
            continue;  // lanjut ke iterasi berikutnya
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("[Server] Koneksi diterima dari %s:%d (fd=%d)\n",
               client_ip, ntohs(client_addr.sin_port), client_sock);

        // Tangani permintaan klien
        handle_client_request(client_sock, services_active);

        // Tutup socket klien setelah transaksi selesai
        close(client_sock);
        printf("[Server] Koneksi klien fd=%d ditutup.\n", client_sock);

        // Cetak status layanan terkini
        printf("[Server] Status layanan terkini:\n");
        for (int i = 0; i < NUM_SERVICES; i++) {
            printf("  [%d] %s = %s\n", i, SERVICE_NAMES[i],
                   services_active[i] ? "ACTIVE" : "INACTIVE");
        }
    }

    // ---- 5. Tutup socket utama ----
    close(server_fd);
    printf("[Server] Socket utama ditutup. Server berhenti.\n");

    return 0;
}
