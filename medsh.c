#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 64 //maximum number of command tokens
#define MAX_ALIASES 50 //maximum number of alias definitions

typedef struct {
    char name[256]; //alias name
    char command[256]; //command to execute
} alias_t;

alias_t aliases[MAX_ALIASES]; //array for storing aliases
int alias_count = 0; //count of loaded aliases
char current_user[256] = ""; //holds the current user name

//load alias definitions from ~/.profile_medsh file
//expected line format: alias aliasname='command'
void load_profile(){
    char *home = getenv("HOME"); //get home directory
    if (!home) return;
    char path[512];
    snprintf(path, sizeof(path), "%s/.profile_medsh", home); //build file path
    FILE *fp = fopen(path, "r"); //open the profile file
    if(!fp) return;
    char line[256];
    while(fgets(line, sizeof(line), fp)){ //read file line by line
        line[strcspn(line, "\n")] = '\0'; // remove newline character if present
        if (strncmp(line, "alias", 5) == 0){ //check if line starts with "alias"
            char *p = line + 5; //skip the word "alias"
            while (*p && isspace(*p)) p++; //skip any whitespace
            char *eq = strchr(p, '='); //find '=' separator
            if(eq){
                *eq = '\0'; //terminate alias name string
                char *alias_name = p; //isolated alias name
                //remove trailing whitespace from alias_name
                char *end = alias_name + strlen(alias_name) - 1;
                while (end >= alias_name && isspace(*end)) { 
                    *end = '\0';
                    end--;
                }
                char *alias_command = eq + 1; //command part starts after '='
                //remove surrounding quotes if present
                if (*alias_command == '\'' || *alias_command == '"') {
                    char quote = *alias_command; //save quote character
                    alias_command++; //skip opening quote
                    char *quote_end = strchr(alias_command, quote);
                    if (quote_end)
                        *quote_end = '\0'; //terminate at closing quote
                }
                //copy alias name and command into global array
                strncpy(aliases[alias_count].name, alias_name, sizeof(aliases[alias_count].name)-1);
                aliases[alias_count].name[sizeof(aliases[alias_count].name)-1] = '\0';
                strncpy(aliases[alias_count].command, alias_command, sizeof(aliases[alias_count].command)-1);
                aliases[alias_count].command[sizeof(aliases[alias_count].command)-1] = '\0';
                alias_count++; //increment alias count
                if (alias_count >= MAX_ALIASES)
                    break; //stop if maximum reached
            }
        }
    }
    fclose(fp); //close the profile file
}

//split the input string into tokens (arguments)
//set *background to 1 if the last token is "&"
void parse_command(char *input, char **args, int *background){
    int i = 0;
    char *token = strtok(input, " \t"); //tokenize using space or tab
    *background = 0; //default: foreground execution
    while(token && i < MAX_ARGS - 1){
        args[i++] = token; //save each token
        token = strtok(NULL, " \t");
    }
    args[i] = NULL; //mark end of tokens
    if(i > 0 && strcmp(args[i - 1], "&") == 0){ //check for background symbol
        *background = 1; //set background flag
        args[i - 1] = NULL; //remove "&" from arguments
    }
}

int main(void){
    char *input; //user input string
    char *args[MAX_ARGS]; //array of tokenized input arguments
    char *exec_args[MAX_ARGS]; //final command arguments (after alias expansion)
    int background; //flag for background execution
    char prompt[256]; //buffer for the command prompt

    load_profile(); //load aliases from profile file
    stifle_history(10); //limit command history to 10 entries

    while (1){
        //build prompt; include username if set
        if(strlen(current_user) > 0)
            snprintf(prompt, sizeof(prompt), "medsh(%s)> ", current_user);
        else
            snprintf(prompt, sizeof(prompt), "medsh> ");

        input = readline(prompt); //read input from user
        if(!input){ //exit on EOF (e.g., Ctrl+D)
            printf("\n");
            break;
        }
        if(strlen(input) > 0)
            add_history(input); //add non-empty input to history

        //handle built-in command: setusr to set the user name
        if(strncmp(input, "setusr ", 7) == 0){
            strncpy(current_user, input + 7, sizeof(current_user)-1);
            current_user[sizeof(current_user)-1] = '\0';
            free(input);
            continue;
        }
        //handle built-in command: unsetusr to clear the user name
        if(strcmp(input, "unsetusr") == 0){
            current_user[0] = '\0';
            free(input);
            continue;
        }
        //handle built-in command: exit to terminate the shell
        if(strcmp(input, "exit") == 0){
            free(input);
            break;
        }
        //handle built-in command: history to display command history
        if(strcmp(input, "history") == 0){
            extern int history_base, history_length;
            for(int i = history_base; i < history_base + history_length; i++){
                HIST_ENTRY *entry = history_get(i);
                if(entry)
                    printf("%d: %s\n", i, entry->line);
            }
            free(input);
            continue;
        }

        //tokenize the input command and check for background flag
        parse_command(input, args, &background);
        if(!args[0]){  //if no command entered, continue loop
            free(input);
            continue;
        }

        //check if the first token matches an alias. if so, expand it
        int alias_found = 0;
        char *alias_copy = NULL;
        for(int i = 0; i < alias_count; i++){
            if(strcmp(args[0], aliases[i].name) == 0) {
                alias_found = 1;
                alias_copy = strdup(aliases[i].command); //duplicate alias command
                break;
            }
        }
        if(alias_found){
            int j = 0;
            char *token = strtok(alias_copy, " \t"); //tokenize the alias command
            while(token && j < MAX_ARGS - 1){
                exec_args[j++] = token;
                token = strtok(NULL, " \t");
            }
            //append extra arguments from the original command, if any
            for (int k = 1; args[k] && j < MAX_ARGS - 1; k++)
                exec_args[j++] = args[k];
            exec_args[j] = NULL; //terminate the exec_args array
        } 
        else{
            //no alias match; use the original tokens
            for (int i = 0; i < MAX_ARGS; i++) {
                exec_args[i] = args[i];
                if (!args[i])
                    break;
            }
        }

        //fork a child process to execute the command
        pid_t pid = fork();
        if(pid < 0){
            perror("fork failed");
        } 
        else if(pid == 0){ //child process executes the command
            if(execvp(exec_args[0], exec_args) < 0){
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        } 
        else{ //parent process
            if(!background)
                waitpid(pid, NULL, 0); //wait for child if foreground
            else
                printf("Background process PID: %d\n", pid);
        }
        if(alias_copy)
            free(alias_copy); //free the alias duplicate if used
        free(input); //free the input string
    }
    return 0;
}
