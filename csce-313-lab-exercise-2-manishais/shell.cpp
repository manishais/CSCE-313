/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // Save original stdin and stdout
    int stdin = dup(0);         // copies stdin to 0 as that is the "next available"
    int stdout = dup(1);

    // TODO: add functionality
    int filedes[2];         // filedes[0] will read from pipe, filedes[1] will write to pipe
    // Create pipe
    pipe(filedes);
    int pid1 = fork();
    int pid2 = fork();

    // Create child to run first command
    if(!pid1)           // first child process
    {
        // In child, redirect output to write end of pipe
        dup2(filedes[1], 1);  // give dup an input, and it will assign to next available index - it will put the value in input index into the next available index
        // Close the read end of the pipe on the child side.
        close(filedes[0]);
        // In child, execute the command
        execvp(cmd1[0], cmd1);
        // Create another child to run second command
    }
    
    else if(!pid2)          // second child process
    {
         // In child, redirect input to the read end of the pipe
        dup2(filedes[0], 0); // dup2(a, b) will copy value of index b into index a
        // Close the write end of the pipe on the child side.
        close(filedes[1]);
        // Execute the second command.
        execvp(cmd2[0], cmd2);
    }
    
    else                // parent process
    {
        // Reset the input and output file descriptors of the parent.
        // Overwrite in/out with what was saved
        dup2(stdin, 0);
        dup2(stdout, 1);
    }
    
}
