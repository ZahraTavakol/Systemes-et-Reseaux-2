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

    int tailleFenetre; // Taille de la fenêtre d'envoi

    // Vérifie que le nombre de paramètres est valide
    if(argc > 2){
        perror("Erreur : nombre de paramètres invalide."); 
        return 1; 
    }

    unsigned char message[MAX_INFO]; // Données récupérées de l'application
    int taille_msg; // Longueur des données
    paquet_t rep;   // Réponse reçue (ACK)

    int dup_ACK = 0; // Compteur d'ACKs dupliqués pour fast retransmit
    int cursor = 0;  // Prochain numéro de séquence
    int borneInf = 0; // Borne inférieure de la fenêtre
    paquet_t table[CAP_SEQ]; // Stocke les paquets en attente d'ACK

    // Détermination de la taille de la fenêtre
    if(argc == 2){
        tailleFenetre = atoi(argv[1]); 
    } else {
        tailleFenetre = 4; 
    }

    if (tailleFenetre >= CAP_SEQ) {
        perror("Erreur : fenêtre supérieure à la capacité de séquence.");
        return 1;
    }

    init_reseau(EMISSION);
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    // Transmission tant qu'il reste des données ou des acquittements en attente
    while ((taille_msg != 0) || (cursor != borneInf)) {
        if ((dans_fenetre(borneInf, cursor, tailleFenetre)) && (taille_msg > 0)) {

            // Création du paquet à envoyer
            memcpy(table[cursor].info, message, taille_msg);
            table[cursor].num_seq = cursor; 
            table[cursor].type = DATA; 
            table[cursor].lg_info = taille_msg; 
            table[cursor].somme_ctrl = generator(table[cursor]); 

            vers_reseau(&table[cursor]); 
            printf("[INFO] Paquet %d transmis au réseau.\n", cursor);

            // Démarrage du temporisateur si c'est le début de la fenêtre
            if(cursor == borneInf){
                depart_temporisateur(100); 
            }

            cursor = inc(cursor, 16);
            de_application(message, &taille_msg);

        } else {
            // Blocage temporaire : attendre un événement
            if(attendre() == -1){
                de_reseau(&rep); 
                printf("[INFO] Réception d'ACK %d (pré-validation).\n", rep.num_seq);

                if (verifier(rep) && dans_fenetre(borneInf, rep.num_seq, tailleFenetre)) {
                    printf("[INFO] ACK confirmé pour %d.\n", rep.num_seq); 
                    borneInf = inc(rep.num_seq, 16);
                    if(borneInf == cursor){
                        arret_temporisateur(); 
                    }
                } else if (inc(rep.num_seq, 16) == borneInf) {
                    // Traitement des ACKs dupliqués
                    dup_ACK++; 
                    if(dup_ACK == 3){
                        printf("[ALERTE] 3 ACKs identiques reçus : activation fast retransmit.\n");
                        arret_temporisateur(); 
                        int i = borneInf; 
                        depart_temporisateur(100); 
                        while(i != cursor){
                            vers_reseau(&table[i]); 
                            i = inc(i, 16); 
                        }
                        dup_ACK = 0;
                    }
                }
            } else {
                // Timeout : retransmission standard
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
