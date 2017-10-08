#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

void create_command(char*[], char*[], int start, int end);
void exec_output(char*[], char*);
void exec_input(char*[], char*);
void exec_pipe(char*[], char*[]);
int nextDelimPos(char*[],int, int);
bool isDelim(char*);


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
                // Build the command before the '>'
                char* built[MAX_LINE_WORDS+1];
                create_command(built, line_words, 0, i);
                // Execute output redirection
                exec_output(built, line_words[i+1]);
                break;
            }
            else if (strcmp(line_words[i], "<") == 0)
            {
                // Build the command before the '<'
                char* built[MAX_LINE_WORDS+1];
                create_command(built, line_words, 0, i);
                // Execute the input redirection
                exec_input(built, line_words[i+1]);
                break;
            }
            else if (strcmp(line_words[i], "|") == 0)
            {
                // Create arrays for both pipe commands
                char* built[MAX_LINE_WORDS+1];
                char* built2[MAX_LINE_WORDS+1];
                // Create command on left side of |
                create_command(built, line_words, 0, i);
                // Find the next delimiter (<,>,|)
                int next = nextDelimPos(line_words, num_words, i+1);  
                // Create the command on right side of |
                create_command(built2, line_words, i+1, next);
                // Execute pipes with both commands
                exec_pipe(built, built2);
                break;
            }
            else if (i == num_words - 1 && fork() == 0)
            {
                // Build the command
                char* built[MAX_LINE_WORDS+1];
                create_command(built, line_words,0,num_words);
                // Execute and single command, i.e ls, ls -al, who, etc...
                execvp(built[0], built);
                break;
            }
        }
    }

    return 0;
}

// Checks if a string is a delimeter
bool isDelim(char* c)
{
        if (strcmp(c, ">") == 0) return true;
        if (strcmp(c, "<") == 0) return true;
        if (strcmp(c, "|") == 0) return true;
        return false;
}

// Find the next delimeter position given the starting index
int nextDelimPos(char* words[], int num_words, int start)
{
    int temp = start;
    while (temp < num_words && !isDelim(words[temp]))
    {
        temp++;
    }  
    return temp;
}

// Create commands array for execvp with the first param 'built' since it complains about returning
// pointers, example built = ["ls", "-al"]
void create_command(char* built[], char* words[], int start, int end)
{
    for(int i = 0; i < end-start; i++)
    {
        built[i] = words[start+i];
    }
}

// Execute single output redirection
void exec_output(char* command[], char* filename)
{
    if (fork() == 0)
    {
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        //stdout
        dup2(fd, 1);
        close(fd);
        execvp(command[0], command); 
    }
}

// Execute single input redirection
void exec_input(char* command[], char* input)
{
    if (fork() == 0)
    {
        int fd = open(input, O_RDONLY);
        dup2(fd,0);
        close(fd);
        execvp(command[0], command);
    }
}

// Execute single pipe
void exec_pipe(char* writer[], char* reader[])
{
    int pfd[2];
    pipe(pfd);
    if (fork() == 0)
    {
        dup2(pfd[0], 0);
        close(pfd[0]);
        close(pfd[1]);
        execvp(reader[0], reader);
    }
    if (fork() == 0)
    {
        dup2(pfd[1], 1);
        close(pfd[0]);
        close(pfd[1]);
        execvp(writer[0], writer);
    }
    close(pfd[0]);
    close(pfd[1]);
    wait(NULL);
}
