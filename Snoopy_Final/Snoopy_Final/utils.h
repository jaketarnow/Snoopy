#ifndef ScanUPNP_h
#define ScanUPNP_h

#include <stdio.h>


#if !defined TRUE && !defined FALSE
#define TRUE    1
#define FALSE   0
#endif

struct str_vector {
    char **str_array;
    int str_count;
};

/* Functions */
int str_vector_init(struct str_vector *vector);
int str_vector_add(struct str_vector *vector, char *str);
int str_vector_search(struct str_vector *vector, char *search_str);
int str_vector_free(struct str_vector *vector);

#endif /* ScanUPNP_h */
