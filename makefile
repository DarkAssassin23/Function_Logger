TARGET = testlogger

CC = gcc
LIBSDIR = libs
OBJSDIR = obj
LOGSDIR = logs

ifeq ($(OS),Windows_NT)
	SYSNAME = Windows
else
	UNAMEOS = $(shell uname -s)
	ifeq ($(UNAMEOS),Darwin)
		SYSNAME = macOS
	endif
	ifeq ($(UNAMEOS),Linux)
		SYSNAME = Linux
	endif
endif

BASE_LIB_NAME = libfunclog
ifeq ($(DEBUG), yes)
	LIBDIR = libs/debug
	STATIC_LIBNAME = $(BASE_LIB_NAME)d.a
	ifeq ($(SYSNAME),Windows)
		SHARED_LIBNAME = $(BASE_LIB_NAME)d.dll
	else ifeq ($(SYSNAME),macOS)
		SHARED_LIBNAME = $(BASE_LIB_NAME)d.dylib
	else	
		SHARED_LIBNAME = $(BASE_LIB_NAME)d.so
	endif
	LIBPOSTFIX = d
	CFLAGS = -g -Wall

	OBJDIR = obj/debug
else
	LIBDIR = libs/release
	STATIC_LIBNAME = $(BASE_LIB_NAME).a
	ifeq ($(OS),Windows_NT)
		SHARED_LIBNAME = $(BASE_LIB_NAME).dll
	else ifeq ($(SYSNAME),macOS)
		SHARED_LIBNAME = $(BASE_LIB_NAME).dylib
	else	
		SHARED_LIBNAME = $(BASE_LIB_NAME).so
	endif
	OBJDIR = obj/release
	CFLAGS = -Os
endif

STATIC_LIB_TARGET = $(LIBDIR)/static/$(STATIC_LIBNAME)
SHARED_LIB_TARGET = $(LIBDIR)/shared/$(SHARED_LIBNAME)
STATIC_OBJDIR = $(OBJDIR)/log/static
SHARED_OBJDIR = $(OBJDIR)/log/shared

ifeq ($(SYSNAME),Windows)
	RCS = $(wildcard version/*.rc)
	RES = $(patsubst %.rc, $(OBJDIR)/%.res, $(RCS))
	SHARED_LIB_FLAGS = -shared
else ifeq ($(SYSNAME),macOS)
	SHARED_LIB_FLAGS =  -install_name "@rpath/$(SHARED_LIBNAME)" \
		-dynamiclib -current_version 1.0 -compatibility_version 1.0 
else
	LINUX_SONAME = $(BASE_LIB_NAME).1.so
	SHARED_LIB_FLAGS = -shared -Wl,-soname,$(LINUX_SONAME)
endif

default: all

all: staticlib sharedlib

test-static: LIBS = -L $(LIBDIR)/static -lfunclog$(LIBPOSTFIX)
test-static: staticlib $(TARGET)

test-shared: LDFLAGS = -Wl,-rpath '$(LIBDIR)/shared'
test-shared: LIBS = -L $(LIBDIR)/shared -lfunclog$(LIBPOSTFIX)
test-shared: sharedlib $(TARGET)
test-shared:
ifeq ($(OS),Windows_NT)
	@mv $(SHARED_LIB_TARGET) ./$(SHARED_LIBNAME)
endif

staticlib: $(STATIC_LIB_TARGET)

sharedlib: CFLAGS += -fPIC
sharedlib: $(SHARED_LIB_TARGET)

INCLUDES = -I .

TESTOBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(wildcard test/*.c))
STATIC_LIBOBJS = $(patsubst log/%.c, $(STATIC_OBJDIR)/%.o, $(wildcard log/*.c))
SHARED_LIBOBJS = $(patsubst log/%.c, $(SHARED_OBJDIR)/%.o, $(wildcard log/*.c))

$(TARGET): $(TESTOBJS) 
	$(CC) $(TESTOBJS) -o $@ $(LIBS) $(LDFLAGS)

$(STATIC_LIB_TARGET): $(STATIC_LIBOBJS) 
	@mkdir -p $(@D)
	ar rcs $@ $^

$(SHARED_LIB_TARGET): $(SHARED_LIBOBJS) $(RES)
	@mkdir -p $(@D)
	$(CC) $(SHARED_LIB_FLAGS) $(RES) $(SHARED_LIBOBJS) -o $@
ifeq ($(SYSNAME),Linux)
	@cd $(LIBDIR)/shared/ && ln -s $(SHARED_LIBNAME) $(LINUX_SONAME)
endif
# ifeq ($(SYSNAME),macOS)
# 	$(CC) -install_name "@rpath/$(SHARED_LIBNAME)" -dynamiclib $^ -o $@
# else
# 	$(CC) -shared $^ -o $@
# endif

$(OBJDIR)/%.res: %.rc
ifeq ($(SYSNAME),Windows)
	@mkdir -p $(@D)
	windres $< -O coff -o $@
endif

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(STATIC_OBJDIR)/%.o: log/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OBJDIR)/%.o: log/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

cleanwin:
ifeq ($(OS),Windows_NT)
	$(RM) -r *.dll
endif

cleanlibs: cleanwin
	$(RM) -r $(OBJSDIR)/release/log $(OBJSDIR)/debug/log $(LIBSDIR)

cleantest: cleanwin
	$(RM) -r $(OBJSDIR)/release/test $(OBJSDIR)/debug/test $(TARGET) $(LOGSDIR)

clean: cleanwin
	$(RM) -r $(OBJSDIR) $(TARGET) $(LIBSDIR) $(LOGSDIR)
