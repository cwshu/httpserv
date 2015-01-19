# compiler related
CC = gcc
CFLAGS = -std=c99 -g
CXX = clang++
CXXFLAGS = -std=c++11 -g

# prefix, src Directory
TEST_PREFIX = ./test_build
PREFIX = ./build
TEST_SRC_DIR = ./src
PART1_SRC_DIR = ./part1
PART2_SRC_DIR = ./part2

# exe, objs without path
PART1_EXE = request_server.cgi
PART2_EXE = httpd
PART1_OBJS = request_server_cgi.o socket.o io_wrapper.o httplib.o
PART2_OBJS = httpd.o io_wrapper.o server_arch.o socket.o string_more.o

# exe, objs with path
PART1_EXE_PATH_TEST = $(addprefix $(TEST_PREFIX)/, $(PART1_EXE))
PART1_OBJS_PATH_TEST = $(addprefix $(TEST_PREFIX)/, $(PART1_OBJS))
PART2_EXE_PATH_TEST = $(addprefix $(TEST_PREFIX)/, $(PART2_EXE))
PART2_OBJS_PATH_TEST = $(addprefix $(TEST_PREFIX)/, $(PART2_OBJS))

PART1_EXE_PATH = $(addprefix $(PREFIX)/, $(PART1_EXE))
PART1_OBJS_PATH = $(addprefix $(PREFIX)/, $(PART1_OBJS))
PART2_EXE_PATH = $(addprefix $(PREFIX)/, $(PART2_EXE))
PART2_OBJS_PATH = $(addprefix $(PREFIX)/, $(PART2_OBJS))

# union of par1, 2 objs
PART1_2_OBJS_PATH_TEST = $(sort $(PART1_OBJS_PATH_TEST) $(PART2_OBJS_PATH_TEST))
PART1_2_OBJS_PATH = $(sort $(PART1_OBJS_PATH) $(PART2_OBJS_PATH))

# part1, part2 srcs
PART1_SRCS = $(patsubst %.o,%.cpp,$(PART1_OBJS))
PART2_SRCS = $(patsubst %.o,%.cpp,$(PART2_OBJS))
PART1_SRCS_PATH = $(addprefix $(PART1_SRC_DIR)/, $(PART1_SRCS))
PART2_SRCS_PATH = $(addprefix $(PART2_SRC_DIR)/, $(PART2_SRCS))

# make
MAKE = make

# platform issue
UNAME = $(shell uname)
ifeq ($(UNAME), FreeBSD)
    MAKE = gmake
endif

# release version

all: release $(PART1_EXE_PATH) $(PART2_EXE_PATH)

release:
	cp -r $(TEST_SRC_DIR)/ $(PART1_SRC_DIR)/
	cp -r $(TEST_SRC_DIR)/ $(PART2_SRC_DIR)/

rclean:
	rm -rf $(PART1_SRC_DIR) $(PART2_SRC_DIR)


clean: 
	rm -f $(PART1_EXE_PATH) $(PART1_OBJS_PATH)
	rm -f $(PART2_EXE_PATH) $(PART2_OBJS_PATH)

$(PART1_EXE_PATH): $(PART1_OBJS_PATH)
	$(CXX) -o $@ $(CXXFLAGS) $^

$(PART2_EXE_PATH): $(PART2_OBJS_PATH)
	$(CXX) -o $@ $(CXXFLAGS) $^

$(PART1_2_OBJS_PATH): $(PREFIX)/%.o: $(PART2_SRC_DIR)/%.cpp | $(PREFIX)
	$(CXX) -o $@ $(CXXFLAGS) -c $<


# test version
tall: $(PART1_EXE_PATH_TEST) $(PART2_EXE_PATH_TEST)

tclean:
	rm -f $(PART1_EXE_PATH_TEST) $(PART1_OBJS_PATH_TEST)
	rm -f $(PART2_EXE_PATH_TEST) $(PART2_OBJS_PATH_TEST)

# part1: .o => executable
$(PART1_EXE_PATH_TEST): $(PART1_OBJS_PATH_TEST)
	$(CXX) -o $@ $(CXXFLAGS) $^

# part2: .o => executable
$(PART2_EXE_PATH_TEST): $(PART2_OBJS_PATH_TEST)
	$(CXX) -o $@ $(CXXFLAGS) $^

# .cpp => .o
$(PART1_2_OBJS_PATH_TEST): $(TEST_PREFIX)/%.o: $(TEST_SRC_DIR)/%.cpp | $(TEST_PREFIX)
	$(CXX) -o $@ $(CXXFLAGS) -c $<

# make directory
$(TEST_PREFIX) $(PREFIX) $(PART1_SRC_DIR) $(PART2_SRC_DIR): 
	mkdir -p $@

.PHONY: all clean tall tclean release rclean
