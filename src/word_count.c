#include "../include/word_count.h"

size_t count_words(const char *text)
{
    size_t count = 0;
    int in_word = 0;

    for (size_t i = 0; text[i] != '\0'; i++) {
        if (text[i] == ' ' ||
            text[i] == '\t' ||
            text[i] == '\n') {
            in_word = 0;
        } else if (in_word == 0) {
            count++;
            in_word = 1;
        }
    }

    return count;
}
