#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#include <cstdlib>

using namespace std;

void openr(int filedes) 
{
    char buffer[256];
    size_t bytesRead;

    while ((bytesRead = read(filedes, buffer, sizeof(buffer))) > 0)
    {
        write(STDOUT_FILENO, buffer, bytesRead);                // reading the file line by line and printing it
    }

    cout << endl;

    close(filedes);
}

int main() 
{
    string filename;
    int PATH_MAX = 256;
    size_t writeToPipe;

    cout << "ENTER FILE NAME: ";
    getline(cin, filename);

    int filed[2]; // pipe file descriptors
    int fd = open(filename.c_str(), O_RDONLY); // file descriptor for the file being opened
    pipe(filed); // creating a pipe for communication between parent and child

    pid_t pid = fork(); // child created

    if (pid == 0) // child process
    {
        // DO SYNCHRONIZATION STUFF
        close(filed[1]); // closing the writing end of the pipe
        char buffer[256]; // creating a message that needs to be read, with a max of 256 bytes
        size_t readFromPipe = read(filed[0], buffer, sizeof(buffer)); // reading a message from parent to child via the pipe

        if (readFromPipe > 0) 
        {
            cout << "Message has been read by child" << endl;

            cout << "Now, reading the given file:" << endl;

            cout << endl;

            openr(fd); // Read the file from the passed file descriptor

            // char buffer[PATH_MAX];
            // openr(getcwd(buffer, sizeof(buffer)));
        } 

        // else 
        // {
        //     cout << "Message not received" << endl;
        // }

        close(filed[0]);
        exit(0);
   } 
    
    
    else // parent process
    {
        if (unlink(filename.c_str()) == 0)                      // deleting the file using the unlink system call
        {
            cout << "File is deleted" << endl;

            close(filed[0]);                // closing the read end of the pipe in the parent
            char message[] = "deletion finished, child can continue";           // this message will be written to the pipe

            size_t writeToPipe = write(filed[1], message, strlen(message)); // message from parent to child via the pipe
        } 
        
        else 
        {
            cout << "file couldn't be deleted, terminating now" << endl;   // file couldn't be deleted  
            exit(0);           
            // char message[] = "";

            // size_t writeToPipe = write(filed[1], message, strlen(message)); // message from parent to child via the pipe
        }


        if (writeToPipe >= 0) 
        {
            cout << "message has been sent" << endl;
        } 
       

        close(filed[1]);                // close writing end of pipe, cannot wrte anymore

        int status;
        wait(&status);
    }
    return 0;
}


// Importance of inodes in Unix file system
// 1. Useful for creating the metadata of a file i.e. information about the file. This can be file size, file type, owner permissions etc.
// 2. They also help organize the structure of a file system by observing the relationships between files and directories
// 3. By keeping track of free blocks and those that are not, indoes can help allocate storage space