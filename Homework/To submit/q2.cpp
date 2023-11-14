#include <iostream>
#include <ctime>  // For clock_gettime
#include <unistd.h>  // For getpid
using namespace std;

int funcCall(int x, int y)
{
   return x + y;
}


int main()
{
    struct timespec start, end; // Declare timespec structures for time measurements
    int a, b;
    pid_t pid;
    long long elapsed_ns;

    cout << "Enter A and B:" << endl;
    cin >> a >> b;

    cout << "FUNCTION CALL:" << endl;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);  // Start time
    int result = funcCall(a, b);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);    // End time

    // Calculate elapsed time in nanoseconds
    elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    cout << "Elapsed Time for Function Call: " << elapsed_ns << " nanoseconds" << endl;

    cout << "SYSTEM CALL:" << endl;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);  // Start time
    pid = getpid();
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);    // End time

    // Calculate elapsed time in nanoseconds
    elapsed_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    cout << "Elapsed Time for System Call (getpid): " << elapsed_ns << " nanoseconds" << endl;

    return 0;
}
