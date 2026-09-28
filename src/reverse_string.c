#include "../include/client_utils.h" 
#include <string.h>
#include <stdlib.h>

char* reverse_string(const char *text) {
    if (text == NULL) {
        return NULL;
    }
    size_t len = strlen(text);
    
    char *reversed = (char *)malloc((len + 1) * sizeof(char));

    if (reversed == NULL) {
        return NULL; 
    }
    for (size_t i = 0; i < len; i++) {
        reversed[i] = text[len - 1 - i];
    }
    
    reversed[len] = '\0';

    return reversed;
}