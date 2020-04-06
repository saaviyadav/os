#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
char cwd[1024];
pid_t all_children[100];
int top_index = 0;
/* Splits the string by space and returns the array of tokens
*
*/

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void mainHandler(int sig_num) 
{   
	signal(SIGINT, mainHandler);
	printf("\n");
}

void execute(char **tokens, int type){
	int num_of_args=0;

	for(int i=0;tokens[i]!=NULL;i++) {
		if(!strcmp(tokens[i],"&") || !strcmp(tokens[i],"&&") || !strcmp(tokens[i],"&&&")) break;
		else num_of_args++;
	}
	char* args[num_of_args+1];
	for(int i=1; i<num_of_args; i++) args[i]=tokens[i];

	pid_t child;
	if (!strcmp(tokens[0],"cd")) {
		if (num_of_args!=2) printf("Shell: Incorrect command\n");
		else if(!chdir(tokens[1])){
			child = fork();
			if(child == 0) {
				all_children[top_index] = getpid();
				top_index++;
				memset(cwd,'\0',sizeof(cwd));
				getcwd(cwd,sizeof(cwd));
			} 
		} 
		else printf("Directory doesn't exist\n");
			
	} 
	else if(!strcmp(tokens[0],"exit")) {
		for(int i=0;i<top_index;i++) kill(all_children[i],SIGINT);
		printf("Shell: Goodbye.\n");
		exit(0);
	}
	else {
		child = fork();
		if(child == 0) {
			all_children[top_index] = getpid();
			top_index++;
			args[0] = "/bin/"; args[0] = concat(args[0],tokens[0]); args[num_of_args] = NULL;
			if(execv(args[0],args) == -1)
			{
				printf("Shell: Incorrect command\n");
				exit(0);
			};
			
		} 
	}

	if (type == 0 && child != 0) wait(NULL); 
	else if (type == 1 && child != 0) return;
	else if (type == 2 && child != 0) wait(NULL);
	else if (type == 3 && child != 0) return;
}


char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens; 

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	signal(SIGINT, mainHandler);
	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';

		} else { // interactive mode
			printf("$ ");
			bzero(line, MAX_INPUT_SIZE);
	        gets(line);
		}

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   
       	//do whatever you want with the commands, here we just print them
       	int type = 0;
       	for (int i=0;tokens[i]!=NULL;i++){
       		if(!strcmp(tokens[i],"&")) type = 1;
       		else if(!strcmp(tokens[i],"&&")) type = 2;
       		else if(!strcmp(tokens[i],"&&&")) type = 3;
       	}

       	for (int i=0;tokens[i]!=NULL;i++){
       		if(i==0) execute(tokens,type);
       		else if(!strcmp(tokens[i],"&") || !strcmp(tokens[i],"&&") || !strcmp(tokens[i],"&&&")){
       			if(tokens[i+1]!=NULL) execute(&tokens[i+1],type);
       		}
       	}

       	if (type == 1) sleep(1);
       	if (type == 3){
	       	for (int i=0;tokens[i]!=NULL;i++){
	       		if(i==0) wait(NULL);
	       		else if(!strcmp(tokens[i],"&&&")) wait(NULL);
	       	}
	    }


		// Freeing the allocated memory	
	for(int i=0;tokens[i]!=NULL;i++) free(tokens[i]);
		free(tokens);
	}
	

	return 0;
}	
