#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    // create copies of stdin/stdout;
    int stdin = dup(0);
    int stdout = dup(1);

    vector<pid_t> backgroundPid = vector<pid_t>();          // creating vector outside loop
    char path[250];

    for (;;) {
        // implement iteration over vector of bg pid (vector also declared outside loop)
        // waitpid - using flag for non-blocking


        // implement date/time with TODO
        time_t timee;
        // implement username with getenv()
        // implement curdir with getcwd()
        char curdir[250];
        // need date/time, username, and absolute path to current dir
        time(&timee);           // gets time in seconds
        char* readableTime = ctime(&timee);   // human readable string
        size_t length = strlen(readableTime);

        if (readableTime[length - 1] == '\n')           // replacing /n with a null character
        {
            readableTime[length - 1] = '\0';
        }
       // cout << YELLOW << "Shell$" << NC << " ";
       cout << YELLOW << readableTime << " " << getenv("USER") << ":" << getcwd(curdir, sizeof(curdir)) << "\n$" << NC << " ";
        
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        // print out every command token-by-token on individual lines
        // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
            for (auto str : cmd->args) {
                cerr << "|" << str << "| ";
            }
            if (cmd->hasInput()) {
                cerr << "in< " << cmd->in_file << " ";
            }
            if (cmd->hasOutput()) {
                cerr << "out> " << cmd->out_file << " ";
            }
            cerr << endl;
        }

        // implement cd with chdir()
        // if dir (cd <dir>) is "-", then go tp previous working directory
        // variable storing revious working directory (needs to be declared oustide loop)
        if(tknr.commands[0]->args[0] == "cd")
        {
            if(tknr.commands[0]->args[1] == "-")
            {
                chdir(path);
            }
            else
            {
                getcwd(path, sizeof(path));
                chdir(tknr.commands[0]->args[1].c_str());
            }
            continue;
        }


        // for piping
        // for cmd : commands
        //      call pipe() to make pipe
        //      fork() - in child, redirect stdout; in par, redirect stdin
        //      ^ is already written (yay!!!)
        // add checks for first/last command

        for(size_t i = 0 ; i < tknr.commands.size() ; i++)
        {
            int filedes[2];
            pipe(filedes);
            // fork to create child
            pid_t pid = fork();
            if (pid < 0) {  // error check
            perror("fork");
            exit(2);
            }

            // if current command is redirected then open file and dup2 stdin/stdout that's being redirected
            // implement it safely for both at same time
            if (pid == 0)                           // if child, exec to run command
            {
                if (tknr.commands[i]->hasInput())             // input redirection
                {         
					FILE* inFiledes = fopen(tknr.commands[i]->in_file.c_str(), "r");    // open file descriptor for reading
					dup2(inFiledes->_fileno, 0);            // redirecting stdin to input file
				}

				if (tknr.commands[i]->hasOutput())             // output redirection
				{
                	FILE* outFiledes = fopen(tknr.commands[i]->out_file.c_str(), "w");    // open file descriptor for writing to the output file
					dup2(outFiledes->_fileno, 1);       // redirecting stdout to output file
				}
                
                if (i < tknr.commands.size() - 1) {
					dup2(filedes[1], 1);                // redirect stdout to previosly opened file descriptor
				}               

                close(filedes[0]);

                // run single commands with no arguments
                // implement multiple arguments - iterate over args of current command to make 
                //                                char* array
                const char** args = new const char*[tknr.commands[i]->args.size()];
                size_t j = 0;
                while (j < tknr.commands[i]->args.size()) 
                {
                    args[j] = tknr.commands[i]->args[j].c_str();
                    j++;
                }
               

                if (execvp(args[0], (char**) args) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
            }
            else  // if parent, wait for child to finish
            {
                dup2(filedes[0], 0);
				close(filedes[1]);   

                int status = 0;
               
                if(tknr.commands[i]->isBackground())
                {
                    backgroundPid.push_back(pid);
                }
			
				else 
                {
                    waitpid(pid, &status, 0);
				}

				if (status > 1) {        // exit if child didn't exec properly
					exit(status);
				}
            }
       
        }

        // add check for bg process - as pid to vector if bg and don't waitpid() in parent
        pid_t check;
        size_t i = 0;
        while(i < backgroundPid.size())
        {
            int status = 0;             // stores exit status
			check = waitpid(backgroundPid[i], &status, WNOHANG);        // specifies non blocking behavior as said in video

			if (check > 0) 
            {
				backgroundPid.erase(backgroundPid.begin() + i);     // removing terminated child process
            }
            i++;        // check next process
        }

        
    }
}
