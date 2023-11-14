/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name:Manisha Subrahmanya
	UIN:331001661
	Date:
*/
#include "common.h"
#include "FIFORequestChannel.h"

using namespace std;


int main (int argc, char *argv[]) 
{
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	int m = MAX_MESSAGE;
	bool newChannel = false;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi(optarg);
				break;
			case 'c':
					newChannel = true;
					break;
		}
	}

	// give arg to server - needs ./server -m <val for -m arg>, 'null'
	char* args [] = {(char*)"./server", (char*)"-m", (char*)to_string(m).c_str(), NULL};

	// fork it
	pid_t childpid = fork();

	if(childpid < 0)		// fork failed
	{
		cout << "ERROR!" << endl;
		return -1;
	}

	// in the child, run execvp using the server arguments
	else if(childpid == 0)			// child process
	{
		execvp(args[0], args);		// running new program - args[0] stores the name of the new program and args is the array that stores args to be passed into program
	}

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	//chan.push_back(&chan);

	if (newChannel) 
	{
			MESSAGE_TYPE message = NEWCHANNEL_MSG;
			chan.cwrite(&message, sizeof(MESSAGE_TYPE));
			char channelName[30];
			chan.cread(&channelName, sizeof(channelName) + 1);

			FIFORequestChannel chan = FIFORequestChannel(channelName, FIFORequestChannel::CLIENT_SIDE);
	}

	//FIFORequestChannel chan = *(chan.back());

	if(p != -1 && t != -1 && e != -1)
	{
		// example data point request
    	char buf[MAX_MESSAGE]; // 256
  	 	// datamsg x(1, 0.0, 1);		// change from hardcoding to user's value
   		datamsg x(p, t, e);	
	
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg)); // question
		double reply;
		chan.cread(&reply, sizeof(double)); //answer
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
	
	else if(p != -1)				// request 1000 datapoints
	{
		char buff[MAX_MESSAGE];
		ofstream filee("received/x1.csv");
		for(double t = 0 ; t < 4 ; t += 0.004)
		{
			// datamsg x(1, 0.0, 1);		// change from hardcoding to user's value
			datamsg x(p, t, 1);	

			// ECG 1
			memcpy(buff, &x, sizeof(datamsg));
			chan.cwrite(buff, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); //answer
			filee << t << ", " << reply << ", " << reply << ",";

			// ECG 2
			x.ecgno = 2;
			memcpy(buff, &x, sizeof(datamsg));
			chan.cwrite(buff, sizeof(datamsg)); // question
			//double replyy;
			chan.cread(&reply, sizeof(double)); //answer

			// Output file
			filee << reply << endl;
			//cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		}

	}
	
    // sending a non-sense message, you need to change this
	//filemsg fm(0, 0);
	// string fname = "teslkansdlkjflasjdf.dat";
	//string fname = filename;

	if(filename != "")
	{
		string fname = filename;
		ofstream filee("received/" + fname);
		filemsg fm(0, 0);
		
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];				// request buffer
		memcpy(buf2, &fm, sizeof(filemsg));
		memcpy(buf2 + sizeof(filemsg), fname.c_str(), fname.length() + 1);
		chan.cwrite(buf2, len);  // I want the file length;

		__int64_t lenfile = 0;
		chan.cread(&lenfile, m);		// returns the file length

		__int64_t remBytes = lenfile % m;
		__int64_t trans = lenfile - remBytes;

		for(__int64_t i = 0 ; i < trans ; i += m)
		{
			filemsg neww(i, m);
			memcpy(buf2, &neww, sizeof(filemsg));
			memcpy(buf2 + sizeof(filemsg), fname.c_str(), fname.length() + 1);
			chan.cwrite(buf2, len);

			char* buf3 = new char[m];
			chan.cread(buf3, m);
			filee.write(buf3, m);

			delete [] buf3;
		}

		filemsg neww(trans, remBytes);
		memcpy(buf2, &neww, sizeof(filemsg));
		memcpy(buf2 + sizeof(filemsg), fname.c_str(), fname.length() + 1);
		chan.cwrite(buf2, len);

		char* buf3 = new char[remBytes];
		chan.cread(buf3, remBytes);

		filee.write(buf3, remBytes);
		delete [] buf3;
		
		filee.close();
		delete [] buf2;

		// close channel
		MESSAGE_TYPE message = QUIT_MSG;
		chan.cwrite(&message, sizeof(MESSAGE_TYPE));
	}
}