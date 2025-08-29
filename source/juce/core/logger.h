#pragma once

#define log_info(...) juce::logger::get_instance()->log(juce::logger::info, __VA_ARGS__)
#define log_debug(...) juce::logger::get_instance()->log(juce::logger::debug, __VA_ARGS__)
#define log_warn(...) juce::logger::get_instance()->log(juce::logger::warn, __VA_ARGS__)
#define log_error(...) juce::logger::get_instance()->log(juce::logger::error, __VA_ARGS__)

namespace juce
{
class logger
{
public:
    enum level
    {
        info,
        debug,
        warn,
        error
    };
    inline static logger* get_instance()
    {
        static logger instance;
        return &instance;
    }
    void log(level level, const char* code, ...);

private:
    logger();
    ~logger() = default;

    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;
};
} // namespace juce
