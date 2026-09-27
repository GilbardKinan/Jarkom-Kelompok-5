#include "../include/client_utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;

  // 1. Inisialisasi socket (AF_INET, SOCK_STREAM)
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVER_PORT);

  if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    exit(EXIT_FAILURE);
  }

  // Connect ke server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[+] Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

  // Panggil fungsionalitas utama
  run_character_count_service(sock, "HaloUgm");

  // 9. Tutup socket
  close(sock);
  printf("[+] Socket closed.\n");

  return 0;
}
