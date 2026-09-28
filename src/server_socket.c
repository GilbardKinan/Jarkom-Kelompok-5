// ============================================================================
// server_socket.c — Handler koneksi klien.
// Protokol: CHECK → ACTIVE/INACTIVE
//           REQUEST → hitung → (30% error) → RESPONSE
//           ACK TRUE/FALSE → deaktivasi layanan jika FALSE → OK
// ============================================================================

#include "../include/server_utils.h"
#include "../include/word_count.h"
#include "../include/reverse_string.h"
#include "../include/remove_vowels.h"
#include "../include/matrix_calc.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define NUM_SERVICES 5
#define ERROR_PROBABILITY 30

static const char *SERVICE_NAMES[NUM_SERVICES] = {
    "CHARACTER_COUNT",
    "WORD_COUNT",
    "REVERSE_STRING",
    "STRING_WITHOUT_VOWELS",
    "DETERMINAN_AND_INVERSE_MATRIX"
};

static int find_service_index(const char *name) {
    for (int i = 0; i < NUM_SERVICES; i++) {
        if (strcmp(name, SERVICE_NAMES[i]) == 0) {
            return i;
        }
    }

    return -1;
}

static int send_msg(int sock, const char *msg) {
    size_t total_sent = 0;
    size_t msg_len = strlen(msg);

    while (total_sent < msg_len) {
        ssize_t n = send(
            sock,
            msg + total_sent,
            msg_len - total_sent,
            0
        );

        if (n < 0) {
            perror("[Server] send() failed");
            return -1;
        }

        total_sent += (size_t)n;
    }

    printf("[Server->Client] %s\n", msg);

    return 0;
}

static ssize_t recv_msg(
    int sock,
    char *buf,
    size_t bufsize
) {
    memset(buf, 0, bufsize);

    ssize_t n = recv(
        sock,
        buf,
        bufsize - 1,
        0
    );

    if (n <= 0) {
        if (n == 0) {
            printf("[Server] Klien menutup koneksi.\n");
        } else {
            perror("[Server] recv() failed");
        }

        return -1;
    }

    buf[n] = '\0';

    printf("[Client->Server] %s\n", buf);

    return n;
}

static int send_service_result(
    int client_sock,
    const char *service_name,
    const char *text
) {
    char response[BUFFER_SIZE];

    /*
     * CHARACTER_COUNT
     */
    if (strcmp(service_name, "CHARACTER_COUNT") == 0) {

        int result = count_characters(text);

        if ((rand() % 100) < ERROR_PROBABILITY) {
            int original = result;
            result += 5;

            printf(
                "[Server] *** ERROR INJECTED *** "
                "jawaban diubah %d -> %d\n",
                original,
                result
            );
        }

        snprintf(
            response,
            sizeof(response),
            "RESPONSE %d",
            result
        );

        return send_msg(client_sock, response);
    }

    /*
     * WORD_COUNT
     */
    if (strcmp(service_name, "WORD_COUNT") == 0) {

        int result = (int)count_words(text);

        if ((rand() % 100) < ERROR_PROBABILITY) {
            int original = result;
            result += 5;

            printf(
                "[Server] *** ERROR INJECTED *** "
                "jawaban diubah %d -> %d\n",
                original,
                result
            );
        }

        snprintf(
            response,
            sizeof(response),
            "RESPONSE %d",
            result
        );

        return send_msg(client_sock, response);
    }

    /*
     * REVERSE_STRING
     */
    if (strcmp(service_name, "REVERSE_STRING") == 0) {

        char *result = reverse_string(text);

        if (result == NULL) {
            return send_msg(
                client_sock,
                "ERROR MEMORY_ALLOCATION"
            );
        }

        if ((rand() % 100) < ERROR_PROBABILITY) {
            size_t len = strlen(result);

            if (len > 0) {
                result[len - 1] =
                    (result[len - 1] == 'X')
                        ? 'Y'
                        : 'X';
            }

            printf(
                "[Server] *** ERROR INJECTED *** "
                "hasil reverse diubah.\n"
            );
        }

        snprintf(
            response,
            sizeof(response),
            "RESPONSE %s",
            result
        );

        free(result);

        return send_msg(client_sock, response);
    }

    /*
     * STRING_WITHOUT_VOWELS
     */
    if (strcmp(
            service_name,
            "STRING_WITHOUT_VOWELS"
        ) == 0) {

        char result[BUFFER_SIZE];

        remove_vowels(
            text,
            result
        );

        if ((rand() % 100) < ERROR_PROBABILITY) {
            size_t len = strlen(result);

            if (len < sizeof(result) - 1) {
                result[len] = 'X';
                result[len + 1] = '\0';
            }

            printf(
                "[Server] *** ERROR INJECTED *** "
                "hasil remove vowels diubah.\n"
            );
        }

        snprintf(
            response,
            sizeof(response),
            "RESPONSE %s",
            result
        );

        return send_msg(client_sock, response);
    }

    /*
     * DETERMINAN_AND_INVERSE_MATRIX
     */
    if (strcmp(
            service_name,
            "DETERMINAN_AND_INVERSE_MATRIX"
        ) == 0) {

        double matrix[3][3];
        double determinant;
        double inverse[3][3];

        char *endptr;

        endptr = NULL;

        const char *ptr = text;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {

                matrix[i][j] = strtod(ptr, &endptr);

                if (ptr == endptr) {
                    return send_msg(
                        client_sock,
                        "ERROR INVALID_MATRIX"
                    );
                }

                ptr = endptr;
            }
        }

        if (!process_matrix_3x3(
                matrix,
                &determinant,
                inverse
            )) {

            return send_msg(
                client_sock,
                "ERROR SINGULAR_MATRIX"
            );
        }

        if ((rand() % 100) < ERROR_PROBABILITY) {
            determinant += 5.0;

            printf(
                "[Server] *** ERROR INJECTED *** "
                "determinan diubah.\n"
            );
        }

        snprintf(
            response,
            sizeof(response),
            "RESPONSE %.10f "
            "%.10f %.10f %.10f "
            "%.10f %.10f %.10f "
            "%.10f %.10f %.10f",
            determinant,
            inverse[0][0],
            inverse[0][1],
            inverse[0][2],
            inverse[1][0],
            inverse[1][1],
            inverse[1][2],
            inverse[2][0],
            inverse[2][1],
            inverse[2][2]
        );

        return send_msg(client_sock, response);
    }

    return send_msg(
        client_sock,
        "ERROR UNKNOWN_SERVICE"
    );
}

void handle_client_request(
    int client_sock,
    bool *services_active
) {
    char buffer[BUFFER_SIZE];

    /*
     * Satu client dapat menggunakan koneksi yang sama
     * untuk banyak layanan.
     */
    while (1) {

        if (recv_msg(
                client_sock,
                buffer,
                BUFFER_SIZE
            ) < 0) {

            return;
        }

        /*
         * ================================================================
         * CHECK SERVICE
         * ================================================================
         */

        char service_name[128] = {0};

        if (sscanf(
                buffer,
                "CHECK %127s",
                service_name
            ) == 1) {

            int svc_idx =
                find_service_index(service_name);

            if (svc_idx < 0) {
                printf(
                    "[Server] Layanan \"%s\" "
                    "tidak dikenal.\n",
                    service_name
                );

                if (send_msg(
                        client_sock,
                        "INACTIVE"
                    ) < 0) {
                    return;
                }

                continue;
            }

            if (services_active[svc_idx]) {

                if (send_msg(
                        client_sock,
                        "ACTIVE"
                    ) < 0) {
                    return;
                }

            } else {

                if (send_msg(
                        client_sock,
                        "INACTIVE"
                    ) < 0) {
                    return;
                }
            }

            continue;
        }

        /*
         * ================================================================
         * REQUEST SERVICE
         * ================================================================
         */

        if (strncmp(
                buffer,
                "REQUEST ",
                8
            ) == 0) {

            char req_service[128] = {0};
            char text[BUFFER_SIZE] = {0};

            char *ptr = buffer + 8;
            char *space = strchr(ptr, ' ');

            if (space == NULL) {
                send_msg(
                    client_sock,
                    "ERROR MISSING_TEXT"
                );
                continue;
            }

            size_t svc_len =
                (size_t)(space - ptr);

            if (svc_len >= sizeof(req_service)) {
                svc_len =
                    sizeof(req_service) - 1;
            }

            strncpy(
                req_service,
                ptr,
                svc_len
            );

            req_service[svc_len] = '\0';

            strncpy(
                text,
                space + 1,
                sizeof(text) - 1
            );

            text[sizeof(text) - 1] = '\0';

            printf(
                "[Server] Layanan: \"%s\", Teks: \"%s\"\n",
                req_service,
                text
            );

            int svc_idx =
                find_service_index(req_service);

            if (svc_idx < 0) {
                send_msg(
                    client_sock,
                    "ERROR UNKNOWN_SERVICE"
                );
                continue;
            }

            if (!services_active[svc_idx]) {
                send_msg(
                    client_sock,
                    "INACTIVE"
                );
                continue;
            }

            if (send_service_result(
                    client_sock,
                    req_service,
                    text
                ) < 0) {

                return;
            }

            /*
             * ============================================================
             * ACKNOWLEDGEMENT
             * ============================================================
             */

            if (recv_msg(
                    client_sock,
                    buffer,
                    BUFFER_SIZE
                ) < 0) {

                return;
            }

            if (strcmp(
                    buffer,
                    "ACK TRUE"
                ) == 0) {

                printf(
                    "[Server] Klien memverifikasi: "
                    "jawaban BENAR.\n"
                );

            } else if (strcmp(
                    buffer,
                    "ACK FALSE"
                ) == 0) {

                printf(
                    "[Server] Klien memverifikasi: "
                    "jawaban SALAH.\n"
                );

                services_active[svc_idx] = false;

                printf(
                    "[Server] Layanan \"%s\" "
                    "(indeks %d) dinonaktifkan.\n",
                    req_service,
                    svc_idx
                );

            } else {

                printf(
                    "[Server] ACK tidak dikenali: \"%s\"\n",
                    buffer
                );
            }

            /*
             * Client hanya menunggu satu response setelah ACK.
             * Jadi server cukup mengirim OK satu kali.
             */
            if (send_msg(
                    client_sock,
                    "OK"
                ) < 0) {

                return;
            }

            printf(
                "[Server] Transaksi untuk layanan "
                "\"%s\" selesai.\n",
                req_service
            );

            /*
             * Jika seluruh layanan sudah nonaktif,
             * handler selesai dan main() akan mematikan server.
             */
            bool any_active = false;

            for (int i = 0; i < NUM_SERVICES; i++) {
                if (services_active[i]) {
                    any_active = true;
                    break;
                }
            }

            if (!any_active) {
                printf(
                    "[Server] Semua layanan nonaktif.\n"
                );
                return;
            }

            continue;
        }

        /*
         * ================================================================
         * FORMAT REQUEST TIDAK DIKENALI
         * ================================================================
         */

        printf(
            "[Server] Format request tidak dikenali: \"%s\"\n",
            buffer
        );

        if (send_msg(
                client_sock,
                "ERROR FORMAT_INVALID"
            ) < 0) {

            return;
        }
    }
}
