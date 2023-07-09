TARGET = testlogger

CC = gcc
LIBSDIR = libs
OBJSDIR = obj
LOGSDIR = logs

ifeq ($(DEBUG), yes)
	LIBDIR = libs/debug
	STATIC_LIBNAME = libfunclogd.a
	ifeq ($(OS),Windows_NT)
		SHARED_LIBNAME = libfunclogd.dll
	else	
		SHARED_LIBNAME = libfunclogd.so
	endif
	LIBPOSTFIX = d
	CFLAGS = -g -Wall

	OBJDIR = obj/debug
else
	LIBDIR = libs/release
	STATIC_LIBNAME = libfunclog.a
	ifeq ($(OS),Windows_NT)
		SHARED_LIBNAME = libfunclog.dll
	else	
		SHARED_LIBNAME = libfunclog.so
	endif
	OBJDIR = obj/release
	CFLAGS = -Os
endif

STATIC_LIB_TARGET = $(LIBDIR)/static/$(STATIC_LIBNAME)
SHARED_LIB_TARGET = $(LIBDIR)/shared/$(SHARED_LIBNAME)
STATIC_OBJDIR = $(OBJDIR)/log/static
SHARED_OBJDIR = $(OBJDIR)/log/shared

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

$(SHARED_LIB_TARGET): $(SHARED_LIBOBJS)
	@mkdir -p $(@D)
	$(CC) -shared $^ -o $@

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
