
/****************
LE2: Basic Shell
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h> // wait
#include "Tokenizer.h"
#include <iostream>
#include <string>
using namespace std;

int main () 
{
    // lists all the files in the root directory in the long format
    // char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    // char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // Save original stdin and stdout
    int stdin = dup(0);         // copies stdin to 0 as that is the "next available"
    int stdout = dup(1);
    string input;

    while(true)
    {
        cout << "Provide commands: ";
        getline(cin, input);

        if(input == "exit")
            break;

        Tokenizer token(input);

        for(size_t i = 0 ; i < token.commands.size() ; i++)
        {
            // TODO: add functionality
            int filedes[2];         // filedes[0] will read from pipe, filedes[1] will write to pipe
            
            // Create pipe
            pipe(filedes);

            int pid1 = fork();
            // int pid2 = fork();

            // Create child to run first command
            if(!pid1)           // first child process
            {
                // In child, redirect output to write end of pipe
                if(i < token.commands.size() - 1)
                {
                    dup2(filedes[1], 1);  // give dup an input, and it will assign to next available index - it will put the value in input index into the next available index
                }
                
                // Close the read end of the pipe on the child side.
                close(filedes[0]);

                // In child, execute the command
                const char** args = new const char*[token.commands[i]->args.size()];

                size_t j = 0;
                while(j < token.commands[i]->args.size())
                {
                    args[j] = token.commands[i]->args[j].c_str();
                    j++;
                }

                execvp(args[0], (char**) args);
            }

            else
            {
                dup2(filedes[0], 0);
                close(filedes[1]);

                if(i == token.commands.size()-1)
                    wait(0);
            }

        }
        // Reset the input and output file descriptors of the parent.
        // Overwrite in/out with what was saved
        dup2(stdin, 0);
        dup2(stdout, 1);
    }

    close(stdin);
    close(stdout);
}