# Define the compiler and the linker. The linker must be defined since
# the implicit rule for linking uses CC as the linker. g++ can be
# changed to clang++.
CXX = g++
CC  = $(CXX)

# Generate dependencies in *.d files
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

# Define preprocessor, compiler, and linker flags. Uncomment the # lines
# if you use clang++ and wish to use libc++ instead of GNU's libstdc++.
# -g is for debugging.
CPPFLAGS =  -Iinclude
CXXFLAGS =  -O2 -Wall -Wextra -pedantic-errors -Wold-style-cast 
CXXFLAGS += -std=c++14
CXXFLAGS += -g
CXXFLAGS += $(DEPFLAGS)
LDFLAGS =   -g -Llib
#CPPFLAGS += -stdlib=libc++
#CXXFLAGS += -stdlib=libc++
#LDFLAGS +=  -stdlib=libc++

# Targets


# Default target
all: src/myserver src/client

# Object files compilation rules
src/connection.o: src/connection.cc include/connection.h
	$(CXX) $(CXXFLAGS) -Iinclude -c -o src/connection.o src/connection.cc

src/server.o: src/server.cc include/server.h
	$(CXX) $(CXXFLAGS) -Iinclude -c -o src/server.o src/server.cc

src/client.o: src/client.cc
	$(CXX) $(CXXFLAGS) -Iinclude -c -o src/client.o src/client.cc

src/myserver.o: src/myserver.cc
	$(CXX) $(CXXFLAGS) -Iinclude -c -o src/myserver.o src/myserver.cc


src/message_handler.o: src/message_handler.cc include/message_handler.h include/protocol.h
	$(CXX) $(CXXFLAGS) -Iinclude -c -o src/message_handler.o src/message_handler.cc

src/client: src/client.o src/connection.o src/message_handler.o
	$(CXX) $(CXXFLAGS) -o client src/client.o src/connection.o src/message_handler.o

src/myserver: src/connection.o src/server.o src/myserver.o src/message_handler.o src/memory_database.o
	$(CXX) $(CXXFLAGS) -o myserver src/connection.o src/server.o src/myserver.o src/message_handler.o src/memory_database.o

src/memory_database.o: src/memory_database.cc include/memory_database.h include/database.h
	$(CXX) $(CXXFLAGS) -Iinclude -c -o src/memory_database.o src/memory_database.cc

# Phony targets
.PHONY: all clean distclean

SRC = $(wildcard src/*.cc)

# Standard clean
clean:
	-rm $(SRC:.cc=.o) $(PROGS)

distclean: clean
	-rm lib/libclientserver.a
	-rmdir lib
	-rm $(SRC:.cc=.d) 
	make -C example distclean


# Include the *.d files
-include $(SRC:.cc=.d)
