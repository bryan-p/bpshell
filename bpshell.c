/*
 * This program mimics a shell.  It forks a child process to run commands
 * passed to it from the command line.  It also has its own built in commands
 * for changing directories, cd(), and printing the current directory, pwd().
 *
 * The program maintains a history of commands using a linked list structure. 
 */

#include "shellcmds.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_CHILDREN  2000

pid_t children[MAX_CHILDREN];

void run(void);

/* 
 * This function is the  entry point into 
 * the execution of the shell 
 */
void run(){
   
   char delim[3] = {' ', '\t', '\n'}; /* delimeters for calls to strtok() */
   char *token = NULL; /* used to hold the command */
   int  bang_num = 0; /* used to hold the number for bang command (i.e. !20) */
   char **argument = NULL; /* holds the arguments to the non built-in commands */
   char *comline = NULL; /* holds the entire command line */
   char *saved_command = NULL; /* used to save comline as it gets modified by the strtok command */
   pid_t pid = 0; /* holds return value of fork() */
   int index = 0; /* used as array index for argument string */
   int running_children = 0; /* number of currently executing children */

   /* keep looping until user types 'exit' or forces quit */
   while(1){   

      /* allocate space for command line input
       * if space is not available exit program     
       ******************************************************** */
      if( (comline = calloc(MAX_LENGTH, sizeof(char))) == NULL){
         fprintf(stderr,"Could not allocate memory\n");
         exit(1);
      }

      if( (argument = calloc(MAX_LENGTH, sizeof(char))) == NULL){
         fprintf(stderr,"Could not allocate memory\n"); 
         exit(1);
      }
 
      if( (saved_command = calloc(MAX_LENGTH, sizeof(char))) == NULL){
         fprintf(stderr,"Could not allocate memory\n");
         exit(1);
      }
      /* ********************************************************** */

      /* read in data from the command line 
       * keep going until something is entered  */
      do{
         fprintf(stdout, "~ ");
         fgets(comline,MAX_LENGTH,stdin);
      }while(strncmp(comline, "\n", 1) == 0);

      /* saved the command line since strtok updates comline string */
      strncpy(saved_command, comline, strlen(comline));

      /* get the command to execute */
      token = strtok(comline, delim);
      /* check for bang-based command recall */

      while(strncmp(comline, "!", 1) == 0){
      
         /* first get the number of the command */
         bang_num = atoi(++token);

         /* check to see that its a valid command number 
          * if not get a new command */
         if( (getcommand(bang_num, comline)) == -1){
            fprintf(stderr,"Command %d not in list\n", bang_num);   

            /* reset comline */
            /* ********************************** */
            free(comline);
            comline = NULL;

            if( (comline = calloc(MAX_LENGTH, sizeof(char))) == NULL){
               fprintf(stderr,"Could not allocate memory");
               exit(1);
            }
            /* ********************************** */

            /* get a new command */
            fprintf(stdout, "~ ");
            fgets(comline,MAX_LENGTH,stdin);

            token = strtok(comline, delim); 
         }
       
         else
            token = strtok(comline, delim); 
      }   

      /* check for built in commands */
      /* *************************************** */
    
      if(strncmp(token, "cd", 2) == 0){
     
         /* add the command to the history list */
         addcommand(saved_command);

         /* change to the given directory if valid */ 
         if( cd(strtok(NULL, delim)) == -1)
            perror("");
      }

      else if(strncmp(token, "pwd", 3) == 0){
 
         /* print working directory */
         addcommand(saved_command);
         pwd();
      }

      else if(strncmp(token, "history", 7) == 0){

         /* print history of commands including this one */
         history();
         addcommand(saved_command); 
      }

      else if(strncmp(token, "pushd", 5) == 0){
      
         /* push directory onto stack and chdir to it */
         addcommand(saved_command);
         pushd(strtok(NULL, delim));
      }  

      else if(strncmp(token, "popd", 4) == 0){

         /* pop directory from end of stack and chdir to it */
         addcommand(saved_command);
         popd();
      }

      else if(strncmp(token, "dirs", 4) == 0){
       
         /* print all directories on the stack */
         addcommand(saved_command);
         dirs();
      }

      /* exit program */   
      else if(strncmp(token, "exit",4) == 0)
         break;

      /* ********************************* */

      /* not a built in command so we want to fork a child and try to execute the command */
      else{
              
         index = 0;

         /* add command to the list */
         addcommand(saved_command); 

         /* this loop sets the arguments for the called program */
         while( (token != NULL) ){ 
          
            argument[index] = token;
            index++;
         
            token = strtok(NULL, delim);   
         }
        
         pid = fork(); /* fork the new child process */
          
         /* this is the child process */
         if(pid == 0){
            if ( (execvp(argument[0], argument)) == -1){
               perror("Error: ");
               exit(1);
            }
         }
            
         else if(pid < 0)
            fprintf(stderr,"Could not fork");
         
         else{

            /* no more than MAX_CHILDREN can run at once */
            if(++running_children > MAX_CHILDREN)
               kill(pid, SIGKILL);
            
            else
               waitpid(pid, NULL, 0); /* wait for the child process to return */  
            
            /* either child is killed or has returned */
            --running_children;
         }
      }
      
      /* free memory for comline and argument and avoid double free error */
      free(comline);
      free(argument);
      free(saved_command);
      comline = NULL;
      argument = NULL;
      saved_command = NULL;
   }

   /* free memory for comline and argument and avoid double free error */
   free(comline);
   free(argument);
   free(saved_command);
   comline = NULL;
   argument = NULL;
   saved_command = NULL;
}
   
int main(int argc, char* argv[]){

   run();

   return 0;  
} 
