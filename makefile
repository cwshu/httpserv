### Parameters

ifeq ($(DEBUG),1)
    DBG_CFLAGS = -O0 -g3
else
    DBG_CFLAGS =
endif

# compiler related
CC  = clang
CXX = clang++
AS  = clang
LD  = clang++
AR  = ar
OBJCOPY = objcopy
OBJDUMP = objdump

CFLAGS   = -std=c99 $(DBG_CFLAGS)
CXXFLAGS = -std=c++11 $(DBG_CFLAGS)

# other tools
MAKE    = make
DOXYGEN = doxygen

UNAME = $(shell uname)
ifeq ($(UNAME), FreeBSD)
    MAKE = gmake
endif

# prefix, src Directory
PREFIX  = ./build
SRC_DIR = ./src

### Verbosity control. Use 'make V=1' to get verbose builds.

ifeq ($(V),1)
TRACE_CC  = 
TRACE_CXX =
TRACE_LD  =
TRACE_AR  =
TRACE_AS  =
Q=
else
TRACE_CC  = @echo "  CC       " $<
TRACE_CXX = @echo "  CXX      " $<
TRACE_LD  = @echo "  LD       " $@
TRACE_AR  = @echo "  AR       " $@
TRACE_AS  = @echo "  AS       " $<
Q=@
endif

### Define source files

SRCS = httpd.cpp httplib.cpp server_arch.cpp socket.cpp utils.cpp

OBJS = $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${SRCS})))
BUILD_OBJS = $(addprefix $(PREFIX)/, $(OBJS))

PROGRAM = httpd
BUILD_PROGRAM = $(addprefix $(PREFIX)/, $(PROGRAM))

### Rules

all: $(BUILD_PROGRAM)

clean: 
	rm -rf $(PREFIX)

$(BUILD_PROGRAM): $(BUILD_OBJS)
	$(TRACE_LD)
	$(Q)$(LD) -o $@ $(CXXFLAGS) $^
	$(OBJDUMP) -d $(BUILD_PROGRAM) > $(BUILD_PROGRAM).asm

$(PREFIX)/%.o: $(SRC_DIR)/%.cpp | $(PREFIX)
	$(TRACE_CXX)
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c $<

# make directory
$(PREFIX):
	mkdir -p $@

doc:
	$(DOXYGEN) Doxyfile

.PHONY: all clean doc
