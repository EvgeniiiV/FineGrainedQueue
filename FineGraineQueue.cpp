#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <shared_mutex>
#include <atomic>
#include <mutex>
#include <vector>
using namespace std;


struct Node
{
    int _value;
    Node* _next;
    std::mutex node_mutex;
    Node(int value) : _value(value), _next{ nullptr }{}
};

class FineGrainedQueue
{
private:
    Node* _head;
    std::mutex queue_mutex;    

public:
    FineGrainedQueue() : _head(nullptr) {}

    void push_back(int data)
    {       
        Node* node = new Node(data);        
        if (_head == nullptr)
        {
            _head = node;
            return;
        }        
        Node* last = _head;
        while (last->_next != nullptr)
        {
            last = last->_next;
        }        
        last->_next = node;
        return;
    }  


    void insertIntoMiddle(int value, int pos)
    {
        Node* newNode = new Node(value);
        queue_mutex.lock();
        if (!_head)
        {
            _head = newNode;
            queue_mutex.unlock();
            //debug
            lock_guard<mutex>l(queue_mutex);
            Node* ccurrent = _head;
            while (ccurrent != nullptr)
            {
                cout << ccurrent->_value << " ";
                ccurrent = ccurrent->_next;
            }
            cout << "thread id " << this_thread::get_id() << endl;
            //
            return;
        }
        if (pos == 0)
        {
            newNode->_next = _head;
            _head = newNode;
            queue_mutex.unlock();
            //debug
            lock_guard<mutex>l(queue_mutex);
            Node* ccurrent = _head;
            while (ccurrent != nullptr)
            {
                cout << ccurrent->_value << " ";
                ccurrent = ccurrent->_next;
            }
            cout << "thread id " << this_thread::get_id() << endl;
            //
            return;
        }
        Node* current = _head;
        current->node_mutex.lock();
        queue_mutex.unlock();
        int currPos = 0;        
        while (currPos < pos - 1 && current->_next != nullptr)
        {
            Node* prev = current;
            current->_next->node_mutex.lock();
            current = current->_next;            
            prev->node_mutex.unlock();
            currPos++;
        }
        Node* next = current->_next;
        current->_next = newNode;
        current->node_mutex.unlock();       
        newNode->_next = next;
        //debug
        lock_guard<mutex>l(queue_mutex);
        Node* ccurrent = _head;
        while (ccurrent != nullptr)
        {
            cout << ccurrent->_value << " ";
            ccurrent = ccurrent->_next;
        }
        cout << "thread id " << this_thread::get_id() << endl; 
        //
    } 

};

int main()
{
    FineGrainedQueue FGQ;
    int size = 10;
    for (size_t i = 0; i < size; i++)
    {
        FGQ.push_back(11);
    }
    
    vector<thread>threads;
    for (int i = 0; i < 8; i++)
    {        
        threads.push_back(thread ([&FGQ, i]() {FGQ.insertIntoMiddle(i, i); }));
    }

    for(auto& i: threads)
    {       
        i.join();
    }   

    return 0;
}