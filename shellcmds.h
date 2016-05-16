/*
 * Shellcmds.h
 *
 * Header file for internal shell commands
 */

#ifndef __SHELLCMDS_H_
#define __SHELLCMDS_H_

#define MAX_LENGTH 256

/* structure to hold history of commands */
struct command_list{

   int command_number;
   char *command;
   struct command_list *next;
};

/* structure to hold directory list */
struct directory_list{
  
   char *directory;
   struct directory_list *next;
};


typedef struct directory_list dir_list_t; 
typedef struct command_list cmd_list_t;


/* function to add a command to a list */
void addcommand(char *);

/* function to retrieve ith command from the list */
int getcommand(int, char *);

/* function to print history of commands */
void history(void);

/* function to print working directory */
void pwd(void);

/* function to change directories */
int cd(char *);

/* function to push current directory onto a stack */
void pushd(char *);

/* function to pop the current directory from the stack and cd to it */
void popd();

/* function to print the directory stack */
void dirs(void);

#endif
