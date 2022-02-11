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
  for (i=0 ; i<lg; i++){
    message[i] = motif;
  }
  
}

void afficher_message (char *message, int lg) {
  int i;
  
  for (i=0 ; i<lg ; i++){
    printf("%c", message[i]) ;
  }
}

void format_numero_message(int numero){
  
  if(numero > 99){
    printf("[%d", numero);
  }else if(numero > 9){
    printf("[-%d", numero);
  }else{
    printf("[--%d", numero);
  }
}


void main (int argc, char **argv)
{
	int c;
	char alphabet[10] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', '\0'};
	extern char *optarg;
	extern int optind;
	int nb_message = 10; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int lg_message = 30;
	int source = -1 ; /* 0=puits, 1=source */
	int prot = -1; /* TCP par default et 1=UDP */
	
	while ((c = getopt(argc, argv, "pn:sul:")) != -1) {
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
		case 'l' :
		  lg_message = atoi(optarg);
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



	/// Debut programme UDP ///

	int sock;
	struct sockaddr_in adr_local;
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

	    printf("PUITS:lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s\n", lg_message, atoi(argv[argc-1]), nb_message, "UDP");
	    
	    for(int i = 0; i<nb_message; i++){
	      M = (char *)malloc(lg_message*sizeof(char));
	      if(recvfrom(sock, M, lg_message, 0, (struct sockaddr*)&adr_local, &p_adr_local) <0){
		printf("Echec reception");
		exit(1);
	      }

	       printf("PUITS : Envoi n°%d (%d)", i+1, lg_message);
	     format_numero_message(i+1);
	    afficher_message(M, lg_message-3);
	    printf("]\n"); 
	    
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

	  printf("SOURCE:lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest =%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP", argv[argc-2]);
	  
	  for(int i = 0; i < nb_message; i++){
	    M = (char *)malloc(lg_message*sizeof(char));
	    construire_message(M, 'a', lg_message);
	    sendto(sock, M, lg_message, 0, (struct sockaddr *)&adr_dist, sizeof(struct sockaddr));


	     printf("SOURCE : Envoi n°%d (%d)", i+1, lg_message);
	     format_numero_message(i+1);
	     afficher_message(M, lg_message-3);
	     printf("]\n");
	    
	  }

	  
	    
	  } // end source
	}else{

	  ////////////////////////////  TCP    ///////////////////////////////////////////////////
	  int sock_bis;

	  // Puit //
	  if(source == 0){

	    printf("PUITS:lg_mesg_lu=%d, port=%d, nb_reception=%d, TP=%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP");
	    
	    // Création socket
	    sock = socket(AF_INET, SOCK_STREAM, 0);

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

	    // Ecoute du serveur sur le port
	    if(listen(sock, 30) == -1){
	      printf("echec du listen");
	      exit(1);
	    }
	    
	    if((sock_bis = accept(sock, (struct sockaddr *)&adr_local, &p_adr_local)) == -1){
	      
	      printf("echec du accept");
	      exit(1);
	    }

	    

	    for(int i = 0; i<nb_message; i++){
	      M = (char *)malloc(lg_message*sizeof(char));
	      lg_message = recv(sock_bis, M, lg_message, 0);
	      if(lg_message <0){	
		perror("Echec reception");
		exit(1);
	      }

	       printf("PUITS : Reception n°%d (%d)", i+1, lg_message);
	       format_numero_message(i+1);
	       afficher_message(M, lg_message-3);
	       printf("]\n");
	    
	    } // end for
	    printf("PUITS : fin\n");
	  }else{ // end puit

	    // Source //
	    struct sockaddr_in adr_dist;
	    struct hostent *hp;

	    // Création socket
	    sock = socket(AF_INET, SOCK_STREAM, 0);

	    // Création @socket distant
	    adr_dist.sin_family = AF_INET;
	    adr_dist.sin_port = htons(atoi(argv[argc-1]));

	    if((hp = gethostbyname(argv[argc-2])) == NULL){
	      printf("Erreur gethostbyname");
	      exit(1);
	    }

	    
	    
	    memcpy((char *)&(adr_dist.sin_addr.s_addr), hp->h_addr, hp->h_length);

	    if( sock_bis = connect(sock, (struct sockaddr*)&adr_dist,sizeof(adr_dist)) == -1){
	      printf("echec connect");
	      exit(1);
	    }

	  // Envoie de Message
	  printf("source : %d\n", lg_message);
	  nb_message = 10;
	  int counter = 0;
	  

	  printf("SOURCE:lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest =%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP", argv[argc-2]);

	  for(int i = 0; i < nb_message; i++){
	    if(counter == sizeof(alphabet) -1){
	      counter = 0;
	    }
	   
	    M = (char *)malloc(lg_message*sizeof(char));
	    char motif_message = alphabet[counter];
	    construire_message(M, motif_message, lg_message);
	    send(sock, M, lg_message, 0);
	     counter++;

	     printf("SOURCE : Envoi n°%d (%d)", i+1, lg_message);
	     format_numero_message(i+1);
	    afficher_message(M, lg_message-3);
	    printf("]\n");
	    
	    
	    
	  }
	}
	
	  
	}
}	
