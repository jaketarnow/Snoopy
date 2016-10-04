//
// Created by Jacob Tarnow on 9/27/16.
//
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"

int str_vector_init(struct str_vector *vector) {
    vector->str_array = NULL;
    vector->str_count = 0;
    return 0;
}

int str_vector_add(struct str_vector *vector, char *str) {
    if ((vector->str_array = (char **)realloc(vector->str_array,
                                               (vector->str_count + 1) * sizeof(char *))) == NULL ) {
        return(-1);
    }
    if ((vector->str_array[vector->str_count++] = strdup(str)) == NULL) {
        return -1;
    }
    return 0;
}

int str_vector_search(struct str_vector *vector, char *search_str) {
    int i;

    for (i = 0; i < vector->str_count; i++) {
        if (strcmp(vector->str_array[i], search_str) == 0) {
            return true;
        }
    }
    return false;
}

int str_vector_free(struct str_vector *vector) {
    int i;

    for (i = 0; i < vector->str_count; i++) {
        free(vector->str_array[i]);
    }
    free(vector->str_array);
    vector->str_count = 0;
    return 0;
}