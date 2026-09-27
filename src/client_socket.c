#include "../include/client_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void run_character_count_service(int sock, const char *text) {
  char buffer[BUFFER_SIZE] = {0};

  // 2. Pengecekan Status
  const char *check_msg = "CHECK CHARACTER_COUNT";
  if (send(sock, check_msg, strlen(check_msg), 0) < 0) {
    perror("Send CHECK failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Client] Sent: %s\n", check_msg);

  // 3. Tangkap respons server
  memset(buffer, 0, BUFFER_SIZE);
  if (recv(sock, buffer, BUFFER_SIZE - 1, 0) < 0) {
    perror("Receive failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Server] Response: %s\n", buffer);

  if (strcmp(buffer, "ACTIVE") != 0) {
    printf("[-] Service is INACTIVE or unknown response. Exiting...\n");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[+] Service is ACTIVE. Proceeding...\n");

  // 4. Validasi Lokal — memanggil count_characters dari stringx.c
  int local_count = count_characters(text);
  printf("[Local] String: \"%s\", Length: %d\n", text, local_count);

  // 5. Kirim Request
  char req_msg[BUFFER_SIZE];
  snprintf(req_msg, sizeof(req_msg), "REQUEST CHARACTER_COUNT %s", text);
  if (send(sock, req_msg, strlen(req_msg), 0) < 0) {
    perror("Send REQUEST failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Client] Sent: %s\n", req_msg);

  // 6. Ekstrak Respons
  memset(buffer, 0, BUFFER_SIZE);
  if (recv(sock, buffer, BUFFER_SIZE - 1, 0) < 0) {
    perror("Receive failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Server] Response: %s\n", buffer);

  int server_count = -1;
  if (sscanf(buffer, "RESPONSE %d", &server_count) != 1) {
    printf("[-] Failed to parse server response.\n");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Client] Extracted server count: %d\n", server_count);

  // 7. Pengiriman Acknowledgement (ACK)
  const char *ack_msg;
  if (server_count == local_count) {
    ack_msg = "ACK TRUE";
  } else {
    ack_msg = "ACK FALSE";
  }
  if (send(sock, ack_msg, strlen(ack_msg), 0) < 0) {
    perror("Send ACK failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Client] Sent: %s\n", ack_msg);

  // 8. Terima konfirmasi akhir
  memset(buffer, 0, BUFFER_SIZE);
  if (recv(sock, buffer, BUFFER_SIZE - 1, 0) < 0) {
    perror("Receive failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
  printf("[Server] Response: %s\n", buffer);

  if (strcmp(buffer, "OK") == 0) {
    printf("[+] Transaction completed successfully.\n");
  } else {
    printf("[-] Transaction failed or unconfirmed.\n");
  }
}
