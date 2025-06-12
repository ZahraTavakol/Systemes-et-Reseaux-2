#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define CAP_SEQ 16

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    int tailleFenetre; 

    // Détermine la taille de la fenêtre de réception
    if(argc == 2){
        tailleFenetre = atoi(argv[1]);
    } else {
        tailleFenetre = 4;
    }

    // Vérifie que la taille ne dépasse pas la capacité autorisée
    if (tailleFenetre >= CAP_SEQ) {
        perror("Erreur : taille de fenêtre invalide.");
        return 1;
    }

    paquet_t paquet; // Paquet reçu du réseau
    paquet_t rep;    // Paquet de réponse (ACK)
    int fin = 0;     // Indique la fin de la transmission
    int borne_Inf = 0; // Position de début de la fenêtre glissante
    int acquitte[CAP_SEQ] = {0}; // Marque les paquets tamponnés en attente de traitement
    paquet_t buffer[CAP_SEQ];    // Tampon pour les paquets hors ordre
    unsigned char mess[MAX_INFO]; // Données destinées à l'application

    init_reseau(RECEPTION);
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    // Boucle principale : traite les paquets entrants
    while (!fin) {
        de_reseau(&paquet);

        // Vérification de l'intégrité des données
        if (verifier(paquet)) {
            printf("[INFO] Contrôle de validité passé.\n");

            if(dans_fenetre(borne_Inf, paquet.num_seq, tailleFenetre)) {
                printf("[INFO] Paquet %d reçu dans la fenêtre.\n", paquet.num_seq);

                // Construction de l'ACK
                rep.num_seq = paquet.num_seq; 
                rep.type = ACK; 
                rep.lg_info = 0; 
                rep.somme_ctrl = generator(rep); 

                if(paquet.num_seq != borne_Inf) { 
                    // Tamponne les paquets hors ordre
                    printf("[INFO] Paquet %d en avance, stocké temporairement.\n", paquet.num_seq);
                    buffer[paquet.num_seq] = paquet; 
                    acquitte[paquet.num_seq] = 1;
                } else {
                    // Transmission séquentielle vers l'application
                    buffer[borne_Inf] = paquet; 
                    do {
                        for (int i = 0; i < buffer[borne_Inf].lg_info; i++) {
                            mess[i] = buffer[borne_Inf].info[i]; 
                        }   
                        fin = vers_application(mess, buffer[borne_Inf].lg_info);
                        acquitte[borne_Inf] = 0; 
                        borne_Inf = inc(borne_Inf, CAP_SEQ);
                    } while(acquitte[borne_Inf] != 0); 
                }
                vers_reseau(&rep); // Envoi de l'accusé de réception
            } else {
                // Répond même aux paquets valides mais hors fenêtre
                rep.num_seq = paquet.num_seq; 
                rep.type = ACK; 
                rep.lg_info = 0;
                rep.somme_ctrl = generator(rep); 
                vers_reseau(&rep); 
            }
        }
    }

    // Dernière vérification pour réenvoyer l'ACK si nécessaire
    depart_temporisateur(1000);   
    if (attendre()){
        vers_reseau(&rep);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}