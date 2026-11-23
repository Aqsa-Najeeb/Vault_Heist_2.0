#pragma once
#include <string>
using namespace std;

struct EventType
{
    static const int VISIT = 0;
    static const int COMPARE = 1;
    static const int INSERT = 2;
    static const int UNLINK = 3;
    static const int SWAP_VALUE = 4;
    static const int ROTATE = 5;
    static const int UPDATE_HEIGHT = 6;
    static const int UPDATE_SIZE = 7;
    static const int BALANCE_ALERT = 8;
    static const int HIGHLIGHT_SUBTREE = 9;
    static const int HEAP_BUILD = 10;
    static const int HEAP_SWAP = 11;
    static const int HEAPIFY_START = 12;
    static const int HEAPIFY_DONE  = 13;
    static const int EXTRACT_MAX = 14;
    static const int MARK_SORTED = 15;
    static const int HEAP_SIZE_CHANGE = 16;
    static const int HEAP_SORT_COMPLETE= 17;
    static const int DONE = 18;
};

struct LogEvent
{
    int type;
    string message;

    LogEvent();
    LogEvent(int type, const std::string& message);
    ~LogEvent();
    LogEvent& operator=(const LogEvent& other);
};

class EventLog
{
public:
    static const int MAX_EVENTS = 64;

    EventLog();
    virtual ~EventLog();
    virtual void record(const LogEvent& ev);
    virtual void record(int type, const std::string& message);
    virtual void record(const std::string& message);
    void undo();
    void clear();
    int size() const;
    const LogEvent& get(int index) const;
    std::string getMessage(int index) const;
    void setEnabled(bool enabled);
    bool isEnabled() const;

private:
    LogEvent m_events[MAX_EVENTS];
    int m_count;
    bool m_enabled;
};
