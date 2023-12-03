#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <getopt.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

// ecgno to use for datamsgs
#define EGCNO 1

using namespace std;

void patient_thread_function(int pnum, int reqnum, BoundedBuffer* reqbuf) {
    double time = 0;
    char* msgbuf = new char[sizeof(datamsg)];

    int i = 0;
    while (i < reqnum) {
        datamsg msg(pnum, time, EGCNO);
        time += 0.004;
        memcpy(msgbuf, &msg, sizeof(datamsg));
        reqbuf->push(msgbuf, sizeof(datamsg));
        i++;
    }

    delete[] msgbuf;
}

void file_thread_function(int m, BoundedBuffer* reqbuf, string filename, int filelen) {
    string path = "received/" + filename;
    FILE* fileobj = fopen(path.c_str(), "w");
    fseek(fileobj, filelen, SEEK_SET);
    fclose(fileobj);

    // while offset < file_size, produce a filemsg(offser, m) + filename and push ti request_buffer
    //      - incrementing offset; and be careful with the final message
    int remBytes = filelen % m;

    int offset = 0;
    while (offset < (filelen - remBytes)) 
    {
        filemsg req(offset, m);
        reqbuf->push((char*)&req, sizeof(filemsg));
        offset += m;
    }

    filemsg req((filelen - remBytes), remBytes);
    reqbuf->push((char*)&req, sizeof(filemsg));
}

void histogram_thread_function(BoundedBuffer* respbuf, HistogramCollection* hc) 
{
    int pairlen = sizeof(pair<int, double>);
    char* bufresp = new char[pairlen];

    while (true) 
    {
        pair<int, double> rep;
        respbuf->pop((char*)&rep, sizeof(rep));

        if (rep.first == -1) 
        {
            break;
        }

        hc->update(rep.first, rep.second);
    }

    delete[] bufresp;

   // Assuming 'respbuf' is a BoundedBuffer object containing pairs of integers and doubles.

}

void worker_thread_function(BoundedBuffer* reqbuf, FIFORequestChannel* channel, BoundedBuffer* respbuf, string filename) {
    int msgsize;
    char* msgbuf;

    if (filename != "") 
    {
        msgsize = sizeof(filemsg);
        msgbuf = new char[msgsize];
    } 
    else 
    {
        msgsize = sizeof(datamsg);
        msgbuf = new char[msgsize];
    }

    while (true) 
    {
        reqbuf->pop(msgbuf, msgsize);
        MESSAGE_TYPE mreq = *((MESSAGE_TYPE*)msgbuf);

        if (mreq == QUIT_MSG)
        {
            break;
        }

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

        if (mreq == DATA_MSG) 
        {
            // datamsg msg = *((datamsg*)msgbuf);
            datamsg msg = *reinterpret_cast<datamsg*>(msgbuf);
            channel->cwrite(msgbuf, sizeof(datamsg));

            double ecgresp;
            channel->cread(&ecgresp, sizeof(ecgresp));

            auto histpair = make_pair(msg.person, ecgresp);
            respbuf->push((char*)&histpair, sizeof(histpair));
        }
        else if (mreq == FILE_MSG) 
        {
            filemsg popfilereq = *(filemsg*)msgbuf;
            string path = "received/" + filename;
            FILE* fileobj = fopen(path.c_str(), "r+");
            fseek(fileobj, popfilereq.offset, SEEK_SET);

            char* buf = new char[sizeof(filemsg) + filename.length() + 1];
            memcpy(buf, &popfilereq, sizeof(filemsg));
            strcpy(buf + sizeof(filemsg), filename.c_str());

            channel->cwrite(buf, sizeof(filemsg) + filename.length() + 1);

            char* data = new char[popfilereq.length];
            channel->cread(data, popfilereq.length);
            fwrite(data, 1, popfilereq.length, fileobj);

            delete[] buf;
            delete[] data;
            fclose(fileobj);
        }
    }

    delete[] msgbuf;
}

int main(int argc, char* argv[]) {
    int n = 1000;  // default number of requests per "patient"
    int p = 10;    // number of patients [1,15]
    int w = 100;   // default number of worker threads
    int h = 20;    // default number of histogram threads
    int b = 20;    // default capacity of the request buffer (should be changed)
    int m = MAX_MESSAGE;  // default capacity of the message buffer
    string f = "";        // name of file to be transferred

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

    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*)to_string(m).c_str(), nullptr);
    }

    FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
    HistogramCollection hc;

    // vector of producer threads (if data, p elements; if file, 1 element)
    vector<thread> prodthreads;
    // vector of FIFOs (w elements)
    vector<FIFORequestChannel*> channels;
    // vector of worker threads (w elements)
    vector<thread> workthreads;
    // vector of histogram threads (if data, h elements; if file, 0 elements)
    vector<thread> histhreads;

    int i = 0;
    while (i < p) 
    {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
        i++;
    }

    struct timeval start, end;
    gettimeofday(&start, 0);

    // If data:
    if (f == "") 
    {
        int pnum = 1;
        while (pnum <= p) 
        {
            prodthreads.push_back(thread(patient_thread_function, pnum, n, &request_buffer));
            pnum++;
        }
    } 
    else 
    {
        filemsg f_msg(0, 0);
        string filename = f;
        int buflen = sizeof(filemsg) + (filename.size() + 1);

        char* filebuf = new char[buflen];
        memcpy(filebuf, &f_msg, sizeof(filemsg));
        strcpy(filebuf + sizeof(filemsg), filename.c_str());
        chan->cwrite(filebuf, buflen);
        int64_t filesize;
        chan->cread(&filesize, sizeof(filesize));
        delete[] filebuf;

        // Create 1 file_thread
        workthreads.push_back(thread(file_thread_function, m, &request_buffer, filename, filesize));
    }

    // Create w worker_threads
    MESSAGE_TYPE new_chan = NEWCHANNEL_MSG;
    i = 0;
    while (i < w) 
    {
        chan->cwrite(&new_chan, sizeof(MESSAGE_TYPE));
        char channelName[30];
        chan->cread(&channelName, sizeof(channelName));

        FIFORequestChannel* newChan = new FIFORequestChannel(channelName, FIFORequestChannel::CLIENT_SIDE);
        channels.push_back(newChan);
        workthreads.push_back(thread(worker_thread_function, &request_buffer, newChan, &response_buffer, f));
        i++;
    }

    // If data:
    if (f == "") 
    {
        // Create h histogram_threads
        i = 0;
        while (i < h) 
        {
            histhreads.push_back(thread(histogram_thread_function, &response_buffer, &hc));
            i++;
        }
    }

    // Join all threads here
    i = 0;
    while (i < prodthreads.size()) 
    {
        prodthreads[i].join();
        i++;
    }

    datamsg quit_msg(0, 0, 0);
    quit_msg.mtype = QUIT_MSG;
    i = 0;
    while (i < w) 
    {
        request_buffer.push((char*)&quit_msg, sizeof(datamsg));
        i++;
    }

    i = 0;
    while (i < workthreads.size()) 
    {
        workthreads[i].join();
        i++;
    }

    if (f == "") 
    {
        pair<int, double> rep = pair(-1, -1);
        i = 0;
        while (i < h) 
        {
            response_buffer.push((char*)&rep, sizeof(rep));
            i++;
        }

        i = 0;
        while (i < histhreads.size()) 
        {
            histhreads[i].join();
            i++;
        }

        hc.print();
    }

    gettimeofday(&end, 0);

    int secs = ((1e6 * end.tv_sec - 1e6 * start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int)1e6);
    int usecs = (int)((1e6 * end.tv_sec - 1e6 * start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int)1e6);
    cout << "Took " << secs << " seconds and " << usecs << " microseconds" << endl;

    MESSAGE_TYPE q = QUIT_MSG;

    // Quit and close all channels in FIFO array
    for (auto&& c : channels) 
    {
        c->cwrite(&q, sizeof(MESSAGE_TYPE));
        delete c;
    }

    chan->cwrite(&q, sizeof(MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

    // Wait for the server to exit
    wait(nullptr);

    return 0;
}
