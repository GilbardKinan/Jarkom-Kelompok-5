#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stdbool.h>

// ========================================================================
// Deklarasi fungsi kalkulator (dari stringx.c)
// Tetap dideklarasikan di sini agar stringx.c bisa dikompilasi tanpa ubah.
// ========================================================================
int count_characters(const char *text);

// ========================================================================
// Deklarasi fungsi handler koneksi klien (dari server_socket.c)
// Dipanggil oleh main.c setiap kali accept() berhasil.
// ========================================================================
void handle_client_request(int client_sock, bool *services_active);

#endif // SERVER_UTILS_H
