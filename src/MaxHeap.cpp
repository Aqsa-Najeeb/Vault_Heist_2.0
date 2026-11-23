#include "MaxHeap.h"
using namespace std;

MaxHeap::MaxHeap() : m_capacity(MAX_CAPACITY), m_size(0)
{
    m_data = new int[m_capacity];
    for (int i = 0; i < m_capacity; ++i)
    {
        m_data[i] = 0;
    }
}

MaxHeap::MaxHeap(const int* arr, int n) : m_capacity(n > MAX_CAPACITY ? n * 2 : MAX_CAPACITY), m_size(0)
{
    m_data = new int[m_capacity];
    for (int i = 0; i < m_capacity; ++i)
    {
        m_data[i] = 0;
    }
    buildMaxHeap(arr, n, nullptr);
}

MaxHeap::~MaxHeap()
{
    delete[] m_data;
}

MaxHeap::MaxHeap(const MaxHeap& other) : m_capacity(other.m_capacity), m_size(other.m_size)
{
    m_data = new int[m_capacity];
    for (int i = 0; i < m_size; ++i)
    {
        m_data[i] = other.m_data[i];
    }
}

MaxHeap& MaxHeap::operator=(const MaxHeap& other)
{
    if (this != &other)
    {
        delete[] m_data;
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_data = new int[m_capacity];
        
        for (int i = 0; i < m_size; ++i)
        {
            m_data[i] = other.m_data[i];
        }
    }
    return *this;
}

void MaxHeap::ensureCapacity(int cap)
{
    if (cap <= m_capacity)
    {
        return;
    }
    int newCap = m_capacity * 2;
    if (newCap < cap)
    {
        newCap = cap;
    }
    int* newData = new int[newCap];
    for (int i = 0; i < m_size; ++i)
    {
        newData[i] = m_data[i];
    }
    delete[] m_data;
    m_data = newData;
    m_capacity = newCap;
}

int MaxHeap::size() const
{
    return m_size;
}

int MaxHeap::capacity() const
{
    return m_capacity;
}

int MaxHeap::get(int idx) const
{
    if (idx >= 0 && idx < m_size)
    {
        return m_data[idx];
    }
    return -1;
}

const int* MaxHeap::getData() const
{
    return m_data;
}

int* MaxHeap::getData()
{
    return m_data;
}

void MaxHeap::setSize(int s)
{
    if (s >= 0 && s <= m_capacity)
    {
        m_size = s;
    }
}

void MaxHeap::clear()
{
    m_size = 0;
}

void MaxHeap::swapElements(int i, int j, EventLog* log, const char* reason)
{
    int temp = m_data[i];
    m_data[i] = m_data[j];
    m_data[j] = temp;
    if (log)
    {
        std::string msg = "[HEAP_SWAP] Swap idx " + to_string(i) + " (" + to_string(m_data[j]) + ") with idx " + to_string(j) + " (" + to_string(m_data[i]) + ")";
        if (reason)
        {
            msg += string(" - ") + reason;
        }
        log->record(EventType::HEAP_SWAP, msg);
    }
}

void MaxHeap::siftUp(int idx, EventLog* log)
{
    int curr = idx;
    while (curr > 0)
    {
        int p = (curr - 1) / 2;
        if (m_data[curr] > m_data[p])
        {
            swapElements(curr, p, log, "siftUp");
            curr = p;
        }
        else
        {
            break;
        }
    }
}

void MaxHeap::siftDown(int idx, int heapSize, EventLog* log)
{
    int curr = idx;
    while (true)
    {
        int largest = curr;
        int l = 2 * curr + 1;
        int r = 2 * curr + 2;

        if (l < heapSize && m_data[l] > m_data[largest])
        {
            largest = l;
        }
        if (r < heapSize && m_data[r] > m_data[largest])
        {
            largest = r;
        }

        if (largest != curr)
        {
            swapElements(curr, largest, log, "siftDown");
            curr = largest;
        }
        else
        {
            break;
        }
    }
}

void MaxHeap::buildMaxHeap(const int* arr, int n, EventLog* log)
{
    ensureCapacity(n);
    m_size = n;
    for (int i = 0; i < n; ++i)
    {
        m_data[i] = arr[i];
    }

    if (log)
    {
        log->record(EventType::HEAP_BUILD, "[BUILD_HEAP] Constructing bottom-up with " + to_string(n) + " elements");
    }

    for (int i = (n - 2) / 2; i >= 0; --i)
    {
        if (log)
        {
            log->record(EventType::HEAPIFY_START, "[HEAPIFY_START] Sifting down index " + to_string(i));
        }
        siftDown(i, m_size, log);
        if (log)
        {
            log->record(EventType::HEAPIFY_DONE, "[HEAPIFY_DONE] Index " + to_string(i) + " heapified");
        }
    }

    if (log)
    {
        log->record(EventType::DONE, "[BUILD_HEAP] Max Heap construction complete");
    }
}

bool MaxHeap::insert(int val, EventLog* log)
{
    ensureCapacity(m_size + 1);
    int idx = m_size++;
    m_data[idx] = val;

    if (log)
    {
        log->record(EventType::INSERT, "[INSERT] Appended " + to_string(val) + " at index " + to_string(idx));
    }

    siftUp(idx, log);

    if (log)
    {
        log->record(EventType::DONE, "[DONE] Insert complete");
    }
    return true;
}

int MaxHeap::extractMax(EventLog* log)
{
    if (m_size <= 0)
    {
        return -1;
    }

    int maxVal = m_data[0];
    if (log)
    {
        log->record(EventType::EXTRACT_MAX, "[EXTRACT_MAX] Root element " + to_string(maxVal) + " extracted");
    }

    swapElements(0, m_size - 1, log, "root swapped with last active element");
    m_size--;

    if (log)
    {
        log->record(EventType::HEAP_SIZE_CHANGE, "[HEAP_SIZE] Active heap reduced to " + to_string(m_size));
        log->record(EventType::MARK_SORTED, "[MARK_SORTED] Element " + to_string(maxVal) + " moved to sorted region");
    }

    if (m_size > 0)
    {
        if (log)
        {
            log->record(EventType::HEAPIFY_START, "[HEAPIFY_START] Restoring heap property from root");
        }
        siftDown(0, m_size, log);
        if (log)
        {
            log->record(EventType::HEAPIFY_DONE, "[HEAPIFY_DONE] Heap property restored");
        }
    }

    if (log)
    {
        log->record(EventType::DONE, "[DONE] ExtractMax complete");
    }
    return maxVal;
}

int MaxHeap::peekMax() const
{
    if (m_size > 0)
    {
        return m_data[0];
    }
    return -1;
}

void MaxHeap::heapSort(int* arr, int n, EventLog* log)
{
    if (!arr || n <= 1)
    {
        if (log)
        {
            log->record(EventType::HEAP_SORT_COMPLETE, "[HEAP_SORT] Array completely sorted in ascending order!");
            log->record(EventType::DONE, "[DONE] Heap sort complete");
        }
        return;
    }

    auto siftDownRaw = [&](int idx, int sz) 
        {
        
        int curr = idx;
        while (true)
        {
            int largest = curr;
            int l = 2 * curr + 1;
            int r = 2 * curr + 2;

            if (l < sz && arr[l] > arr[largest])
            {
                largest = l;
            }
            if (r < sz && arr[r] > arr[largest])
            {
                largest = r;
            }

            if (largest != curr)
            {
                int t = arr[curr];
                arr[curr] = arr[largest];
                arr[largest] = t;
                if (log)
                {
                    log->record(EventType::HEAP_SWAP, "[HEAP_SWAP] SiftDown swap");
                }
                curr = largest;
            }
            else
            {
                break;
            }
        }
    };

    if (log)
    {
        log->record(EventType::HEAP_BUILD, "[HEAP_SORT] Starting bottom-up heap construction");
    }

    for (int i = (n - 2) / 2; i >= 0; --i)
    {
        siftDownRaw(i, n);
    }

    if (log)
    {
        log->record(EventType::HEAPIFY_DONE, "[HEAP_SORT] Built initial Max Heap");
    }

    for (int i = n - 1; i > 0; --i)
    {
        int t = arr[0];
        arr[0] = arr[i];
        arr[i] = t;
        if (log)
        {
            log->record(EventType::HEAP_SWAP, "[HEAP_SWAP] Root swapped with position " + to_string(i));
            log->record(EventType::MARK_SORTED, "[HEAP_SORT] Locked maximum " + to_string(t) + " at index " + to_string(i));
        }
        siftDownRaw(0, i);
    }

    if (log)
    {
        log->record(EventType::HEAP_SORT_COMPLETE, "[HEAP_SORT] Array completely sorted in ascending order!");
        log->record(EventType::DONE, "[DONE] Heap sort complete");
    }
}

bool MaxHeap::isMaxHeap(int idx) const
{
    int l = 2 * idx + 1;
    int r = 2 * idx + 2;

    if (l < m_size)
    {
        if (m_data[idx] < m_data[l])
        {
            return false;
        }
        if (!isMaxHeap(l))
        {
            return false;
        }
    }

    if (r < m_size)
    {
        if (m_data[idx] < m_data[r])
        {
            return false;
        }
        if (!isMaxHeap(r))
        {
            return false;
        }
    }

    return true;
}

bool MaxHeap::isMaxHeapArray(const int* arr, int n)
{
    if (!arr || n <= 1)
    {
        return true;
    }
    for (int i = 0; i <= (n - 2) / 2; ++i)
    {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < n && arr[i] < arr[l])
        {
            return false;
        }
        if (r < n && arr[i] < arr[r])
        {
            return false;
        }
    }
    return true;
}

std::string MaxHeap::serialize() const
{
    std::string out = std::to_string(m_capacity) + " " + to_string(m_size);
   
    for (int i = 0; i < m_capacity; ++i)
    {
        out += " " + to_string(m_data[i]);
    }
    
    return out;
}

static bool getNextToken(const std::string& data, size_t& pos, std::string& tok)
{
    while (pos < data.size() && (data[pos] == ' ' || data[pos] == '\t' || data[pos] == '\n' || data[pos] == '\r'))
    {
        pos++;
    }
    if (pos >= data.size())
    {
        return false;
    }
    size_t start = pos;
    while (pos < data.size() && data[pos] != ' ' && data[pos] != '\t' && data[pos] != '\n' && data[pos] != '\r')
    {
        pos++;
    }
    tok = data.substr(start, pos - start);
    return true;
}

void MaxHeap::deserialize(const string& data)
{
    size_t pos = 0;
    string tok;
    
    if (getNextToken(data, pos, tok))
    {
        int cap = stoi(tok);
       
        if (getNextToken(data, pos, tok))
        {
            int sz = std::stoi(tok);
            delete[] m_data;
            m_capacity = cap > MAX_CAPACITY ? cap : MAX_CAPACITY;
            m_size = sz;
            m_data = new int[m_capacity];
            for (int i = 0; i < m_capacity; ++i)
            {
                if (getNextToken(data, pos, tok))
                {
                    m_data[i] = std::stoi(tok);
                }
                else
                {
                    m_data[i] = 0;
                }
            }
        }
    }
}
