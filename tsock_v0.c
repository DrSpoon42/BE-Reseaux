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
	struct Lettre * lettre_suiv;
};

struct Config_envoie{
	int nb; // Nombre de lettre à envoyer par le serveur
	int taille_first; // Taille de la premiere lettre à envoyer par le serveur
};


struct Liste{
	int identifiant; // Identifiant de l'emetteur
	struct Lettre * lettre_start;
	struct Lettre  * lettre_courant;
	struct Lettre  * lettre_fin;
	struct Liste * liste_suiv;
};

struct BAL{
	struct Liste * liste_start;
	struct Liste  * liste_courant;
	struct Liste  * liste_fin;
};

struct Envoie_lettre{
	char * lettre;
	int taille_lettre_suiv;
}; 

void Initialisation_BAL(struct BAL * box){

	box->liste_start = NULL;
	box->liste_courant = NULL;
	box->liste_fin = NULL;
}

void Initialisation_Liste(struct Liste * liste){

	liste->lettre_courant = NULL;
	liste->lettre_start = NULL;
	liste->lettre_fin = NULL;
	liste->liste_suiv = NULL;
	liste->identifiant = 0;

}

//On passe la liste concernée et la lettre à ajouter
void ajouter_a_liste(struct Liste * liste, struct Lettre * lettre){
	if(liste->lettre_start == NULL){
		liste->lettre_start = lettre;
		liste->lettre_courant = lettre;
		liste->lettre_fin = lettre;
		liste->lettre_fin->lettre_suiv = NULL;
	}else{
		// On ajoute en fin de liste
		struct Lettre * tmp;
		tmp = liste->lettre_fin;
		liste->lettre_fin = lettre;
		tmp->lettre_suiv = liste->lettre_fin;
	}
}

void ajouter_une_liste_a_BAL(struct BAL * box, struct Liste *liste){
	if(box->liste_start == NULL){
		box->liste_start = liste;
		box->liste_courant = liste;
		box->liste_fin = liste;
	}else{
		// On ajoute en fin de liste
		struct Liste * tmp;
		tmp = box->liste_fin;
		box->liste_fin = liste;
		tmp->liste_suiv = box->liste_fin;
	}
	box->liste_fin->liste_suiv = NULL;
}
void afficher_une_liste(struct Liste * liste){
	struct Lettre * lettre_tmp;
	lettre_tmp = malloc(sizeof(struct Lettre));
	lettre_tmp = liste->lettre_start;
	while(lettre_tmp != NULL){
		printf("taille : %d & message : %s\n", lettre_tmp->taille_lettre, lettre_tmp->M);
		lettre_tmp = lettre_tmp->lettre_suiv;
	}

	free(lettre_tmp);
}

int count(struct Liste * l){
	int counter = 0;
	struct Lettre * tmp;
	tmp = malloc(sizeof(struct Lettre));
	tmp = l->lettre_start;
	while(tmp != NULL){
		counter++;
		tmp = tmp->lettre_suiv;
	}

	return counter;
}

void afficher_une_bal(struct BAL * bal){
	struct Liste * liste_tmp;
	liste_tmp = malloc(sizeof(struct Liste));
	liste_tmp = bal->liste_start;
	while(liste_tmp != NULL){
		printf("La liste d'identifiant numéro %d a %d lettres\n" ,liste_tmp->identifiant ,count(liste_tmp));
		liste_tmp = liste_tmp->liste_suiv;
		
	}

	free(liste_tmp);
}


// Return 1 si la liste existe | O sinon 
int check_is_exist(struct BAL * bal, int id){
	struct Liste * liste_tmp = malloc(sizeof(struct Liste));
	liste_tmp = bal->liste_start;
	int find = 0;

	while(find == 0 && liste_tmp != NULL){
		if(liste_tmp->identifiant == id){
			find = 1;
		}else{
			liste_tmp = liste_tmp->liste_suiv;
		}	
	}
	return find;
}

struct Liste * get_liste_from_id(struct BAL * bal , int id){
	struct Liste * tmp;
	tmp = malloc(sizeof(struct Liste));
	tmp = bal->liste_start;
	while(tmp->identifiant != id && tmp != NULL){
		tmp = tmp->liste_suiv;
	}

	return tmp;
}

// Classe authentification
struct Auth{
	int role;  // Role == 0 : Emetteur ; Role  == 1 : Recepteur
	int nb_lettres;
	int taille_lettre; 
	int id; //Numéro de lémetteur ou recepteur
};

void construire_message (char *message, char motif, int lg) {
  int i;
  for (i=0 ; i<lg; i++){
    message[i] = motif;
  }
  message[lg] = '\0';

  
}

void afficher_message (char *message, int lg) {
  int i;
  
  for (i=0 ; i<lg ; i++){
	  //printf("indice tab = %d\n", i);
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
		  source = 1;
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
	    
		while(1){
			 for(int i = 0; i<nb_message; i++){
				M = (char *)malloc(lg_message*sizeof(char));
				if(recvfrom(sock, M, lg_message, 0, (struct sockaddr*)&adr_local, &p_adr_local) <0){
					printf("Echec reception");
					exit(1);
				}

					printf("PUITS : Réception n°%d (%d)", i+1, lg_message);
					format_numero_message(i+1);
					afficher_message(M, lg_message-3);
					printf("]\n"); 
				
				} // end for
		}

	   

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
	  int counter = 0;

	  printf("SOURCE:lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest =%s\n", lg_message, atoi(argv[argc-1]), nb_message, "UDP", argv[argc-2]);
	  
		for(int i = 0; i < nb_message; i++){
			if(counter == sizeof(alphabet) -1){
				counter = 0;
			}
					
					
			M = (char *)malloc((lg_message+1)*sizeof(char));
			char motif_message = alphabet[counter];
			construire_message(M, motif_message, lg_message);
			sendto(sock, M, lg_message, 0, (struct sockaddr *)&adr_dist, sizeof(struct sockaddr));
			counter++;

			printf("SOURCE: Envoie n°%d (%d) ", i+1, lg_message);
			format_numero_message(1+i);
			afficher_message(M, lg_message-3);
			printf("]\n");

		}
				printf("SOURCE: fin\n");

	 

	  
	    
	  } // end source
	}else{

	  ////////////////////////////  TCP    ///////////////////////////////////////////////////
	  	int sock_bis;
		struct sockaddr_in adr_dist;
	    struct hostent *hp;

	  // Puit 
	  if(source == 0){

		  if(bal != 1){
			  	printf("lg_mesg_lu=%d, port=%d, nb_reception=%d, TP=%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP");
		  }else{
			  //Affichage
				printf("PUITS: port=%d, TP=tcp\n", atoi(argv[argc-1]));
		  }
	  
	    
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

		// Initialisation des variables
			struct BAL * bal_liste;
			bal_liste = malloc(sizeof(struct BAL));
			Initialisation_BAL(bal_liste);

			lg_message++;
	    
		while((sock_bis = accept(sock, (struct sockaddr *)&adr_local, &p_adr_local)) != -1){

			// bal  == 1 : on est en mode boite aux lettres sinon on est en puit standard
			if(bal == 1){

				
				// Initialisation des variables
				struct Auth * M_Auth;
				struct Liste * liste;
				liste = malloc(sizeof(struct Liste));
			
				// Récupération du message d'authentification et affichage des données
				M_Auth = (struct Auth *)malloc(sizeof(struct Auth));
				int lg_message_auth = recv(sock_bis, M_Auth, sizeof(struct Auth), 0);
				

				// Emetteur
				if(M_Auth->role  == 0){

					// On check si il existe déja une liste dans la BAL
					if(!check_is_exist(bal_liste, M_Auth->id)){

						// La liste n'existe pas
						// On initialise la Liste
						Initialisation_Liste(liste);
						// l'ID de la liste correspond a l'id de l'emmeteur
						liste->identifiant = M_Auth->id;

					}else{

						// Sinon il existe une liste, donc on récupère la liste deja existante
						liste = get_liste_from_id(bal_liste, M_Auth->id);
			
					}

					// On récupère toutes les lettres envoyées par l'emetteur
					for(int i = 0; i < M_Auth->nb_lettres; i++){

						//On initialise la lettre
						struct Lettre  * lettre;
						lettre = malloc(sizeof(struct Lettre));

						//Initialisation et Affectation des champs de la lettre
						lettre->taille_lettre = M_Auth->taille_lettre;
						lettre->M = (char *)malloc(lettre->taille_lettre*sizeof(char));
						lettre->lettre_suiv = NULL;
						lg_message = recv(sock_bis, lettre->M, lettre->taille_lettre, 0);

						printf("PUITS: Réception et stockage lettre n°%d pour le recepteur n°%d", i+1, M_Auth->id);
						format_numero_message(M_Auth->id);
						afficher_message(lettre->M, lettre->taille_lettre-4);
						printf("]\n");

						if(lg_message <0){	
							perror("Echec reception");
							exit(1);
						}

						// Ajout de la lettre a la liste
						ajouter_a_liste(liste, lettre);

						//afficher_une_liste(liste);

					} // end for

					

					// Ajout de la liste a la bal si la liste n'existe pas
					if(!check_is_exist(bal_liste, M_Auth->id)){
						ajouter_une_liste_a_BAL(bal_liste, liste);
					}
					//afficher_une_bal(bal_liste);
					//afficher_une_liste(liste);
					
					// Fermeture du socket correspondant a la transimission
					if((close(sock_bis) )== -1){
						perror("fermeture connexion error");
						exit(1);
					};

				}else if(M_Auth->role == 1){ // Recepteur

					// On check si la liste correspondante à l'id existe
					if(check_is_exist(bal_liste, M_Auth->id)){
					
						// La liste existe
						// On récupère la liste concernée
						liste = get_liste_from_id(bal_liste, M_Auth->id);
						int lg_message = liste->lettre_start->taille_lettre;
					
						// Creation du message qui dit le nb de lettre qu'on va envoyer
						// Ainsi que la taille de la premiere lettre
						int nb_lettres = count(liste);
						struct Config_envoie * config_envoie;
						config_envoie = malloc(sizeof(struct Config_envoie));
						config_envoie->nb = nb_lettres;
						config_envoie->taille_first = liste->lettre_start->taille_lettre;


						// Envoie du message de config
						send(sock_bis, config_envoie, sizeof(struct Config_envoie), 0);

						struct Lettre * lettre = malloc(sizeof(struct Lettre));
						lettre = liste->lettre_start;
						
						// Envoie de toute les lettres
						while(lettre != NULL){
							//perror("je suis dans l'envoie de liste");

							// Lettre tampon
							struct Lettre * lettre_tmp;
							lettre_tmp = malloc(sizeof(struct Lettre));
							lettre_tmp = lettre->lettre_suiv;
							int taille_lettre_suiv = 0;

							// On veut envoyer le message et la taille de la lettre suivante

							//printf("le mesage a envoyer est  %s\n", lettre->M);

							// Gestion si on est en fin de liste
							if(lettre_tmp == NULL){
								taille_lettre_suiv = -1;
							}else{
								taille_lettre_suiv = lettre_tmp->taille_lettre;
							}  
							
							// Envoie du message
							//printf("taille lettre a envoyer : %d\n", lettre->taille_lettre);
							send(sock_bis, lettre->M, lettre->taille_lettre, 0);

							// Envoie de la taille de la lettre suivante
							send(sock_bis, &taille_lettre_suiv, sizeof(int), 0);

							// incrementation du pointeur de liste
							lettre = lettre->lettre_suiv;

						} // end while


						// Libertion de la memeoire de la liste
						free(liste);


					}else{

						// La liste n'existe pas
						struct Config_envoie * config_envoie;
						config_envoie = malloc(sizeof(struct Config_envoie));
						config_envoie->nb = 0;
						config_envoie->taille_first = 0;

						send(sock_bis, config_envoie, sizeof(struct Config_envoie), 0);
					}

				} // end recepteur

			}else{ // On est en TCP normal

				printf("%d",nb_message);
				for(int i = 0; i<nb_message; i++){
					M = (char *)malloc(lg_message*sizeof(char));
					lg_message = recv(sock_bis, M, lg_message, 0);
					if(lg_message <0){	
						perror("Echec reception");
						exit(1);
					}
					printf("PUITS : Reception n°%d (%d)", i+1, lg_message-1);
					format_numero_message(i+1);
					afficher_message(M, lg_message-3);
					printf("]\n");

				} // end for	
	  		}	
		}// End while accept
			
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
	    if(connect(sock, (struct sockaddr*)&adr_dist,sizeof(adr_dist) )== -1){
	      perror("echec connect bal");
	      exit(1);
	    }

		
			// On creer le message d'authentification pour l'emmeteur et recepteur

			// Role == 0 : Emetteur ; Role  == 1 : Recepteur
			if(role == 0){

				// On implemente la structure auth
				struct Auth auth;
				auth.role = 0;
				auth.nb_lettres = nb_message;
				auth.taille_lettre = lg_message + 1;
				auth.id = numero_e;

				// Envoie message authentification a BAL
				send(sock, &auth, sizeof(struct Auth), 0);

				 // Envoie de Message
				int counter = 0;
				

				printf("SOURCE: lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest=%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP", argv[argc-2]);
				for(int i = 0; i < nb_message; i++){
					if(counter == sizeof(alphabet) -1){
					counter = 0;
					}
					
					
					M = (char *)malloc((lg_message+1)*sizeof(char));
					char motif_message = alphabet[counter];
					construire_message(M, motif_message, lg_message);
					send(sock, M, lg_message+1, 0);
					counter++;

					printf("SOURCE: Envoi lettre n°%d à destination du recepteur %d (%d)", i+1, numero_e , lg_message);
					format_numero_message(numero_e);
					afficher_message(M, lg_message-3);
					printf("]\n");

				}
				printf("SOURCE: fin\n");

			}else if (role == 1){

				struct Auth auth;
				auth.role = 1;
				auth.nb_lettres = -1;
				auth.taille_lettre = -1;
				auth.id = numero_r;

				// Envoie message authentification a BAL
				send(sock, &auth, sizeof(struct Auth), 0);

				// Récupération des messages de BAL

				// Récupération du nombre de message que l'on va recevoir
				struct Config_envoie * config_envoie;		
				config_envoie = (struct Config_envoie *)malloc(sizeof(struct Config_envoie));
				if((recv(sock, config_envoie , sizeof(struct Config_envoie), 0)) == -1){
					perror("erreur recep");
				};

				// Initialisation des variables avec les données renvoyées par bal
				int nb_lettre_reception = config_envoie->nb;
				int taille_first = config_envoie->taille_first;

				// Traitement
				if(nb_lettre_reception == 0){
					printf("Il n'y a pas de lettre dans la boite aux lettre\n");

					
				}else{

					printf("RECEPTION: Demande de récupération de ses lettres par le récepteur %d port=%d, TP=tcp, dest=%s\n", numero_r,atoi(argv[argc-1]), argv[argc-2]);
					// Recuperation des lettres
					for(int i = 0; i < nb_lettre_reception; i++){

						// Initialisation de la structure de reception
						struct Envoie_lettre * recep;
						recep = (struct Envoie_lettre *)malloc(sizeof(struct Envoie_lettre));
						recep->taille_lettre_suiv = 0;
						recep->lettre = (char *)malloc(taille_first*sizeof(char));


						lg_message = recv(sock, recep->lettre, taille_first, 0);
						if(lg_message <0){	
							perror("Echec reception");
							exit(1);
						}

						lg_message = recv(sock, &recep->taille_lettre_suiv, sizeof(int), 0);
						if(lg_message <0){	
							perror("Echec reception");
							exit(1);
						}

						printf("RECEPTION: Récupération par le récepteur %d (%d) ", numero_r, taille_first-1);
						format_numero_message(numero_r);
						afficher_message(recep->lettre, taille_first-4);
						printf("]\n");

						taille_first = recep->taille_lettre_suiv;
					} // end for 

					printf("RECEPTION: fin\n");
				} 
				

				
			}else{

			

			
				 // Envoie de Message
				int counter = 0;
				

				printf("SOURCE: lg_mesg_emis=%d, port=%d, nb_envois=%d, TP=%s, dest=%s\n", lg_message, atoi(argv[argc-1]), nb_message, "TCP", argv[argc-2]);
				for(int i = 0; i < nb_message; i++){
					if(counter == sizeof(alphabet) -1){
					counter = 0;
				}
					
					
					M = (char *)malloc((lg_message+1)*sizeof(char));
					char motif_message = alphabet[counter];
					construire_message(M, motif_message, lg_message);
					send(sock, M, lg_message+1, 0);
					counter++;

					printf("SOURCE: Envoie n°%d (%d) ", i+1, lg_message);
					format_numero_message(i+1);
					afficher_message(M, lg_message-3);
					printf("]\n");

				}
				printf("SOURCE: fin\n");
		
			}
		

	 

	  //shutdown(sock, 1);
	}
	
	  
	}
}

