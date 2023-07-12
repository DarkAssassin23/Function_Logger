#ifndef LOG_H
#define LOG_H

#include <stdlib.h>
#include <stdbool.h>
#define RESET 0

typedef enum {
    DEFAULT,
    OFF,
    DEBUG,
    INFO,
    WARNING,
    ERROR
} LOGGING_LEVELS;

/**
 * @brief Function to initialize the logger.
 * @param[in] level Log level.
 * @param[in] log_dir Log directory to save logs to
 * @param[in] argv argv variable from main.
 * @param[in] to_file Boolean variable to allow writing to file.
 * @param[in] to_console Boolean variable to allow writing to stderr stream.
 * @return None.
 */
void init_logger(LOGGING_LEVELS, char*, char **, bool, bool);

/**
 * @brief Function to write the message to the log file.
 * @param[in] level Log level.
 * @param[in] filename Name of the file calling log_func
 * @param[in] line Line in the file where log_func was called
 * @param[in] function_name The name of the function that called log_func
 * @param[in] display_calling_info Whether or not to display the file, line and function in the log
 * @param[in] newline Whether or not to auto print a new line character
 * @param[in] frmt Format message.
 * @param[in] ... Variable arguments.
 * @return None.
 */
void log_func(LOGGING_LEVELS level, const char *filename, 
                const size_t line, const char* function_name, 
                bool display_calling_info, bool newline, const char *frmt, ...);

/**
* @brief Clean up the logger upon exit.
* @details Cleans up the logger by freeing any necessary memory
*       and resetting values back to their defaults
* @return None.
*/
void log_cleanup(void);

#define LOG(...) log_func(DEFAULT, __FILE__, __LINE__, __func__, false, false, __VA_ARGS__)
#define LOGL(level, ...) log_func(level, __FILE__, __LINE__, __func__, false, false, __VA_ARGS__)
#define LOGF(...) log_func(DEFAULT, __FILE__, __LINE__, __func__, true, false, __VA_ARGS__)
#define LOGFL(level, ...) log_func(level, __FILE__, __LINE__, __func__, true, false, __VA_ARGS__)

#define NLOG(...) log_func(DEFAULT, __FILE__, __LINE__, __func__, false, true, __VA_ARGS__)
#define NLOGL(level, ...) log_func(level, __FILE__, __LINE__, __func__, false, true, __VA_ARGS__)
#define NLOGF(...) log_func(DEFAULT, __FILE__, __LINE__, __func__, true, true, __VA_ARGS__)
#define NLOGFL(level, ...) log_func(level, __FILE__, __LINE__, __func__, true, true, __VA_ARGS__)

#endif