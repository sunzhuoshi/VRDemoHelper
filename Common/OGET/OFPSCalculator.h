#pragma once
#include <Windows.h>

// TODO: minimize overhead as small as possible
template<unsigned int BUFFER_SIZE>
class OFPSCalculator {
public:
    bool init() {
        return 0 != QueryPerformanceFrequency(&m_frequency);
    }
    bool presentNewFrameAndGetFPS(unsigned int &FPS) {
        LARGE_INTEGER now;
        bool calculateFPS = false;
        if (QueryPerformanceCounter(&now)) {
            pushBack(now);
            // frames span over 1s 
            if (front().QuadPart - cursor().QuadPart >= m_frequency.QuadPart) {
                LARGE_INTEGER nextFrameData;
                // move the cursor to the newest one in 1s
                while (nextCursor(nextFrameData)) {
                    if (front().QuadPart - nextFrameData.QuadPart >= m_frequency.QuadPart) {
                        stepCursor();
                    }
                    else {
                        break;
                    }
                }
                calculateFPS = true;
            }
            // less than 1s, but buff is full
            else if (full()) {
                calculateFPS = true;
            }
        }
        if (calculateFPS) {
            FPS = static_cast<unsigned int>(double((m_begin - m_cursor + BUFFER_SIZE) % BUFFER_SIZE + 1) * m_frequency.QuadPart / (front().QuadPart - cursor().QuadPart));
        }
        return calculateFPS;
    }
private:
    inline void pushBack(LARGE_INTEGER dataItem) {
        m_begin = (m_begin + 1) % BUFFER_SIZE;
        m_buff[m_begin] = dataItem;
        if (m_begin == m_end) {
            stepEnd();
        }
    }
    inline bool full() {
        return size() == BUFFER_SIZE - 1;
    }
    inline size_t size() {
        return (m_begin - m_end + BUFFER_SIZE) % BUFFER_SIZE;
    }
    inline LARGE_INTEGER cursor() {
        return m_buff[m_cursor];
    }
    inline LARGE_INTEGER front() {
        return m_buff[m_begin];
    }
    inline bool nextCursor(LARGE_INTEGER& dataItem) {
        unsigned int nextCursor = m_cursor + 1;
        if ((m_begin - nextCursor + BUFFER_SIZE) % BUFFER_SIZE) {
            dataItem = m_buff[nextCursor];
            return true;
        }
        return false;
    }
    inline void stepCursor() {
        if ((m_begin - m_cursor + BUFFER_SIZE) % BUFFER_SIZE) {
            m_cursor = (m_cursor + 1) % BUFFER_SIZE;
        }
    }
    inline void stepEnd() {
        m_end = (m_end + 1) % BUFFER_SIZE;
        if (m_end == m_cursor && (m_begin - m_cursor + BUFFER_SIZE) % BUFFER_SIZE) {
            m_cursor = (m_cursor + 1) % BUFFER_SIZE;
        }
    }
private:
    unsigned int m_begin = 0;               // the newest frame data index
    unsigned int m_end = 0;                 // the oldest frame data index - 1 (data assumed invalid)
    unsigned int m_cursor = 1;              // frame data between cursor and begin(span over 1s) will be used to calculate FPS  
    LARGE_INTEGER m_frequency = { 0 };
    LARGE_INTEGER m_buff[BUFFER_SIZE] = { 0 };
};