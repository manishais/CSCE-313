#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

void openr(const char *pathname) {
    int fd = open(pathname, O_RDONLY);

    char buffer[256];
    size_t line;

    while ((line = read(fd, buffer, sizeof(buffer))) > 0) 
    {
        write(STDOUT_FILENO, buffer, line);
    }

    cout << endl;
    close(fd);
}

int main() 
{
    // You don't need to duplicate stdin and stdout, and you don't need to assign them.
    
    openr("/home/manishais/Homework/q1test.txt");
   
    return 0;
}


// ASSEMBLY
// section .data
//         pathname db "/home/manishais/Homework/q1test.txt", 0
// section .bss
//         line resb 18
// section .text
//         global _start
// start:
//     openr: 
//         MOV rax, 2                  // returns file descriptor to rax ; system call number for open is 2
//         MOV rdi, pathname           // rdi = 1st function parameter ; pointer to the string (pathname)
//         MOV rsi, 0                  // rsi = 2nd function parameter ; readonly is 0
//         syscall
//     read:
//         MOV rdi, rax                // to read the opened file, the first argument has to be the file descriptor (held in rax)
//         MOV rax, 0                  // system call is now read, value returned to rax ; system call number for read is 0
//         MOV rsi, line               // line is the read text, pointer is stored in rsi
//         MOV rdx, 1
//         print line
//     closefile:
//         MOV rax, 3                  // instruction to close a file ; system call number for close is 3
//         pop rdi                     // specifying wha file needs to be closed
//         syscall
// exit