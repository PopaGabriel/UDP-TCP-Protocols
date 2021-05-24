#include "helpers.h"

typedef struct mesaj {
    int type;
    char topic[50];
    union data
    {
        int int_valoare;
        u_int16_t u16_valoare;
        u_int32_t u32_valoare;
        char text[1600];
    } *data;
}*mesaj;