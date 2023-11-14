#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

int main()
{
    int maxfiles = 0;

    while(true)
    {
        int filedes = open("q5test.txt", O_RDONLY);           // open test.txt repeatedly

        if (filedes == -1)          // error
        {
            if(errno == EMFILE)         // testing that the open system call failed with emfile error
            {
                break;              // exit loop
            }
        }
        maxfiles = maxfiles + 1;        // increment max
    }
   cout << "max reached: " << maxfiles << endl;     // max number of open files that a process can open
   return 0;
}
