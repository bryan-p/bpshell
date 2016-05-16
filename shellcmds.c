/*
 * shellcmds.c
 *
 * This file contains built in functions for shll
 */

#include "shellcmds.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* structs for the head and tail of a linked list of commands */
static cmd_list_t *comlisthead = NULL;
static cmd_list_t *comlisttail = NULL;

/* structs for the head and tail of a linked list of directories */
static dir_list_t *dirlisthead = NULL;
static dir_list_t *dirlisttail = NULL;

/*
 * This function adds a command to the end of a list, 
 * or in this case the top of the stack.
 */
void addcommand(char *cmd){

   cmd_list_t *current; /* the node to be added to the list */
   static int index = 0;

   /* no need to do anything without a command to save */
   if(cmd == NULL)
      return;

   /* allocate space for the new node in the list */
   if( (current = malloc(sizeof(cmd_list_t))) == NULL){
      fprintf(stderr,"Could not allocate memory for command");
      perror(" "); 
   }

   /* set the struct variables for the new node */
   current->command_number = ++index;
   current->command = malloc(strlen(cmd) + 1);
   strncpy(current->command, cmd, (strlen(cmd) + 1));
   current->next = NULL;
   
   /* check to see if list is empty, if so this is first element */
   if(comlisthead == NULL)
      comlisthead = current;

   /* otherwise add current element to end of list */
   else
      comlisttail->next = current;
   
   /* set the tail to point to the end of the list */
   comlisttail = current;
}

/*
 * This function gets the ith command from the list
 * It simply traverses the list from the beginning until the ith
 * command is found and copies value of that command.
 *
 * Returns 0 on success, -1 if cmd_num does not exist
 */
int getcommand(int cmd_num, char *cmd){

   cmd_list_t *current;
   int error_code = 0; 

   /* start from beginning of list */
   current = comlisthead;

   /* loop until command number is found */
   while( (current != NULL) && (current->command_number != cmd_num) ){
      current = current->next;
   }

   /* make sure the command exists as the loop could terminate on NULL condition */
   if( (current == NULL) || (current->command_number != cmd_num) )
      error_code = -1;

   else
      strncpy(cmd, current->command, strlen(current->command));

   return error_code;
} 

/*
 * This function prints the history of commands from the list
 */
void history(){

   cmd_list_t *current; 

   /* start from beginning of list */
   current = comlisthead;

   /* keep going until end of list */
   while( current != NULL){
      fprintf(stdout,"%s", current->command);
      current = current->next;
   }
}

/*
 * This function pushes the directory path onto the stack
 * and then cd()s to the directory
 */
void pushd(char *target){

   dir_list_t *current = NULL; /* the node to be added to the list */

   /* no need to do anything without a directory to push on the stack */
   if(target == NULL)
      return;

   fprintf(stdout,"%s\n", target);
  
   /* change to the directory */
   if( cd(target) == -1){
      fprintf(stderr,"Could not change to %s directory", target);
      perror("");
      return;
   }   
   /* allocate memory */
   if( (current = malloc(sizeof(dir_list_t))) == NULL){
      fprintf(stderr,"Could not allocate memory");
      perror(" "); 
   }

   if( (current->directory = malloc(strlen(target) + 1)) == NULL){
      fprintf(stderr,"Could not allocate memory");
      perror(" "); 
   }

   /* set the struct value for the new directory */
   strncpy(current->directory, target, (strlen(target) + 1));
   current->next = NULL;
   
   /* check to see if list is empty, if so this is first element */
   if(dirlisthead == NULL)
      dirlisthead = current;

   /* otherwise add current element to end of list */
   else
      dirlisttail->next = current;
   
   /* set the tail to point to the end of the list */
   dirlisttail = current;
}

/*
 * This function pops the the last node from the list and 
 * changes to the directory stored in that node 
 */
void popd(){

   dir_list_t *current = NULL; /* used to set new end of list */ 
   char *tmpdir = NULL; /* hold the directory to change to */

   /* don't do anything if list is empty */
   if(dirlisthead == NULL)
      return;
   
   /* allocate space for tmpdir */
   if( (tmpdir = malloc(strlen(dirlisttail->directory) + 1)) == NULL){
      fprintf(stderr,"Could not allocate memory");
      perror(" "); 
   }

   /* since we know list has at least one node get the directory stored in the
    * last node */
   strncpy(tmpdir, dirlisttail->directory, strlen(dirlisttail->directory) + 1); 

   /* special case of one element in the list, in this case loop below
    * will cause a segfault so we don't want to do that */  
   if(dirlisthead->next == NULL){

      /* clear the list */
      dirlisthead = NULL;
   }
   
   else{
      /* start from beginning of list */
      current = dirlisthead;

      /* keep going until we find 2nd to last node of the list 
       * this is so the last node can be popped */
      while( current->next->next != NULL){
         current = current->next;
      }

      /* print out directory so user knows where he will be in the filesystem */
      fprintf(stdout,"%s\n", current->next->directory);

      /* remove reference to last node on list */
      current->next = NULL;
   }

   /* point tail to new end of the list */
   dirlisttail = current;

   /* change to the directory. Need to check that we still can */
   if( (cd(tmpdir)) == -1){
      fprintf(stderr,"Could not change to %s directory", current->next->directory);
      perror("");
      return;
   }   
}
     
/*
 * This function prints all directories on the directory stack
 */
void dirs(){

   dir_list_t *current = NULL;

   /* start from beginning of list */
   current = dirlisthead;

   /* keep going until end of list */
   while( current != NULL){
      fprintf(stdout,"%s\n", current->directory);
      current = current->next;
   }
}

/* 
 * This function prints out the current working directory
 */
void pwd(){

   char *dir = NULL;
  
   if( (dir = malloc(MAX_LENGTH)) == NULL){
      fprintf(stderr,"Could not allocate memory\n");
      return;
   }

   if ( (dir = getcwd(dir, MAX_LENGTH)) == NULL){
      fprintf(stderr,"Could not access path:");
      perror(" ");
   }

   fprintf(stdout,"%s\n", dir);
}


/*
 * This function changes to the target directory if successful.
 * Returns 0 on success and -1 on error 
 */
int cd(char *target){

   static char *pwd = NULL; /* previous working directory */
   char *tmpbuf = NULL; /* temporary buffer to hold value of previous working directory */
   int error_code = 0; 

   if( (tmpbuf = calloc(MAX_LENGTH, sizeof(char))) == NULL){
      fprintf(stderr, "Could not allocate memory\n");
      return -1;
   }

   /* check for special case of '-', which changes to previous working directory */
   if(strncmp(target, "-", 1) != 0){

      /* get value of current working directory and point pwd to it
       * this is to be used as the previous working directory */  
      tmpbuf = getcwd(tmpbuf, MAX_LENGTH);
      pwd = tmpbuf;
   }

   else 
      target = pwd;
      

   if( (chdir(target)) == -1){
      error_code = -1;
   }

   return error_code;
}

