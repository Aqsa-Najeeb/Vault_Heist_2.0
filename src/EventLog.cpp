#include "EventLog.h"
using namespace std;

LogEvent::LogEvent() : type(EventType::DONE), message("")
{}

LogEvent::LogEvent(int type, const string& message) : type(type), message(message)
{}

LogEvent::~LogEvent()
{}

LogEvent& LogEvent::operator=(const LogEvent& other)
{
    if (this != &other)
    {
        type = other.type;
        message = other.message;
    }
    return *this;
}

EventLog::EventLog() : m_count(0), m_enabled(true)
{}

EventLog::~EventLog()
{}

void EventLog::record(const LogEvent& ev)
{
    if (!m_enabled)
    {
        return;
    }
    if (m_count < MAX_EVENTS)
    {
        m_events[m_count++] = ev;
    }
    else
    {
        for (int i = 0; i < MAX_EVENTS - 1; ++i)
        {
            m_events[i] = m_events[i + 1];
        }
        m_events[MAX_EVENTS - 1] = ev;
    }
}

void EventLog::record(int type, const string& message)
{
    record(LogEvent(type, message));
}

void EventLog::record(const string& message)
{
    record(LogEvent(EventType::DONE, message));
}

void EventLog::undo()
{
    if (m_count > 0)
    {
        m_count--;
    }
}

void EventLog::clear()
{
    m_count = 0;
}

int EventLog::size() const
{
    return m_count;
}

const LogEvent& EventLog::get(int index) const
{
    static LogEvent emptyEvent;
    if (index >= 0 && index < m_count)
    {
        return m_events[index];
    }
    return emptyEvent;
}

std::string EventLog::getMessage(int index) const
{
    if (index >= 0 && index < m_count)
    {
        return m_events[index].message;
    }
    return "";
}

void EventLog::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool EventLog::isEnabled() const
{
    return m_enabled;
}
