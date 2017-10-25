#include <stdio.h>
#include "constants.h"
#include "parsetools.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define STDINPUT 0
#define STDOUTPUT 1
#define NUM(a) (sizeof(a) / sizeof(a[0]))
#define ERROR -1

void print(char ** arr);
void exec_output(char * output);
void exec_input(char * input);
int exec_command(char ** cmd, int num_words);
int input, output;

void syserror(const char * s)
{
    extern int errno;

    fprintf(stderr, "%s\n", s);
    fprintf(stderr, "(%s)\n", strerror(errno));
    exit(1);
}

int main() {
    // Buffer for reading one line of input
    char line[MAX_LINE_CHARS];
    char* line_words[MAX_LINE_WORDS + 1];
    int num_words = 0;
    
    // Loop until user hits Ctrl-D (end of input)
    // or some other input error occurs
    while( fgets(line, MAX_LINE_CHARS, stdin) ) {
        num_words = split_cmd_line(line, line_words);
        int cmdidx = 0;
        input = 0;
        output = 1;
        char* command[num_words];
        int stdincopy;
        int stdoutcopy;
        stdincopy = dup(STDINPUT);
        stdoutcopy = dup(STDOUTPUT);
        if (stdincopy == ERROR || stdoutcopy == ERROR)
            syserror("Error copying stdin or stdout\n");
        for (int i = 0; i < num_words; i++) {
	        if ((strcmp(line_words[i], ">")) == 0) {
                exec_output(line_words[i+1]);
                i++;
            }
            else if ((strcmp(line_words[i], "<")) == 0) {
                stdincopy = dup(STDINPUT);
                if (stdincopy == ERROR)
                    syserror("Error copying stdin\n");
                exec_input(line_words[i+1]);
                i++;
            }
            else if ((strcmp(line_words[i], "|")) == 0) {
                stdincopy = dup(STDINPUT);
                stdoutcopy = dup(STDOUTPUT);
                if (stdincopy == ERROR || stdoutcopy == ERROR)
                    syserror("Error copying stdin or stdout\n");
                int pfd[2];
                if (pipe(pfd) == ERROR)
                    syserror("There was an error piping\n");

                output = pfd[1];
                exec_command(command, cmdidx);
                cmdidx = 0;
                if (close(pfd[1]) == ERROR)
                    syserror("Error closing pfd[1]\n");
                dup2(stdoutcopy, STDOUTPUT);
                output = 1;

                input = pfd[0];
            }
            else {
                command[cmdidx++] = line_words[i];
            }
        }
        exec_command(command, cmdidx);
        if (input != 0)
            dup2(stdincopy, STDINPUT);
        if (output != 1)
            dup2(stdoutcopy, STDOUTPUT);
        if (close(stdoutcopy) == ERROR)
            syserror("Error closing stdoutcopy\n");
        if (close(stdincopy) == ERROR)
            syserror("Error closing stdincopy\n");
    }
    wait(NULL);
    if (close(STDINPUT) == ERROR)
        syserror("Error closing stdinput\n");
    if (close(STDOUTPUT) == ERROR)
        syserror("Error closing stdoutput\n");
    return 0;
}

void exec_output(char * filename) {
    output = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (output == ERROR)
        syserror("Error opening file for output\n");
}

void exec_input(char * _input) {
    input = open(_input, O_RDONLY);
    if (input == ERROR)
        syserror("Error opening file for input\n");
}

int exec_command(char ** cmd, int num_words) {
  int pid = fork();
  if (pid < 0)
        syserror("Error creating fork\n");
  cmd[num_words] = (char *) NULL;
    if (pid == 0) {
        if (input != 0) {
            dup2(input, 0);
            if (close(input) == ERROR)
                syserror("Error closing input\n");
        }
        if (output != 1) {
            dup2(output, 1);
            if (close(output) == ERROR)
                syserror("Error closing output\n");
        }
        if (num_words > 1) {
	        execvp(cmd[0], cmd);
        }
        else {
            execlp(cmd[0], cmd[0], (char *)NULL);
        }
    }
    
    return pid;
}

void print(char ** arr) {
//    printf("Printing array\n");
    for (int i = 0; i < NUM(arr); i++) {
        printf("%s ", arr[i]);
    }
    printf("\n");
}
