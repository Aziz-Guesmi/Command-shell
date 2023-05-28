#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>


#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW    "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"


void removeWhiteSpace(char* lineCommande){
	if(lineCommande[strlen(lineCommande)-1]==' ' || lineCommande[strlen(lineCommande)-1]=='\n')
		lineCommande[strlen(lineCommande)-1]='\0';

	if(lineCommande[0]==' ' || lineCommande[0]=='\n') 
		memmove(lineCommande, lineCommande+1, strlen(lineCommande));
}

void tokenize_buffer(char** param,int *NbOfCommande,char *lineCommande,const char *c){
	char *token;
	token=strtok(lineCommande,c);
	int pc=-1;
	while(token){
		param[++pc]=malloc(sizeof(token)+1);
		strcpy(param[pc],token);
		removeWhiteSpace(param[pc]);
		token=strtok(NULL,c);
	}
	param[++pc]=NULL;
	*NbOfCommande=pc;
}

void executeBasic(char** argv){

	if(fork()!=0){
		wait(NULL);
	}
	else{

		if (execvp(argv[0],argv) < 0 )
		{
			perror(RED   "Cette commande n’existe pas ou ne peut pas être exécutée"   COLOR_RESET );
			exit(1);
		}
		
	}
}

void PIPE(char** lineCommande,int NbOfCommande){
	if(NbOfCommande>10) return;

	int fd[10][2],i,pc;
	char *argv[100];

	if(strlen((lineCommande[0]))==0 ) 
		printf("Commande invalide\n");
	else {
		for(i=0;i<NbOfCommande;i++){
			tokenize_buffer(argv,&pc,lineCommande[i]," ");
			if(i!=NbOfCommande-1){
				if(pipe(fd[i])<0){
					perror("pipe creating was not successfull\n");
					return;
				}
			}
			if(fork()==0){
				if(i!=NbOfCommande-1){
					dup2(fd[i][1],1);
					close(fd[i][0]);
					close(fd[i][1]);
				}

				if(i!=0){
					dup2(fd[i-1][0],0);
					close(fd[i-1][1]);
					close(fd[i-1][0]);
				}
				execvp(argv[0],argv);
				perror("invalid input ");
				exit(1);
			}

			if(i!=0){
				close(fd[i-1][0]);
				close(fd[i-1][1]);
			}
			wait(NULL);
		}
	}
}

void AND(char** lineCommande,int NbOfCommande){
	int i,pc,test=0;
	char *argv[100];
	    if(strlen((lineCommande[0]))==0 ) printf("Commande invalide\n");
		else {
		for(i=0;i<NbOfCommande;i++){
		if (strlen(lineCommande[i])==0)
		printf (RED "Commande invalide");
		else {
		tokenize_buffer(argv,&pc,lineCommande[i]," ");
		if(fork()!=0){
			wait(NULL);
		}
		else{
			if ( execvp(argv[0],argv) < 0)
			{perror(RED   "Commande invalide"   COLOR_RESET );
			break;
			exit(1);
			}
		}
		}
		}
	}
}

void OR(char** lineCommande,int NbOfCommande){

	int pc;
	char *argv[100];
	if(strlen((lineCommande[0]))==0 ) printf("Commande invalide\n");
	else {
		for(int i=0;i<NbOfCommande;i++){
			tokenize_buffer(argv,&pc,lineCommande[i]," ");
			if(fork()!=0){
				wait(NULL);
				break;
			}else{
				if (execvp(argv[0],argv) != -1 ){	
					break;
				}
			}
		}
	}
}

void PV(char** lineCommande,int NbOfCommande){
	int pc;
	char *argv[100];

	if(strlen((lineCommande[0]))==0 ) 
	printf("Commande invalide\n");
	else {
		for(int i=0;i<NbOfCommande;i++){
			tokenize_buffer(argv,&pc,lineCommande[i]," ");
			if(fork()==0){
				execvp(argv[0],argv);
				printf(RED "Cette commande ( %s ) n’existe pas ou ne peut pas être exécutée \n" COLOR_RESET, lineCommande[i])	;
				exit(1);
			}else{
				wait(NULL);
			}
		}
	}
}

void History(){

	char lineCommande[500];
	int i = 1;
	FILE *file = fopen("history","r");
	if (file != NULL){
		while (fgets(lineCommande,sizeof(lineCommande), file))
		{
			printf("  %d %s",i++,lineCommande);
		}
		fclose(file);
	}
}

void UpdateHistory(char lineCommande[500]){
	
	FILE *file = fopen("history","a+");
	fputs(lineCommande, file);
	fclose(file);	

}

void executeRedirect(char** lineCommande,int NbOfCommande){
	int pc,fd;
	char *argv[100];
	removeWhiteSpace(lineCommande[1]);
	tokenize_buffer(argv,&pc,lineCommande[0]," ");
	if(strlen((lineCommande[0]))==0 ) 
		printf("Commande invalide\n");
	else {
		if(fork()==0){
			
			FILE *f = fopen(lineCommande[1],"a+");
			fd=open(lineCommande[1],O_WRONLY); 
			dup2(fd,1);
			execvp(argv[0],argv);
			perror(RED "Commande invalide" COLOR_RESET);
			exit(1);
		}else{
			wait(NULL);
		}
	}
}

int ExecuteCMD (char lineCommande[500]){
	char *param[100],*ListOfCommand[100];
	int NbOfCommande=0;
	removeWhiteSpace(lineCommande);

	if (((strstr(lineCommande,"&&")-lineCommande)==0) ||
	((strstr(lineCommande,"||")-lineCommande)==0) 
	|| ((strstr(lineCommande,";")-lineCommande)==0) ||
	((strstr(lineCommande,"|")-lineCommande)==0) ||
	((strstr(lineCommande,">")-lineCommande)==0)
	) {
		printf ("Commande invalide \n");
	}
	else 
		if(strstr(lineCommande,"||")){
		
		tokenize_buffer(ListOfCommand,&NbOfCommande,lineCommande,"||");
		OR(ListOfCommand,NbOfCommande);
	}
	else if(strchr(lineCommande,'|')){
		tokenize_buffer(ListOfCommand,&NbOfCommande,lineCommande,"|");
		PIPE(ListOfCommand,NbOfCommande);
	}
	else if(strstr(lineCommande,"&&")){
		tokenize_buffer(ListOfCommand,&NbOfCommande,lineCommande,"&&");
		AND(ListOfCommand,NbOfCommande);
	}
	else if(strchr(lineCommande,';')){
		tokenize_buffer(ListOfCommand,&NbOfCommande,lineCommande,";");
		PV(ListOfCommand,NbOfCommande);
	}
	else if(strchr(lineCommande,'>')){
		tokenize_buffer(ListOfCommand,&NbOfCommande,lineCommande,">");
		if(NbOfCommande==2)executeRedirect(ListOfCommand,NbOfCommande);
		else printf(RED "Format de commande est incorrect \n" COLOR_RESET);
	}
	else{
		tokenize_buffer(param,&NbOfCommande,lineCommande," ");
		if(strstr(param[0],"cd")){
			chdir(param[1]);
		}
		else if(strstr(param[0],"quit") || strstr(param[0],"eof")){
			return 1;
		}
		else if(strstr(param[0],"history")){
			History();
		}
		else executeBasic(param);
	}
return 0;
}

void InteractiveMode(){
	char lineCommande[500],cwd[1024];
	
	printf(YELLOW   "Entrer la commande quit pour quitter:\n" COLOR_RESET);
	while(1){
		
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			printf(GREEN "%s " COLOR_RESET, strcat(cwd,"%"));
		}
		else 	
			perror("getcwd failed\n");

		fgets(lineCommande, 500, stdin);
		UpdateHistory(lineCommande);
		int Quitter = ExecuteCMD(lineCommande);
		if(Quitter) exit(0);
	}
}

void BatchMode (char *argv[]){
	FILE *file;
	char lineCommande[500];

	file = fopen(argv[1],"r");
	
	if (file == NULL){
			printf(RED "Fichier batch non existant \n" COLOR_RESET);
			exit(0);
	}

	while(fgets(lineCommande,250,file) != NULL)
	{		
			UpdateHistory(lineCommande);
			printf("____________________________________________________________________________________\n ");
			printf( YELLOW "%s\n" COLOR_RESET,lineCommande );
			printf("----------------------------------\n ");

			lineCommande[strlen(lineCommande)-1]='\0';
			int Quitter = ExecuteCMD(lineCommande);
			if(Quitter) exit(0);
	}
}

void main( int argc, char *argv[] )  {

	switch(argc){
		case 1 : InteractiveMode();break;
		case 2 : BatchMode(argv);break;
		default: printf(RED "Vous avez ajouter plus que 2 arguments dans l’invite de commande  \n" 
		COLOR_RESET); break;
	}	
}	