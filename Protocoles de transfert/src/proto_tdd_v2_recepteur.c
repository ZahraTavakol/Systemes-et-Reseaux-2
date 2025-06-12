/*************************************************************
* proto_tdd_v2 -  récepteur                                  *
* TRANSFERT DE DONNEES  v2                                  *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; // message pour l'application 
    paquet_t paquet; //paquet utilisé par le protocole 
    paquet_t pack;
    int fin = 0; //condition d'arrêt 
    int prochain=0;

    
    
    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    //tant que le récepteur reçoit des données 
    while ( !fin ) {
        de_reseau(&paquet);

        if(verifier(paquet)){
            if(paquet.num_seq == prochain){
                //extraction des donnees du paquet recu 
                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                // remise des données à la couche application 
                fin = vers_application(message, paquet.lg_info);
                prochain = increase(prochain)%SEQ_NUM_SIZE;
                pack.type = ACK;
            }
        
        
        vers_reseau(&pack);
        }
        
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
