#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

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
                break;
            }
            else if (strcmp(line_words[i], "<") == 0)
            {
                execute_command(line_words, argv, '<', i);
                break;
            }
            else if (num_words == 1 && fork() == 0)
            {
                execlp(line_words[0], line_words[0], (char*)NULL);
                break;
            }
            else if (i == num_words - 1 && num_words > 1)
            {
                argv[i] =  line_words[i];
                execute_command(line_words, argv, '\0', i);
                break;
            }
            argv[i] =  line_words[i];
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
        
        /*printf("Filename = %s\n", filename);
        printf("Position = %s\n", line[position]);
        printf("Args = %s\n", arguments[0]);
        printf("Args = %s\n", arguments[1]);
        printf("Args = %s\n", arguments[2]);
        *///Begin reirection
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        //stdout
        dup2(fd, 1);
        close(fd);
        if (position > 1)
            execvp(arguments[0], arguments); 
        else
            execlp(line[0], line[0], (char*)NULL);
    }
    // wc -l < test.txt
    else if (operator == '<' && fork() == 0)
    {
        
        char* filename = line[position+1];

        /*printf("Filename = %s\n", filename);
        printf("Position = %s\n", line[position]);
        printf("Args = %s\n", arguments[0]);
        printf("Args = %s\n", arguments[1]);
        printf("Args = %s\n", arguments[2]);
        */
        int fd = open(filename, O_RDONLY);
        // stdin
        dup2(fd, 0);
        close(fd);
        if (position > 1)
        {
            execvp(arguments[0], arguments);
        }
        else
        {
            execlp(line[0], line[0], (char*)NULL);
        }
    }
    // exmaple ls -al
    else if (operator == '\0' && fork() == 0)
    {
        /*printf("Position = %s\n", line[position]);
        printf("Args = %s\n", arguments[0]);
        printf("Args = %s\n", arguments[1]);
        printf("Args = %s\n", arguments[2]);
        */
        execvp(line[0], line);
    }
}
