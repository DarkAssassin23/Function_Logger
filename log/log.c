#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <assert.h>
// Makeing sure log dir exists
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//Global variables to keep track of log files.
static int LOG_LEVEL = OFF;
static char* log_filename = "log";
static char* log_dir_name = "logs/";
static bool write_to_file = false;
static bool write_to_console = true;
static bool free_log_filename = false;
static bool free_log_dir_name = false;

static const char* log_level_strings[] = {
    "DEFAULT",
    "OFF",
    "[DEBUG]",
    "[INFO]",
    "[WARNING]",
    "[ERROR]",
};

const char* colors[] = {
    "\x1B[0m",
    "\x1B[0m",
    "\x1B[34m",
    "\x1B[32m",
    "\x1B[33m",
    "\x1B[31m",
};

/**
* @brief Generates a timestamp in Y-m-d_HMS format.
* @return The timestamp formated as a string.
*/
static char* generate_file_timestamp(void)
{
    time_t t = time(NULL);
    struct tm *tm  = localtime(&t);

    char timestr[32];
    size_t timestamp_len = strftime(timestr, sizeof(timestr), "%Y-%m-%d_%H%M%S", tm);
    char *modified_date = (char*)malloc(sizeof(char)*(timestamp_len + 1));
    strncpy(modified_date, timestr, timestamp_len);
    modified_date[timestamp_len] = '\0';

    return modified_date;
}

/**
* @brief Generates a timestamp in m/d/y:H:M:S format.
* @return The timestamp formated as a string.
*/
static char* generate_timestamp(void)
{
    time_t t = time(NULL);
    struct tm *tm  = localtime(&t);

    char timestr[32];
    size_t timestamp_len = strftime(timestr, sizeof(timestr), "%D@%T", tm);
    char *modified_date = (char*)malloc(sizeof(char)*(timestamp_len + 1));
    strncpy(modified_date, timestr, sizeof(char)*timestamp_len);
    modified_date[timestamp_len] = '\0';

    return modified_date;
}

/**
* @brief Create a directory from a path.
* @param[in] full_path The full path of the directory path.
* @param[in] end The ending index where the subdirectory we're trying to make is.
* @return If the directory creation was successful.
*/
static bool create_dir_from_path(const char *path, int end)
{
    if(end == 0)
        return false;

    bool success = false;
    size_t sub_dir_len = end;
    char *sub_dir = (char*)malloc(sizeof(char)*(sub_dir_len + 1));
    strncpy(sub_dir, path, sizeof(char)*sub_dir_len);
    sub_dir[sub_dir_len] = '\0';

    struct stat sb;

    if (stat(sub_dir, &sb) == 0 && S_ISDIR(sb.st_mode)) 
        success = true;
    else 
        success = !(mkdir(sub_dir, 0755));

    free(sub_dir);
    return success;
}

/**
* @brief Makes a directory structure given a path.
* @param[in] path The full path to make the directory structure of.
* @return If the path was created successfully.
*/
static bool make_dir_path(char* path)
{
    bool success = true;
    char *validated_log_dir = path;
    // Make sure log directory ends in a /
    if(path[strlen(path)-1] != '/')
    {
        validated_log_dir = (char*)malloc(sizeof(char)*(strlen(path)+2));
        strncpy(validated_log_dir, path, sizeof(char)*strlen(path));
        validated_log_dir[strlen(path)] = '/';
        validated_log_dir[strlen(path)+1] = '\0';
        log_dir_name = validated_log_dir;
        free_log_dir_name = true;
    }

    int current_index = 0;
    while(validated_log_dir[current_index] != '\0')
    {
        if(validated_log_dir[current_index] == '/')
            success = create_dir_from_path(validated_log_dir, ++current_index);
        
        if(!success)
            return false;
        current_index++;
    }

    return success;
}

/**
* @brief Make sure the log directory exists.
* @param[in] log_dir Path to the log directory.
* @return If the log directory exists or was created successfully.
*/
static bool validate_log_dir(char *log_dir)
{
    struct stat sb;

    if (stat(log_dir, &sb) == 0 && S_ISDIR(sb.st_mode)) 
        return true;
    else 
        return make_dir_path(log_dir);

    return false;
}

/**
 * @brief Function to generate the name of the log file.
 * @param[in] log_dir The directory to save the log files too.
 * @param[in] prog_name The name of the program from argv[0].
 * @return The name of the log file to write to.
 */
static char* generate_log_filename(char *log_dir, const char* prog_name)
{
    if(!validate_log_dir(log_dir))
    {
        fprintf(stderr, "%sError:%s Unable to create the log directory \'%s\'\nExiting the program with status 1.\n", 
            colors[ERROR], colors[RESET], log_dir);
        log_cleanup();
        exit(1);
    }
    
    bool free_memory = false;
    char *validated_log_dir = log_dir;
    // Make sure log directory ends in a /
    if(log_dir[strlen(log_dir)-1] != '/')
    {
        validated_log_dir = (char*)malloc(sizeof(char)*(strlen(log_dir)+2));
        strncpy(validated_log_dir, log_dir, sizeof(char)*strlen(log_dir));
        validated_log_dir[strlen(log_dir)] = '/';
        validated_log_dir[strlen(log_dir)+1] = '\0';
        free_memory = true;
    }

    size_t log_dir_len = strlen(validated_log_dir);

    char *prog_name_start = strrchr(prog_name, '/');
    int prog_name_start_index = 0;

    if (prog_name_start)
        prog_name_start_index = (prog_name_start - prog_name)+1;

    size_t prog_name_len = strlen(prog_name)-prog_name_start_index;

    char *timestamp_str = generate_file_timestamp();
    size_t timestamp_str_len = strlen(timestamp_str);

    size_t filename_len = log_dir_len + prog_name_len + timestamp_str_len;
    // Adding 6 to add '_' between file and time stamp as well as '.log\0'
    char *filename = (char*)malloc((sizeof(char)*filename_len)+6); 
    strncpy(filename, validated_log_dir, sizeof(char)*log_dir_len);
    strncpy(filename+log_dir_len, &prog_name[prog_name_start_index], (sizeof(char)*prog_name_len));
    filename[log_dir_len+prog_name_len] = '_';
    strncpy(filename+(log_dir_len+prog_name_len+1), timestamp_str, sizeof(char)*timestamp_str_len);
    strncpy(filename+(filename_len+1), ".log", sizeof(char)*4); // strlen(".log) = 4 + '\0' = 5
    filename[strlen(filename)] = '\0';

    if(free_memory)
        free(validated_log_dir);
    free(timestamp_str);
    free_log_filename = true;
    return filename;
}

/**
 * @brief Function to get the current date and time and write to log file.
 * @param[in] fptr File pointer to the log file.
 * @return None.
 */
static void get_datetime(FILE *fptr) 
{
    char *modified_date = generate_timestamp();
    fprintf(fptr, "[%s]", modified_date); 
    free(modified_date);
}

/**
 * @brief Function to extract the name of the file from the provided path.
 * @param[in] path The path to, and including, the file.
 * @return The filename.
 */
static char* remove_dir_from_filename(const char* path)
{
    char *filename_start = strrchr(path, '/');
    int filename_start_index = 0;

    if (filename_start)
        filename_start_index = (filename_start - path)+1;

    size_t filename_len = strlen(path)-filename_start_index;

    char *filename = (char*)malloc(sizeof(char)*(filename_len + 1));
    strncpy(filename, path+filename_start_index, sizeof(char)*filename_len);
    filename[filename_len] = '\0';
    return filename;
}

/**
 * @brief Function to get the filename and write to log file.
 * @param[in] fptr File pointer to the log file.
 * @param[in] path Full filepath of the file.
 * @return None.
 */
static void get_filename(FILE *fptr, const char *path) 
{
    char *filename = remove_dir_from_filename(path);
    fprintf(fptr, "%s:", filename);
    free(filename);
}

/**
 * @brief Function to get the name of the function calling
 *      the logger and write to log file.
 * @param[in] fptr File pointer to the log file.
 * @param[in] function_name Name of the function calling the logger.
 * @return None.
 */
static void get_function_name(FILE *fptr, const char *function_name) 
{
    fprintf(fptr, "[%s():", function_name);
}

/**
 * @brief Function to get the name of the function calling
 *      the logger and write to log file.
 * @param[in] fptr File pointer to the log file.
 * @param[in] line The line of the file where the logger was called.
 * @return None.
 */
static void get_line_num(FILE *fptr, size_t line) 
{
    fprintf(fptr, "%zu] ", line);
}
/**
 * @brief Function to write the log level to the log file.
 * @param[in] level Log level.
 * @param[in] fptr File pointer to the log file.
 * @return None.
 */
static void log_msg(LOGGING_LEVELS level, FILE *fptr) 
{
    if(level == DEFAULT)
        return;

    if(fptr == stderr) 
        fprintf(fptr, "%s%-9s%s", colors[level], log_level_strings[level], colors[RESET]);
    else 
        fprintf(fptr, "%-9s", log_level_strings[level]);
}

void log_func(LOGGING_LEVELS level, const char *file, const size_t line, 
                const char* function_name, bool display_calling_info, const char *frmt, ...) 
{

    if(LOG_LEVEL > level && level != DEFAULT)
        return;
    
    if(level > ERROR)
    {
        char *filename = remove_dir_from_filename(file);
        fprintf(stderr, "%sError:%s Invalid logging level provided\nWhere: [%s():%s] Line: %zu\n", 
            colors[ERROR], colors[RESET], function_name, filename, line);
        free(filename);
        return;
    }

    FILE *fptr = stderr;
    if(write_to_file) 
    {
        fptr = fopen(log_filename, "a");
        if(NULL == fptr) 
        {
            fprintf(stderr, "%sError:%s File \'%s\' not existing or Permission denied\nExiting the program with status 1.\n", 
                colors[ERROR], colors[RESET], log_filename);
            log_cleanup();
            exit(1);
        }
    }

    if(display_calling_info)
        get_datetime(fptr);

    log_msg(level, fptr);

    if(display_calling_info)
    {
        get_function_name(fptr, function_name);
        get_filename(fptr, file);
        get_line_num(fptr, line);
    }
    
    char *format = strdup(frmt);
    strcat(format, "\n");
    va_list argp;
    va_start(argp, frmt);
    vfprintf(fptr, format, argp);
    va_end(argp);
    free(format);
    if(fptr != stderr) 
    {
        fclose(fptr);
        if(write_to_console)
        {
            fptr = stderr;
            if(display_calling_info)
                get_datetime(fptr);
            log_msg(level, fptr);
            if(display_calling_info)
            {
                get_function_name(fptr, function_name);
                get_filename(fptr, file);
                get_line_num(fptr, line);
            }
            char *format = strdup(frmt);
            strcat(format, "\n");
            va_list argp;
            va_start(argp, frmt);
            vfprintf(fptr, format, argp);
            va_end(argp);
            free(format);
        }
    }
}

void init_logger(LOGGING_LEVELS level, char* log_dir, char** argv, bool to_file, bool to_console) 
{
    if(LOG_LEVEL != OFF) 
    {
        fprintf(stderr, "%sError:%s Logger already initialised in the project\nExiting the program with status 1.\n", 
            colors[ERROR], colors[RESET]);
        exit(1);
    }
    assert(level > OFF && level <= ERROR);

    LOG_LEVEL = level;
    write_to_file = to_file;
    write_to_console = to_console;

    if(argv == NULL)
        write_to_file = false;

    if(write_to_file)
    {
        if(log_dir == NULL)
            log_filename = generate_log_filename(log_dir_name, argv[0]);
        else
            log_filename = generate_log_filename(log_dir, argv[0]);
    }
}

void log_cleanup()
{
    if(free_log_filename)
        free(log_filename);
    if(free_log_dir_name)
        free(log_dir_name);
}