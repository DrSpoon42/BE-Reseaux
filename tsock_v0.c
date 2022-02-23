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


// Creation du stockage de la boite aux lettres

struct Lettre{
	int taille_lettre;
	char * M;
	struct Lettre * suiv;
};

struct Liste{
	int identifiant; // Identifiant de l'emetteur
	struct Lettre * lettre_start;
	struct Lettre  * lettre_courant;
	struct Lettre  * lettre_fin;
};

struct BAL{
	struct Liste * liste_start;
	struct Liste  * liste_courant;
	struct Liste  * liste_fin;
};

void Initialisation_BAL(struct BAL * box){

	// On initalise la boite aux lettres
	box = malloc(sizeof(struct BAL));

	box->liste_start = NULL;
	box->liste_courant = NULL;
	box->liste_fin = NULL;
}

// On passe la liste concernée et la lettre a ajouter
void ajouter_a_liste(struct Liste * liste, struct Lettre * lettre){
	if(liste->lettre_start == NULL){
		liste->lettre_start = lettre;
		liste->lettre_courant = lettre;
		liste->lettre_fin = lettre;
	}else{
		// On ajoute en fin de liste
		struct Lettre * tmp;
		tmp = liste->lettre_fin;
		liste->lettre_fin = lettre;
		tmp->suiv = liste->lettre_fin;
	}
}


// Classe authentification
struct Auth{
			int role;  // Role == 0 : Emetteur ; Role  == 1 : Recepteur
			int nb_lettres;
			int taille_lettre; 
};

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
	int role = -1 ;/* 0 = e et 1 = r*/
	int numero_r, numero_e = -1;
	int bal = -1;

	
	while ((c = getopt(argc, argv, "pn:sul:e:r:b")) != -1) {
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
		case 'e':
		  role = 0;
		  source = 1;
		  numero_e = atoi(optarg);
		  break;
		case 'r':
		  role = 1;
		  numero_r = atoi(optarg);
		  break;
		case 'b':
		
			source = 0;
			bal = 1;
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


	int sock;
	struct sockaddr_in adr_local;
	char* M;
	int p_adr_local = sizeof(adr_local);
	

	/// Debut programme UDP ///

	if(prot == 1){

	  // Puit //
	  if(source == 0){

	    // Création socket
	    sock = socket(AF_INET, SOCK_DGRAM, 0);

	    // Création @socket
		memset((char *)&adr_local,0,sizeof(adr_local));
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

	  }else{
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

		afficher_message(M, lg_message-3);
	     printf("]\n");
	    
	  }

	  
	    
	  } // end source
	}else{

	  ////////////////////////////  TCP    ///////////////////////////////////////////////////
	  int sock_bis;
		struct sockaddr_in adr_dist;
	    struct hostent *hp;

	  // Puit 
	  if(source == 0){
	  
	  	printf("lg_mesg_lu=%d, port=%d, nb_reception=%d, TP=%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP");
		printf("bal : %d\n", bal);
	    
	    // Création socket
	    sock = socket(AF_INET, SOCK_STREAM, 0);
		

	    // Création @socket
	    memset((char*)&adr_local, 0, sizeof(adr_local));
	    adr_local.sin_family = AF_INET;
	    adr_local.sin_port = htons(atoi(argv[argc-1]));
		adr_local.sin_addr.s_addr = INADDR_ANY;

		if(bind(sock, (struct sockaddr *)&adr_local, sizeof(adr_local)) == -1){
			perror("erreur bind");
			exit(1);
		}

	    if(listen(sock, 30) == -1){
	      printf("echec du listen");
	      exit(1);
	    }
	    
		if((sock_bis = accept(sock, (struct sockaddr *)&adr_local, &p_adr_local)) == -1){
				printf("echec du accept");
				exit(1);
		}
			
		// bal  == 1 : on est en mode boite aux lettres sinon on est en puit standard
		if(bal == 1){

			struct BAL  * box;

			printf("coucou c'est moi la boiite aux lettres\n");
			struct Auth * M_Auth;
			M_Auth = (struct Auth *)malloc(sizeof(struct Auth));
			int lg_message_auth = recv(sock_bis, M_Auth, sizeof(struct Auth), 0);
			printf("role=%d, nb_lettres=%d, taille_lettre=%d\n", M_Auth->role, M_Auth->nb_lettres, M_Auth->taille_lettre);

			// Emetteur
			if(M_Auth->role  == 0){

				

				// On initialise la Liste
				struct Liste * liste;
				liste = malloc(sizeof(struct Liste));
				
				for(int i = 0; i< M_Auth->nb_lettres; i++){

					
					//On initialise la lettre
					struct Lettre  * lettre;
					lettre = malloc(sizeof(struct Lettre));

					lettre->taille_lettre = M_Auth->taille_lettre;
					lettre->M = (char *)malloc(lettre->taille_lettre*sizeof(char));

					lg_message = recv(sock_bis, lettre->M, lg_message, 0);

					if(lg_message <0){	
						perror("Echec reception");
						exit(1);
					}

					ajouter_a_liste(liste, lettre);
					

				} // end for	
				
			}

			

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


		// Connexion avec le serveur
	    if( sock_bis = connect(sock, (struct sockaddr*)&adr_dist,sizeof(adr_dist)) == -1){
	      perror("echec connect bal");
	      exit(1);
	    }

		

		// On creer le message d'authentification pour ml'emmeteur et recepteur

		// Role == 0 : Emetteur ; Role  == 1 : Recepteur
		if(role == 0){

			// On implemente la structure auth
			struct Auth auth;
			auth.role = 0;
			auth.nb_lettres = nb_message;
			auth.taille_lettre = lg_message;

			// Envoie message authentification a BAL
			send(sock, &auth, sizeof(struct Auth), 0);

		}else if (role == 1){
			struct Auth auth;
			auth.role = 1;
			auth.nb_lettres = -1;
			auth.taille_lettre = -1;

		}

	  // Envoie de Message
	  printf("source : %d\n", lg_message);
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
