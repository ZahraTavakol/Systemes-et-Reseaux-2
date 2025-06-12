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

    int tailleFenetre; // Taille de la fenêtre glissante

    // Vérifie que le bon nombre de paramètres est fourni
    if(argc > 2){
        perror("Nombre de paramètres incorrect");
        return 1;
    }

    unsigned char message[MAX_INFO]; // Données reçues de l'application
    int taille_msg; // Longueur du message
    paquet_t rep; // Paquet pour l'ACK ou données

    int cursor = 0; // Prochain numéro de séquence à utiliser
    int borne_Inf = 0; // Début de la fenêtre glissante
    int acquitte[CAP_SEQ] = {0}; // Suivi des acquittements
    int retransmis[CAP_SEQ] = {0}; // Compteur de retransmissions
    paquet_t table[CAP_SEQ]; // Stocke les paquets envoyés en attente d'ACK

    // Définir la taille de la fenêtre à partir de l'argument ou par défaut à 4
    if(argc == 2){
        tailleFenetre = atoi(argv[1]);
    } else {
        tailleFenetre = 4;
    }

    if (tailleFenetre >= CAP_SEQ) {
        perror("Erreur : la taille de la fenêtre ne peut pas être supérieure à la capacité de séquence.");
        return 1;
    }

    init_reseau(EMISSION);
    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    de_application(message, &taille_msg);

    // Continuer tant qu'il reste des données à envoyer ou à acquitter
    while ((retransmis[borne_Inf] < 20) && ((taille_msg != 0) || (borne_Inf != cursor))) {
        if (dans_fenetre(borne_Inf, cursor, tailleFenetre) && taille_msg > 0) {
            // Construction du paquet à envoyer
            for (int i = 0; i < taille_msg; i++) {
                table[cursor].info[i] = message[i];
            }
            table[cursor].num_seq = cursor;
            table[cursor].type = DATA;
            table[cursor].lg_info = taille_msg;
            table[cursor].somme_ctrl = generator(table[cursor]);

            // Envoi du paquet et démarrage du timer
            vers_reseau(&table[cursor]);
            depart_temporisateur_num(cursor, 100);
            printf("[TRP] Paquet %d transmis.", cursor);

            cursor = inc(cursor, CAP_SEQ);
            de_application(message, &taille_msg);
        } 
        else {
            // Attente d'un événement : ACK ou timeout
            if (attendre() == -1) {
                de_reseau(&rep);

                // Vérification de l'ACK reçu
                if (verifier(rep) && dans_fenetre(borne_Inf, rep.num_seq, tailleFenetre)) {
                    retransmis[rep.num_seq] = 0;
                    arret_temporisateur_num(rep.num_seq);
                    printf("[TRP] ACK reçu pour %d.", rep.num_seq);

                    if(rep.num_seq == borne_Inf){
                        // Avance la fenêtre glissante
                        acquitte[borne_Inf] = 0;
                        borne_Inf = inc(borne_Inf, CAP_SEQ);
                        while (acquitte[borne_Inf] == 1){
                            acquitte[borne_Inf] = 0;
                            borne_Inf = inc(borne_Inf, CAP_SEQ);
                        }

                        // Stoppe les temporisateurs hors de la nouvelle fenêtre
                        for (int i = borne_Inf - tailleFenetre; i < borne_Inf; i++) {
                            arret_temporisateur_num(i % tailleFenetre);
                        }
                    } else {
                        acquitte[rep.num_seq] = 1;
                    }
                }
            } 
            else {
                // Timeout détecté, retransmission nécessaire
                depart_temporisateur_num(attendre(), 100);
                vers_reseau(&table[attendre()]);
                retransmis[attendre()]++;
            }
        }
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
