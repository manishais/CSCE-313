#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

using namespace std;

void fileType(mode_t mode) 
{
    if (S_ISREG(mode)) 
    {
        cout << "Regular File" << endl;
    } 
    else if (S_ISFIFO(mode)) 
    {
        cout << "FIFO/pipe" << endl;
    } 
    else if (S_ISLNK(mode)) 
    {
        cout << "Symbolic Link" << endl;
    } 
    else 
    {
        cout << "Unknown file type" << endl;
    }
}

void ownerPermissions(mode_t mode) 
{
    cout << "Owner's permissions: ";
    if (mode & S_IRUSR) 
    {
        cout << "Read ";
    }
    if (mode & S_IWUSR) 
    {
        cout << "Write ";
    }
    if (mode & S_IXUSR) 
    {
        cout << "Execute ";
    }
    cout << endl;
}

int main() 
{
    struct stat filedata;
    string path;

    cout << "Enter the file path: ";
    getline(cin, path);

    if (lstat(path.c_str(), &filedata) == 0) 
    {
        cout << "File type: ";
        fileType(filedata.st_mode);
        ownerPermissions(filedata.st_mode);
    }
    else 
    {
        cout << "Error" << endl;
    }

    return 0;
}
