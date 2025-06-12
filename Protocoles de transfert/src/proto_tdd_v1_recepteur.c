/*************************************************************
* proto_tdd_v1 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"
#include <stdbool.h>
/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; // message pour l'application 
    paquet_t paquet; //paquet utilisé par le protocole 
    int fin = 0; // condition d'arrêt 
    paquet_t rep_ack;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    // tant que le récepteur reçoit des données 
    while ( !fin ) {
        de_reseau(&paquet);

        if(verifier(paquet)){
            // extraction des donnees du paquet recu 
            for (int i=0; i<paquet.lg_info; i++) {
                message[i] = paquet.info[i];
            }
            // remise des données à la couche application 
            fin = vers_application(message, paquet.lg_info);
            rep_ack.type = ACK;
        }
        else{
            rep_ack.type = NACK;
        }
        vers_reseau(&rep_ack);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
