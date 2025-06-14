/*************************************************************
* proto_tdd_v1 -  émetteur                                   *
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
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; // message de l'application 
    int taille_msg; // taille du message /
    paquet_t paquet; // paquet utilisé par le protocole 
    paquet_t rep_ack;
    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    // lecture de donnees provenant de la couche application 
    de_application(message, &taille_msg);

    //tant que l'émetteur a des données à envoyer 
    while ( taille_msg != 0 ) {

        // construction paquet 
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.somme_ctrl = generator(paquet);
        
        //renvoie le paquet en cas d'erreur 
        do{
            //remise à la couche reseau 
            vers_reseau(&paquet);
            de_reseau(&rep_ack);
        }while(rep_ack.type != ACK);

        // lecture des donnees suivantes de la couche application 
        de_application(message, &taille_msg);
        
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
