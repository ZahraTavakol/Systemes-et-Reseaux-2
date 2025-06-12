#ifndef __COUCHE_TRANSPORT_H__
#define __COUCHE_TRANSPORT_H__
#include <stdbool.h>
#include <stdint.h> /* uint8_t */

#define MAX_INFO 124

/*************************
 * Structure d'un paquet *
 *************************/

typedef struct paquet_s {
    uint8_t type;         /* type de paquet, cf. ci-dessous */
    uint8_t num_seq;      /* numéro de séquence */
    uint8_t lg_info;      /* longueur du champ info */
    uint8_t somme_ctrl;   /* somme de contrôle */
    unsigned char info[MAX_INFO];  /* données utiles du paquet */
} paquet_t;

/******************
 * Types de paquet *
 ******************/
#define DATA          1  /* données de l'application */
#define ACK           2  /* accusé de réception des données */
#define NACK          3  /* accusé de réception négatif */
#define CON_REQ       4  /* demande d'établissement de connexion */
#define CON_ACCEPT    5  /* acceptation de connexion */
#define CON_REFUSE    6  /* refus d'établissement de connexion */
#define CON_CLOSE     7  /* notification de déconnexion */
#define CON_CLOSE_ACK 8  /* accusé de réception de la déconnexion */
#define OTHER         9  /* extensions */

/* Capacite de numerotation pour l'anticipation */
#define SEQ_NUM_SIZE 16

/* ************************************** */
/* Fonctions utilitaires couche transport */
/* ************************************** */

bool verifier(paquet_t paquet);
uint8_t generator(paquet_t paquet); 
//int verifier_Controle(paquet_t paquet); 
int increase(int n); 
int inc(int n, int m); 
/*--------------------------------------*
* Fonction d'inclusion dans la fenetre *
*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille);

#endif
