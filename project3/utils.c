#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int int_cmp(const void *a, const void*b){
    return ( *(int*)a - *(int*)b );
}

struct server_command create_command(char *mycmd, int length){
    int mychar = *mycmd;
    char cmd_name[length];
    char args[length];
    int cmd_name_index = 0;
    int args_index = 0;
    bool cmd_name_found = FALSE;
    int number_of_args = 0;
    int argv_index = 0;
    char argv_buffer[length];
    int argv_buffer_index = 0;
    struct server_command scmd;
    while(mychar){
        if(!cmd_name_found && mychar != ' '){
            cmd_name[cmd_name_index++] = mychar;
            mycmd++;
            mychar = *mycmd;
        }
        else if(!cmd_name_found && mychar == ' '){
            /* Found end of cmd name. Consume whitespace. */
            cmd_name[cmd_name_index++] = '\0';
            cmd_name_found = TRUE;
            while((mychar = *mycmd++) == ' ') ;
            if(mychar != '\0') /* End of string */
                number_of_args++;
        }
        else if(mychar == ' '){
            args[args_index++] = mychar;
            /* Consume whitespace. */
            while((mychar = *mycmd++) == ' ') ;
            if(mychar != '\0') /* End of string */
                number_of_args++;
        }
        else{
            args[args_index++] = mychar;
            mychar = *mycmd++;
        }
    }
    scmd.argv = malloc(sizeof(char *) * number_of_args);
    for(int i = 0; i < args_index; i++){
        if(args[i] != ' '){
            argv_buffer[argv_buffer_index++] = args[i];
        }
        if(i == args_index - 1 || args[i] == ' '){
            scmd.argv[argv_index] = malloc(sizeof(char) * (argv_buffer_index + 1));
            strncpy(scmd.argv[argv_index], argv_buffer, argv_buffer_index);
            scmd.argv[argv_index][argv_buffer_index] = '\0';
            argv_buffer_index = 0;
            argv_index++;
        }
    }
    
    scmd.argc = number_of_args;
    scmd.cmd = malloc(sizeof(char) * cmd_name_index);
    strncpy(scmd.cmd, cmd_name, cmd_name_index);
    return scmd;
}

void delete_command(struct server_command scmd){
    free(scmd.cmd);
    for(int i = 0; i < scmd.argc; i++){
        free(scmd.argv[i]);
    }
    free(scmd.argv);
}

bool is_number(char *str){
    int size = strlen(str);
    for(int i = 0; i < size; i++){
        if(!isdigit(str[i]))
            return FALSE;
    }
    return TRUE;
}

bool safe_read(int fd, char *buffer, int length){
    int bytesRead;
    int nRead;
    bytesRead = 0;
    nRead = 0;
    
    while(bytesRead < length){
        nRead = read(fd, buffer, length);
        if(nRead == 0)
            return FALSE;
        if(nRead == -1){
            perror("Socket reading failed. ");
            return FALSE;
        }
        bytesRead += nRead;
    }
    return TRUE;
}
