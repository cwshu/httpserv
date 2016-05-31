## Parameters

# compiler related
CC = gcc -g
CFLAGS = -std=c99 -g
CXX = clang++
CXXFLAGS = -std=c++11 -g

# prefix, src Directory
PREFIX = ./build
SRC_DIR = ./src

# exe, objs without path
EXE = httpd
OBJS = httpd.o io_wrapper.o server_arch.o socket.o string_more.o httplib.o

# exe, objs with path
EXE_PATH = $(addprefix $(PREFIX)/, $(EXE))
OBJS_PATH = $(addprefix $(PREFIX)/, $(OBJS))

# srcs
SRCS = $(patsubst %.o,%.cpp,$(OBJS))
SRCS_PATH = $(addprefix $(SRC_DIR)/, $(SRCS))

# make
MAKE = make
# platform issue
UNAME = $(shell uname)
ifeq ($(UNAME), FreeBSD)
    MAKE = gmake
endif

## Rules

all: $(EXE_PATH)

clean: 
	rm -f $(EXE_PATH) $(OBJS_PATH)

$(EXE_PATH): $(OBJS_PATH)
	$(CXX) -o $@ $(CXXFLAGS) $^

$(OBJS_PATH): $(PREFIX)/%.o: $(SRC_DIR)/%.cpp | $(PREFIX)
	$(CXX) -o $@ $(CXXFLAGS) -c $<

# make directory
$(PREFIX):
	mkdir -p $@

.PHONY: all clean 
