//
// Created by Jacob Tarnow on 9/27/16.
//

#ifndef UPNPDISCOVER_UTILS_H
#define UPNPDISCOVER_UTILS_H

struct str_vector {
    char **str_array;
    int str_count;
};

/* Functions */
int str_vector_init(struct str_vector *vector);
int str_vector_add(struct str_vector *vector, char *str);
int str_vector_search(struct str_vector *vector, char *search_str);
int str_vector_free(struct str_vector *vector);

#endif //UPNPDISCOVER_UTILS_H
