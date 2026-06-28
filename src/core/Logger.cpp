#include "Logger.h"
#include <stdarg.h>

LogLevel Logger::_level = LogLevel::DEBUG;

void Logger::begin(unsigned long baud) {
    Serial.begin(baud);
    uint32_t start = millis();
    while (!Serial && (millis() - start) < 1500) { delay(10); }
}

const char* Logger::levelStr(LogLevel level) {
    switch (level) {
        case LogLevel::ERROR: return "E";
        case LogLevel::WARN:  return "W";
        case LogLevel::INFO:  return "I";
        case LogLevel::DEBUG: return "D";
        default:              return "?";
    }
}

void Logger::log(LogLevel level, const char* tag, const char* fmt, ...) {
    if (level > _level) return;

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Serial.printf("[%8lu][%s][%s] %s\n",
                  (unsigned long)millis(), levelStr(level), tag, buf);
}
