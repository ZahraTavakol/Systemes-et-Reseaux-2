# Protocoles de transfert fiable de données (SR2)

Ce dépôt contient mon travail réalisé dans le cadre des TP de l’UE SR2 (Systèmes et Réseaux 2), dans la Licence Informatique à l’Université Toulouse III – Paul Sabatier.

## 🎯 Objectif
L’objectif de ces TP est de développer plusieurs **protocoles de transport** fiables en **langage C**, en se basant sur une architecture réseau à 3 couches (application, transport, réseau). L'accent est mis sur la **fiabilité du transfert de données**, en intégrant des mécanismes tels que :
- le **contrôle de flux**
- la **détection et correction d’erreurs**
- la **gestion de retransmissions**

##  Compétences développées

Au cours de ce projet, j’ai appris à :
- Concevoir des **protocoles de communication** du type Stop-and-Wait, Go-Back-N, et Selective Repeat
- Implémenter des **structures de paquets (paquet_t)** et gérer les champs de contrôle (somme, séquence, etc.)
- Gérer les **temporisateurs, acquittements (ACK/NACK)**, et retransmissions en cas d’erreurs ou de pertes
- Comprendre les **modèles réseau orientés couches**, notamment la couche transport
- Tester mes implémentations dans des environnements simulant des erreurs et des pertes de paquets

##  Organisation du projet

Chaque version du protocole est représentée par deux fichiers principaux :
- `proto_tdd_vX_emetteur.c`
- `proto_tdd_vX_recepteur.c`

X correspond à la version du protocole :
- `v0` : Pas de garantie
- `v1` : Stop-and-Wait avec NACK
- `v2` : Stop-and-Wait avec timeout
- `v3` : Go-Back-N (et amélioration `v3.2` avec Fast Retransmit)
- `v4` : Selective Repeat

Le code partagé est centralisé dans `couche_transport.c`.
