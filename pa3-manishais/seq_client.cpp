#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;


void patient_thread_function (/* add necessary arguments */) {
    // functionality of the patient threads

    // take a pateint p_no
    // for n requests, prodi=uce a datamsg(p_no, time, ECGNO) and push to request_buffer
    //      - time dependent on current requests
    //      - at 0 -> time = 0.000; at 1 -> time = 0.004; at 2 -> time = 0.008; ...
}

void file_thread_function (/* add necessary arguments */) {
    // functionality of the file thread

    // file size
    // open output file; allocate the memort fseek; close the file
    // while offset < file_size, produce a filemsg(offser, m) + filename and push ti request_buffer
    //      - incrementing offset; and be careful with the final message
}

void worker_thread_function (/* add necessary arguments */) {
    // functionality of the worker threads

    // forever loop
    // pop message from the request_buffer
    // view line 120 in server (process_request function) for how to decide current message
    // send the messaget across a FIFO channel
    // collect response 
    // if DATA:
    //      - create pair of p_no from message an response from server
    //      - push that pair to the response_buffer
    // if FILE:
    //      - collect the filename from the message
    //      - open the file in update mode
    //      - fseek(SEEK_SET) to offset of the filemsg
    //      - write the buffer from the server

}

void histogram_thread_function (/* add necessary arguments */) {
    // functionality of the histogram threads

    // forever loop
    // pop response from the response_buffer
    // call HC::update(resp->p_no, resp_double)
}


int main (int argc, char* argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 20;		// default capacity of the request buffer (should be changed)
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
		}
	}
    
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    
	// initialize overhead (including the control channel)
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // array of producrer threads (if data, p elements; if file, 1 element)
    // array of FIFOs (w elements)
    // array of worker threads (w elements)
    // array of histrgram threads (if data, h elements; if file, zero elements)

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* create all threads here */
    // if data:
    //      - create p patient_threads
    // if file:
    //      - create 1 file_thread
    //
    // create w worker_threads
    //      - create w channels
    //
    // if data:
    //      - create h hist_threads

	/* join all threads here */
    // iterate over all thread arrays, calling join
    //      - order is imprtant; producers before consumers

	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    // quit and close all channels in FIFO array

    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}
