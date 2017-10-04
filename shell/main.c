#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

void execute_command(char*[],char*[], char, int);  

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        // Num words = the input
        int num_words = split_cmd_line(line, line_words);
        char* argv[MAX_LINE_WORDS];
        int pos = -1;

        // Just for demonstration purposes
        for (int i=0; i < num_words; i++)
        {
            // Testing only output redirection. Need to fix for adding pipes and nested redirection
            if (strcmp(line_words[i], ">") == 0)
            {
                execute_command(line_words, argv, '>', i);
                pos = i;
                break;
            }
            if (strcmp(line_words[i], "<") == 0)
            {
                execute_command(line_words, argv, '<', i);
                pos = i;
                break;
            }
            argv[i] =  line_words[i];
        }
        if (fork() == 0)
        {
            // Single command no args, 'ls'
            if (num_words == 1){
                execlp(line_words[0], line_words[0], (char*)NULL);
            }
            // Shit way of output redirection 'ls > hello.txt'
            else if (pos > 0)
            {
                // File after the '>'
                char* filename = line_words[pos+1];
                //Begin reirection
                int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                dup2(fd, 1);
                execvp(argv[0], argv); 
            }
            // Single command with args 'ls -al'
            execvp(line_words[0], line_words);
        }
        /* execvp for dynamic arguments
        char *argv[] = {"ls", "-l", "-t", 0};
        execvp(argv[0], argv);
        */
    }

    return 0;
}

// Might be an option, still testing
void execute_command(char* line[], char* arguments[], char operator, int position)
{
    if (operator == '>' && fork() == 0)
    {
        char* filename = line[position+1];
        //Begin reirection
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        //stdout
        dup2(fd, 1);
        execvp(arguments[0], arguments); 
    }
    else if (operator == '<' && fork() == 0)
    {
        char* filename = line[position+1];
        int fd = open(filename, O_RDONLY);
        // stdin
        dup2(fd, 0);
        execvp(arguments[0], arguments);
    }
}
