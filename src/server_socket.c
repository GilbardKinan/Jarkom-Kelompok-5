// ============================================================================
// server_socket.c — Handler koneksi klien.
// Protokol: CHECK → ACTIVE/INACTIVE
//           REQUEST → hitung → (30% error) → RESPONSE
//           ACK TRUE/FALSE → deaktivasi layanan jika FALSE → OK
// ============================================================================

#include "../include/server_utils.h"
#include "../include/word_count.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define NUM_SERVICES 5

// Error probability: 30% → rand() % 100 < 30
#define ERROR_PROBABILITY 30

// Nama-nama layanan — harus sinkron dengan main.c
static const char *SERVICE_NAMES[NUM_SERVICES] = {
    "CHARACTER_COUNT",
    "WORD_COUNT",
    "UPPER_CASE",
    "LOWER_CASE",
    "REVERSE"
};

// ---------------------------------------------------------------------------
// Helper: cari indeks layanan berdasarkan nama.  Kembalikan -1 jika tidak ada.
// ---------------------------------------------------------------------------
static int find_service_index(const char *name) {
    for (int i = 0; i < NUM_SERVICES; i++) {
        if (strcmp(name, SERVICE_NAMES[i]) == 0) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Helper: kirim string ke socket (null-terminator TIDAK dikirim).
// ---------------------------------------------------------------------------
static int send_msg(int sock, const char *msg) {
    ssize_t n = send(sock, msg, strlen(msg), 0);
    if (n < 0) {
        perror("[Server] send() failed");
        return -1;
    }
    printf("[Server->Client] %s\n", msg);
    return 0;
}

// ---------------------------------------------------------------------------
// Helper: terima pesan dari socket.  Kembalikan jumlah byte, atau -1.
// ---------------------------------------------------------------------------
static ssize_t recv_msg(int sock, char *buf, size_t bufsize) {
    memset(buf, 0, bufsize);
    ssize_t n = recv(sock, buf, bufsize - 1, 0);
    if (n <= 0) {
        if (n == 0)
            printf("[Server] Klien menutup koneksi.\n");
        else
            perror("[Server] recv() failed");
        return -1;
    }
    buf[n] = '\0';
    printf("[Client->Server] %s\n", buf);
    return n;
}

// ---------------------------------------------------------------------------
// Hitung hasil berdasarkan nama layanan dan teks input.
// Mengembalikan hasil kalkulasi (int).
// ---------------------------------------------------------------------------
static int compute_service_result(const char *service_name, const char *text) {
    if (strcmp(service_name, "CHARACTER_COUNT") == 0) {
        // Memanggil fungsi dari stringx.c
        return count_characters(text);
    }
    if (strcmp(service_name, "WORD_COUNT") == 0) {
        // Memanggil fungsi dari word_count.c
        return (int)count_words(text);
    }
    if (strcmp(service_name, "UPPER_CASE") == 0) {
        // Hitung jumlah huruf besar
        int count = 0;
        for (size_t i = 0; text[i] != '\0'; i++) {
            if (text[i] >= 'A' && text[i] <= 'Z') count++;
        }
        return count;
    }
    if (strcmp(service_name, "LOWER_CASE") == 0) {
        // Hitung jumlah huruf kecil
        int count = 0;
        for (size_t i = 0; text[i] != '\0'; i++) {
            if (text[i] >= 'a' && text[i] <= 'z') count++;
        }
        return count;
    }
    if (strcmp(service_name, "REVERSE") == 0) {
        // Untuk REVERSE, kita kembalikan panjang teks (reversed string
        // tetap sama panjangnya — angka ini yang akan diverifikasi klien).
        return count_characters(text);
    }
    return -1;  // layanan tidak dikenal
}

// ===========================================================================
// Fungsi utama: handle_client_request
// Dipanggil oleh main.c setiap kali ada koneksi klien baru.
// ===========================================================================
void handle_client_request(int client_sock, bool *services_active) {
    char buffer[BUFFER_SIZE];

    // ---------------------------------------------------------------
    // Tahap 1: Terima pesan "CHECK <NAMA_LAYANAN>"
    // ---------------------------------------------------------------
    if (recv_msg(client_sock, buffer, BUFFER_SIZE) < 0) return;

    char service_name[128] = {0};

    if (sscanf(buffer, "CHECK %127s", service_name) != 1) {
        printf("[Server] Format CHECK tidak valid: \"%s\"\n", buffer);
        send_msg(client_sock, "ERROR FORMAT_INVALID");
        return;
    }

    int svc_idx = find_service_index(service_name);
    if (svc_idx < 0) {
        printf("[Server] Layanan \"%s\" tidak dikenal.\n", service_name);
        send_msg(client_sock, "INACTIVE");
        return;
    }

    // Balas status layanan
    if (services_active[svc_idx]) {
        if (send_msg(client_sock, "ACTIVE") < 0) return;
    } else {
        if (send_msg(client_sock, "INACTIVE") < 0) return;
        return;  // layanan nonaktif, akhiri di sini
    }

    // ---------------------------------------------------------------
    // Tahap 2: Terima pesan "REQUEST <NAMA_LAYANAN> <TEXT>"
    // ---------------------------------------------------------------
    if (recv_msg(client_sock, buffer, BUFFER_SIZE) < 0) return;

    char req_service[128] = {0};
    char text[BUFFER_SIZE] = {0};

    // Parse: "REQUEST <service> <text...>"
    // Kita perlu menangkap sisa string sebagai teks
    char *ptr = buffer;

    // Lewati kata "REQUEST "
    if (strncmp(ptr, "REQUEST ", 8) != 0) {
        printf("[Server] Format REQUEST tidak valid.\n");
        send_msg(client_sock, "ERROR FORMAT_INVALID");
        return;
    }
    ptr += 8;

    // Ambil nama layanan (sampai spasi berikutnya)
    char *space = strchr(ptr, ' ');
    if (space == NULL) {
        printf("[Server] Tidak ada teks setelah nama layanan.\n");
        send_msg(client_sock, "ERROR MISSING_TEXT");
        return;
    }

    size_t svc_len = (size_t)(space - ptr);
    if (svc_len >= sizeof(req_service)) svc_len = sizeof(req_service) - 1;
    strncpy(req_service, ptr, svc_len);
    req_service[svc_len] = '\0';

    // Sisa setelah spasi = teks input
    strncpy(text, space + 1, sizeof(text) - 1);
    text[sizeof(text) - 1] = '\0';

    printf("[Server] Layanan: \"%s\", Teks: \"%s\"\n", req_service, text);

    // Pastikan layanan dari REQUEST cocok dengan CHECK
    if (strcmp(req_service, service_name) != 0) {
        printf("[Server] Layanan REQUEST (%s) tidak cocok dengan CHECK (%s).\n",
               req_service, service_name);
        send_msg(client_sock, "ERROR SERVICE_MISMATCH");
        return;
    }

    // ---------------------------------------------------------------
    // Tahap 3: Hitung jawaban asli
    // ---------------------------------------------------------------
    int result = compute_service_result(service_name, text);
    printf("[Server] Jawaban asli (benar): %d\n", result);

    // ---------------------------------------------------------------
    // Tahap 4: Fitur Pengacak Error (30% probabilitas)
    // ---------------------------------------------------------------
    bool error_injected = false;
    if ((rand() % 100) < ERROR_PROBABILITY) {
        int original = result;
        result += 5;  // sengaja salah: tambah 5
        error_injected = true;
        printf("[Server] *** ERROR INJECTED *** jawaban diubah %d -> %d\n",
               original, result);
    }

    // Kirim respons ke klien
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "RESPONSE %d", result);
    if (send_msg(client_sock, response) < 0) return;

    // ---------------------------------------------------------------
    // Tahap 5: Terima ACK dari klien ("ACK TRUE" atau "ACK FALSE")
    // ---------------------------------------------------------------
    if (recv_msg(client_sock, buffer, BUFFER_SIZE) < 0) return;

    if (strcmp(buffer, "ACK TRUE") == 0) {
        printf("[Server] Klien memverifikasi: jawaban BENAR.\n");
    } else if (strcmp(buffer, "ACK FALSE") == 0) {
        printf("[Server] Klien memverifikasi: jawaban SALAH.\n");

        // Nonaktifkan layanan
        services_active[svc_idx] = false;
        printf("[Server] Layanan \"%s\" (indeks %d) dinonaktifkan.\n",
               service_name, svc_idx);

        // Kirim informasi penonaktifan ke klien
        char deactivate_msg[BUFFER_SIZE];
        snprintf(deactivate_msg, sizeof(deactivate_msg),
                 "SERVICE %s DEACTIVATED", service_name);
        if (send_msg(client_sock, deactivate_msg) < 0) return;
    } else {
        printf("[Server] ACK tidak dikenali: \"%s\"\n", buffer);
    }

    // ---------------------------------------------------------------
    // Tahap 6: Kirim "OK" — akhir transaksi
    // ---------------------------------------------------------------
    send_msg(client_sock, "OK");
    printf("[Server] Transaksi untuk layanan \"%s\" selesai.%s\n",
           service_name, error_injected ? " (error was injected)" : "");
}
