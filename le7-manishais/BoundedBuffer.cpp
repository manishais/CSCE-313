#include "BoundedBuffer.h"
#include <cassert>
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
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    // 3. Then push the vector at the end of the queue
    // 4. Wake up threads that were waiting for push

    vector<char> vec(msg, msg + size);
    unique_lock<mutex> lock(mute);
    slot.wait(lock, [this]  { return ((int)q.size() < cap);});

    q.push(vec);
    lock.unlock();

    data.notify_one();
}

int BoundedBuffer::pop (char* msg, int size) {
    // 1. Wait until the queue has at least 1 item
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    // 4. Wake up threads that were waiting for pop
    // 5. Return the vector's length to the caller so that they know how many bytes were popped

    unique_lock<mutex> lockk(mute);
    data.wait(lockk, [this] { return q.size() >= 1; });

    vector<char> items = q.front();
    q.pop();
    lockk.unlock();

    assert((int)items.size() <= size);
    memcpy(msg, items.data( ), (int) items.size());

    slot.notify_one();

    return (int) items.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}