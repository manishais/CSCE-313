#include "BoundedBuffer.h"
#include <iostream>
#include <assert.h>
#include <cstring>

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
} 

void BoundedBuffer::push (char* msg, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    //      use one of the vector constructors
    vector<char> vec(msg, msg + size);
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    //      waiting on slot available
    unique_lock<mutex> lock(mute);
    slot.wait(lock, [this]  { return ((int)q.size() < cap);});
    // 3. Then push the vector at the end of the queue
    q.push(vec);
    lock.unlock();
    // 4. Wake up threads that were waiting for push
    //      // notifying data available
    data.notify_one();
}

int BoundedBuffer::pop (char* msg, int size) {
    // 1. Wait until the queue has at least 1 item
    //      waiting on data available
    unique_lock<mutex> lockk(mute);
    data.wait(lockk, [this] { return q.size() >= 1; });
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    vector<char> items = q.front();
    q.pop();
    lockk.unlock();
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    //      use vector::data()
    // int itemsize = items.size();
    //const char* item = items.data();
    assert((int)items.size() <= size);
    memcpy(msg, items.data( ), (int) items.size());
    // 4. Wake up threads that were waiting for pop
    //      notifying slot available
    slot.notify_one();
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    return (int) items.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}