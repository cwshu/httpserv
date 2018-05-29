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

### Verbosity control. Use 'make V=1' to get verbose builds.

ifeq ($(V),1)
TRACE_CC  = 
TRACE_CXX =
TRACE_LD  =
Q=
else
TRACE_CC  = @echo "  CC       " $<
TRACE_CXX = @echo "  CXX      " $<
TRACE_LD  = @echo "  LD       " $@
Q=@
endif

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
	$(TRACE_LD)
	$(Q)$(CXX) -o $@ $(CXXFLAGS) $^

$(BUILD_OBJS): $(PREFIX)/%.o: $(SRC_DIR)/%.cpp | $(PREFIX)
	$(TRACE_CXX)
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c $<

# make directory
$(PREFIX):
	mkdir -p $@

doc:
	$(DOXYGEN) Doxyfile

.PHONY: all clean doc
