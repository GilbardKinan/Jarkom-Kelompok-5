#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

// Mengembalikan jumlah karakter dari string (strlen wrapper)
int count_characters(const char *text);

// Menjalankan alur protokol layanan CHARACTER_COUNT melalui socket
void run_character_count_service(int sock, const char *text);

#endif // CLIENT_UTILS_H
