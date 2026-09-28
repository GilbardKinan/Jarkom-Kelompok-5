#include "../include/remove_vowels.h"

void remove_vowels(const char *text, char *result)
{
    int j = 0;

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] != 'a' &&
            text[i] != 'i' &&
            text[i] != 'u' &&
            text[i] != 'e' &&
            text[i] != 'o' &&
            text[i] != 'A' &&
            text[i] != 'I' &&
            text[i] != 'U' &&
            text[i] != 'E' &&
            text[i] != 'O') {

            result[j] = text[i];
            j++;
        }
    }

    result[j] = '\0';
}
