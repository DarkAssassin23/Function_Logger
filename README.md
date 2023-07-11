# Function Logger

## Table of Contents
 * [About](#about) 
 * [Building from source](#building-from-source)
 * [Linking](#linking)
 * [Usage](#usage)

----------
## About
A cross-platform library, written in C, to log information from the runtime
of a program and output it to a file and/or the console.

----------

## Building from source

To build the library from source, simply open your terminal/command prompt 
and type one of the following:
```bash
# Build both the static and shared library
make

# Build just the static library
make staticlib

# Build just the shared library
make sharedlib
```

You can also build the provided test program by typing the following:
```bash
# Build the test program linking against the shared library
make test-shared

# Build the test program linking against the static library
make test-static
```

The libraries also have debug variants that can be built by
appending `DEBUG=yes` to your make command
```bash
# Builds both the static and shared debug versions of the library
make DEBUG=yes 

# Builds the test program, linking against the static library, in
# debug mode, using the debug library
make test-static DEBUG=yes
```

----------
## Linking
In order to properly link, you have to add `-lfunclog` to your linked 
libraries as well as the path it is located in. Here is the example
from the provided test program:
```bash
# Linking against the static library
LIBS = -L 'libs/release/static -lfunclog'

# Linking against the shared library
LIBS = -L 'libs/release/shared -lfunclog'
```
### Linking against the shared library
When linking with the shared library on macOS and GNU/Linux, 
it is best to also set an `rpath` to point to the location of the 
shared library by adding an `LDFLAG`. Again, here is the example
from the provided test program:
```bash
LDFLAGS = -Wl,-rpath 'libs/release/shared'
```
Doing this will ensure your program can actually find the library 
when you try to run it.

On Windows, make sure the `dll` is in the same directory as your 
executable.

----------
## Usage
Usage of the library has three components, linking (which we discussed
above), including, and calling the functions

### Including
First, ensure you are including the header file `log.h` in your program.
```c
// include in c
#include "log.h"

// include in c++
extern "C"{
    #include "log.h" 
}
```
You also need to ensure the directory where you put the `log.h` file is 
included in your project. For example, if you have a `headers/` folder 
where you added `log.h`, you would need `-I headers/` in your makefile.

### Using in your program
The library includes four functions to allow you to log your information.
All of which function just like `printf()` in terms of syntax.
The only difference is in the `LOGF()/NLOGF()` and 
`LOGFL()/NLOGFL()` functions,  which require you to add the level 
of logging you would like.

```c
// Don't auto print a new line at the end
#define LOG(...)
#define LOGL(level, ...)
#define LOGF(...)
#define LOGFL(level, ...)

// Do auto print a new line at the end
#define NLOG(...)
#define NLOGL(level, ...)
#define NLOGF(...)
#define NLOGFL(level, ...)
```

While not required, it is best practice to initialize the logger at the
beginning of your program with `init_logger` and call `log_cleanup` at 
the end to ensure all allocated memory is released. If you wish to log 
output to a file, calling `init_logger` is required.

### Example code
```c
#include <stdio.h>
#include <string.h>
#include "log.h"

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        LOGFL(ERROR, "%s takes one argument\n", argv[0]);
        LOG("Usage: %s [name]\n");
        return -1;
    }

    init_logger(INFO, "logs", argv, true, true);
    NLOG("Hello %s!", argv[1]);
    log_cleanup();
    
    return 0;
}
```
The above program initializes the logger, so only messages of debug level
`INFO` or higher will be logged. The next argument tells the logger to save
logs to a directory called `logs/`. Passing in `argv` adds the name of the 
program to the log file name, along with a timestamp as to when the file was
generated. Lastly, the first `true` tells the logger to save the data to a
file, and the second `true` tells the logger to also write the data to the
console.

Assuming you called the program `example.c` and had `log.h` and that 
static version of the library in the same directory,
it could be compiled with the following command:
```bash
gcc example.c -o example -L ./ -lfunclog
```

### Example output
Below is some example output from the logger:
```
Log from LOG()
[INFO]    Log from LOGL() at level 3
[07/10/23@21:25:05][main():test.c:38] Log from LOGF()
[07/10/23@21:25:05][main():test.c:39] [DEBUG]   Log from LOGFL() at level 2
[07/10/23@21:25:05][main():test.c:40] [WARNING] Log from LOGFL() at level 4
```
