### Parameters

ifeq ($(DEBUG),1)
    DBG_CFLAG = -O0 -g3
else
    DBG_CFLAG =
endif

# compiler related
CC  = gcc
CXX = clang++

CFLAGS   = -std=c99 $(DBG_CFLAG)
CXXFLAGS = -std=c++11 $(DBG_CFLAG)

# other tools
MAKE    = make
DOXYGEN = doxygen

UNAME = $(shell uname)
ifeq ($(UNAME), FreeBSD)
    MAKE = gmake
endif

# prefix, src Directory
PREFIX = ./build
SRC_DIR = ./src

### Define source files

SRCS = httpd.cpp httplib.cpp server_arch.cpp socket.cpp utils.cpp
SRCS_PATH = $(addprefix $(SRC_DIR)/, $(SRCS))

OBJS = $(patsubst %.cpp,%.o,$(SRCS))
BUILD_OBJS = $(addprefix $(PREFIX)/, $(OBJS))

PROGRAM = httpd
BUILD_PROGRAM = $(addprefix $(PREFIX)/, $(PROGRAM))

### Rules

all: $(BUILD_PROGRAM)

clean: 
	rm -rf $(PREFIX)

$(BUILD_PROGRAM): $(BUILD_OBJS)
	$(CXX) -o $@ $(CXXFLAGS) $^

$(BUILD_OBJS): $(PREFIX)/%.o: $(SRC_DIR)/%.cpp | $(PREFIX)
	$(CXX) -o $@ $(CXXFLAGS) -c $<

# make directory
$(PREFIX):
	mkdir -p $@

doc:
	$(DOXYGEN) Doxyfile

.PHONY: all clean doc
