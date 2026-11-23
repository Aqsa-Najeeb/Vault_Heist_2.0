#pragma once
#include "EventLog.h"
#include <string>
using namespace std;

class MaxHeap
{
public:
    static const int MAX_CAPACITY = 32;

    MaxHeap();
    MaxHeap(const int* arr, int n);
    ~MaxHeap();
    MaxHeap(const MaxHeap& other);
    MaxHeap& operator=(const MaxHeap& other);

    int size() const;
    int capacity() const;
    int get(int idx) const;
    const int* getData() const;
    int* getData();
    void setSize(int s);
    void clear();

    bool insert(int val, EventLog* log = nullptr);
    int extractMax(EventLog* log = nullptr);
    int peekMax() const;
    void siftUp(int idx, EventLog* log = nullptr);
    void siftDown(int idx, int heapSize, EventLog* log = nullptr);
    void buildMaxHeap(const int* arr, int n, EventLog* log = nullptr);
    static void heapSort(int* arr, int n, EventLog* log = nullptr);

    bool isMaxHeap(int idx = 0) const;
    static bool isMaxHeapArray(const int* arr, int n);

    string serialize() const;
    void deserialize(const std::string& data);

private:
    int* m_data;
    int m_capacity;
    int m_size;

    void ensureCapacity(int cap);
    void swapElements(int i, int j, EventLog* log, const char* reason = nullptr);
};
