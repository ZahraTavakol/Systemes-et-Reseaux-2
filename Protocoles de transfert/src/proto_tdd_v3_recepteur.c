#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define CAPA_SEQUENCE 16

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; // Données à transmettre à l'application
    paquet_t paquet; // Paquet reçu du réseau
    paquet_t rep;    // Paquet de réponse (ACK)
    int fin = 0;     // Condition d'arrêt du protocole

    init_reseau(RECEPTION);
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    uint8_t paquetAttend = 0; // Numéro de séquence attendu par le récepteur

    // Boucle de réception principale
    while (!fin) {
        de_reseau(&paquet); // Attente et réception d'un paquet

        // Vérifie l'intégrité des données
        if (verifier(paquet)) {
            printf("[INFO] Contrôle d'intégrité réussi.\n");

            if (paquet.num_seq == paquetAttend) {
                printf("[INFO] Paquet attendu reçu : %d\n", paquet.num_seq);
                printf("[INFO] Transfert vers application en cours.\n");

                // Construction de l'ACK
                rep.num_seq = paquetAttend; 
                rep.type = ACK; 
                rep.lg_info = 0; 
                rep.somme_ctrl = generator(rep); 

                // Extraction des données pour l'application
                for (int i = 0; i < paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }

                // Transmission à la couche application
                fin = vers_application(message, paquet.lg_info);

                // Mise à jour du numéro de séquence attendu
                paquetAttend = inc(paquetAttend, 16); 
            }
            // Envoi de l'accusé de réception dans tous les cas
            vers_reseau(&rep); 
        }
    }

    // Gestion de fin de transmission en cas de perte du dernier ACK
    depart_temporisateur(1000); 
    if (attendre() == -1) {
        vers_reseau(&rep); 
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
