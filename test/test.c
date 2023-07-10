// Test to check that the logging functions work properly.

#include "log/log.h"
#include <stdio.h>
#include <string.h>

void test_init(char **argv) 
{
    // Will write to [name of the program]_[timestamp].log in the 
    // 'logs/' directory as well as print the output to the screen
    init_logger(DEBUG, "logs", argv, true, true); 

    // Will write to [name of the program]_[timestamp].log 
    // in the 'custom_dir/logs/' directory
    // but not print anything to the screen
    //init_logger(DEBUG, "custom_dir/log/", argv, true, false); 

    // Will only output to stderr console.
    //init_logger(DEBUG, NULL, NULL, false, true);

    // Will throw an error because the log level is too high
    //init_logger(10, NULL, NULL, false, true);

    // Will throw an error because the log level is too low
    //init_logger(-3, NULL, NULL, false, false);
}

int main(int argc, char **argv) 
{
    test_init(argv);

    // This will fail since we already called init
    // init_logger(DEBUG, "logs/", argv, false, false);
    LOG("Log from LOG()");
    LOGL(INFO, "Log from LOGL() at level %d", INFO);
    LOGL(10, "This will not work");
    LOGF("Log from LOGF()");
    LOGFL(DEBUG, "Log from LOGFL() at level %d", DEBUG);
    LOGFL(WARNING, "Log from LOGFL() at level %d", WARNING);
    log_cleanup();
    return 0;
}