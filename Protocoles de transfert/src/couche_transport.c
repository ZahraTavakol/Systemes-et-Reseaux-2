#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"
#include <stdbool.h>



/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

uint8_t generator(paquet_t paquet){
    uint8_t somme_ctrl = 0;
    somme_ctrl = paquet.lg_info ^ 
    paquet.num_seq ^ 
    paquet.type;
    for(int i = 0 ; i  < paquet.lg_info; i++){
      somme_ctrl = somme_ctrl ^ paquet.info[i];
    }
    return somme_ctrl;
}

//incremente les valeurs
int increase(int n){ 
  return (n + 1);
}
int inc(int n,int m){ 
  return (n + 1%m);
}


bool verifier(paquet_t paquet){ // verifier que les sommes de controle sont egaux

    generator(paquet);
    return paquet.somme_ctrl == generator(paquet);

}

/*int verifier_controle(paquet_t paquet){ 

    return paquet.somme_ctrl == generer_controle(paquet);

}*/


/* ===================== Fenêtre d'anticipation ============================= */

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
