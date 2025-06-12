#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

#define CAP_SEQ 16

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[]) {

    int tailleFenetre; // Définition de la taille de la fenêtre glissante

    // Vérification des arguments passés au programme
    if(argc > 2){
        perror("Erreur : trop de paramètres fournis."); 
        return 1; 
    }

    unsigned char message[MAX_INFO]; // Contient les données issues de la couche application
    int taille_msg; // Taille du message
    paquet_t rep;   // Stockage temporaire pour les ACK

    int cursor = 0; // Numéro de séquence du prochain paquet à envoyer
    int borneInf = 0; // Limite inférieure de la fenêtre glissante
    paquet_t table[CAP_SEQ]; // Mémorisation des paquets envoyés non acquittés

    // Détermination de la taille de la fenêtre depuis les arguments ou défaut
    if(argc == 2){
        tailleFenetre = atoi(argv[1]); 
    } else {
        tailleFenetre = 4; 
    }

    if (tailleFenetre >= CAP_SEQ) {
        perror("Fenêtre trop grande pour la capacité de séquence.");
        return 1;
    }

    init_reseau(EMISSION);
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    // Boucle principale tant qu'il reste des données à envoyer ou des ACK à recevoir
    while ((taille_msg != 0) || (cursor != borneInf)) {
        if ((dans_fenetre(borneInf, cursor, tailleFenetre)) && (taille_msg > 0)) {

            // Préparation du paquet à envoyer
            memcpy(table[cursor].info, message, taille_msg);
            table[cursor].num_seq = cursor; 
            table[cursor].type = DATA; 
            table[cursor].lg_info = taille_msg; 
            table[cursor].somme_ctrl = generator(table[cursor]); 

            vers_reseau(&table[cursor]); 
            printf("[INFO] Envoi du paquet numéro %d.\n", cursor);

            // Démarrage du timer pour le premier de la fenêtre
            if(cursor == borneInf){
                depart_temporisateur(100); 
            }

            cursor = inc(cursor, 16);
            de_application(message, &taille_msg);
        } else {
            // Fenêtre pleine, attente d'un événement réseau
            if(attendre() == -1){
                de_reseau(&rep); 
                printf("[INFO] ACK reçu pour %d (avant vérification).\n", rep.num_seq);

                if (verifier(rep) && dans_fenetre(borneInf, rep.num_seq, tailleFenetre)) {
                    printf("[INFO] ACK validé pour %d.\n", rep.num_seq); 
                    borneInf = inc(rep.num_seq, 16);
                    if(borneInf == cursor){
                        arret_temporisateur(); 
                    }
                }
            } else {
                // Gestion d'un timeout : retransmettre les paquets en attente
                int i = borneInf; 
                depart_temporisateur(100); 
                while(i != cursor){
                    vers_reseau(&table[i]); 
                    i = inc(i, 16); 
                }
            }   
        }
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
