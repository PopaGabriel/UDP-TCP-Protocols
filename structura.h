#include "helpers.h"

//Mesaj de la TCP la server
typedef struct mesaj {
    char type[2];
    char id_client[11];
    char topic[52];
    char text[1600];
}*mesaj_transmis;

//Mesaj UDP
typedef struct mesajUDP {
    char udp[4];
    char topic[52];
    char tipp;
    char text[1502];
    char ip[50];
    char port[10];
}*mesaj_udp;

//Date client
typedef struct id_topica {
    char id_client[52];
    char subscribed;
    char SF;
}*pereche_socket_id;

//"Vector cu clienti"
typedef struct clienti {
    pereche_socket_id *socketi_clienti;
    int size_max;
    int size;
}*lista_subscriberi;

//"Vector cu mesaje"
typedef struct sf_topics {
    char **vector;
    int size_max;
    int size;
}*topics_sf;
