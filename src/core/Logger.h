// ============================================================================
//  Logger.h — Простой уровневый логгер VARSYS поверх Serial
// ============================================================================
#pragma once
#include <Arduino.h>
#include "varsys_config.h"

enum class LogLevel : uint8_t { ERROR = 0, WARN = 1, INFO = 2, DEBUG = 3 };

class Logger {
public:
    static void begin(unsigned long baud = VARSYS_LOG_BAUD);
    static void setLevel(LogLevel level) { _level = level; }

    static void log(LogLevel level, const char* tag, const char* fmt, ...);

private:
    static LogLevel _level;
    static const char* levelStr(LogLevel level);
};

// Удобные макросы. Тег обычно — имя модуля.
#if VARSYS_LOG_ENABLED
  #define LOGE(tag, ...) Logger::log(LogLevel::ERROR, tag, __VA_ARGS__)
  #define LOGW(tag, ...) Logger::log(LogLevel::WARN,  tag, __VA_ARGS__)
  #define LOGI(tag, ...) Logger::log(LogLevel::INFO,  tag, __VA_ARGS__)
  #define LOGD(tag, ...) Logger::log(LogLevel::DEBUG, tag, __VA_ARGS__)
#else
  #define LOGE(tag, ...) ((void)0)
  #define LOGW(tag, ...) ((void)0)
  #define LOGI(tag, ...) ((void)0)
  #define LOGD(tag, ...) ((void)0)
#endif
