#Compiler and Linker
CC          := g++

#The Target Binary Program
TARGET      := dotest
EXAMPLE 		:= example

# what platform we are running on
PLATFORM    := $(shell uname -so | sed -r s'/[^a-zA-Z0-9]/_/g')

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
INCDIR      := src
TESTDIR     := test
TESTINCDIR  := test
EXDIR       := examples
EXINCDIR    := examples
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res
SRCEXT      := cpp
OBJEXT      := obj
LIBDIR      := test/lib/$(PLATFORM)


#Flags, Libraries and Includes
#CFLAGS      := -ggdb -static-libgcc -static-libstdc++ -fopenmp -Wall -O3 -g -std=gnu++11 -DWITHIN_UNITTEST
CFLAGS      := -ggdb -Wall -O3 -g -std=gnu++11 -DWITHIN_UNITTEST
#LIB         := -fopenmp -lm -L$(LIBDIR) -lgtest_main -lgtest -pthread
LIB         :=  -lm -L$(LIBDIR) -lgtest_main -lgtest -pthread
EXLIB       := -lm -L$(LIBDIR) -lgtest_main -lgtest -pthread
#GOOGLETEST_DIR := ../GitHub/googletest/googletest
INC         := -I$(INCDIR) -I$(TESTINCDIR) -Itest/include -I/usr/local/include #-I$(GOOGLETEST_DIR)/include
INCDEP      := -I$(INCDIR) -I$(INCDIR2) -I$(TESTINCDIR) -I$(GOOGLETEST_DIR)/include

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
#SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
SOURCES      := $(SRCDIR)/wifi_utils.cpp \
								$(SRCDIR)/RobustWiFiServer.cpp

TESTSOURCES  := $(TESTDIR)/test.cpp \
								$(TESTDIR)/mock_Arduino.cpp \
								$(TESTDIR)/mock_wifi.cpp
								
EXSOURCES    :=

OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT))) \
							 $(patsubst $(TESTDIR)/%,$(BUILDDIR)/%,$(TESTSOURCES:.$(SRCEXT)=.$(OBJEXT)))

EXOBJECTS   := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT))) \
							 $(patsubst $(EXDIR)/%,$(BUILDDIR)/%,$(EXSOURCES:.$(SRCEXT)=.$(OBJEXT)))

#$(info $$EXOBJECTS is [${EXOBJECTS}])

#Defauilt Make
all: $(TARGET) #$(EXAMPLE)

#Remake
remake: cleaner all

#Copy Resources from Resources Directory to Target Directory
resources: directories
	@cp $(RESDIR)/* $(TARGETDIR)/

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
	@$(RM) -rf $(BUILDDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
#-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))
#-include $(EXOBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

$(EXAMPLE): $(EXOBJECTS)
	$(CC) -o $(TARGETDIR)/$(EXAMPLE) $^ $(EXLIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/%.$(OBJEXT): $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/%.$(OBJEXT): $(EXDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

#print out variable
print-%  : ; @echo $* = $($*)

#Non-File Targets
.PHONY: all remake clean cleaner resources
