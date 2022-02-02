/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>



void construire_message (char *message, char motif, int lg) {
  int i;
  for (i=0 ; i<lg; i++) message[i] = motif;
}

void afficher_message (char *message, int lg) {
  int i;
  printf("message construit : ");
  for (i=0 ; i<lg ; i++){
    printf("%c", message[i]) ;
  }
  printf("\n");
}


void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int prot = -1 ; /* TCP par default et 1=UDP */
	
	while ((c = getopt(argc, argv, "pn:su")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;

		case 'n':
			nb_message = atoi(optarg);
			break;
		case 'u':
		prot = 1;
		break;
		  

		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (source == 1)
		printf("on est dans le source\n");
	else
		printf("on est dans le puits\n");

	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");

	}

	/// Debut programme UDP ///

	int sock;
	struct sockaddr_in adr_local;
	int lg_message = 30;
        nb_message = 10;
	char* M;
	int p_adr_local = sizeof(adr_local);
	

	

	if(prot == 1){

	  // Puit //
	  if(source == 0){

	    // Création socket
	    sock = socket(AF_INET, SOCK_DGRAM, 0);

	    // Création @socket
	    memset((char*)&adr_local, 0, sizeof(adr_local));
	    adr_local.sin_family = AF_INET;
	    adr_local.sin_port = htons(atoi(argv[argc-1]));
	    adr_local.sin_addr.s_addr = INADDR_ANY;

	    // Association @socket et socket
	    if(bind(sock, (struct sockaddr*)&adr_local, sizeof(adr_local)) == -1){
	      printf("Echec du bind");
	      exit(1);
	    }

	    for(int i = 0; i<nb_message; i++){
	      M = (char *)malloc(lg_message*sizeof(char));
	      if(recvfrom(sock, M, lg_message, 0, (struct sockaddr*)&adr_local, &p_adr_local) <0){
		printf("Echec reception");
		exit(1);
	      }

	      afficher_message(M, lg_message); 
	    
	    } // end for
	  }else{ // end puit

	    // Source //
	    struct sockaddr_in adr_dist;
	    struct hostent *hp;

	    // Création socket
	    sock = socket(AF_INET, SOCK_DGRAM, 0);

	    // Création @socket distant
	    adr_dist.sin_family = AF_INET;
	    adr_dist.sin_port = htons(atoi(argv[argc-1]));

	    if((hp = gethostbyname(argv[argc-2])) == NULL){
	      printf("Erreur gethostbyname");
	      exit(1);
	    }
	    
	    memcpy((char *)&(adr_dist.sin_addr.s_addr), hp->h_addr, hp->h_length);

	  // Envoie de Message
	  nb_message = 10;
	  for(int i = 0; i < nb_message; i++){
	    M = (char *)malloc(lg_message*sizeof(char));
	    construire_message(M, 'a', lg_message);
	    sendto(sock, M, lg_message, 0, (struct sockaddr *)&adr_dist, sizeof(struct sockaddr));
	    afficher_message(M, lg_message);
	  }

	  
	    
	  } // end source
	}
	
	  
	
}

