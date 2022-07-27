BUILD_DIR=build
CXX = g++
CXXFLAGS = -std=c++11 -g -fPIC

ROOT_DIR := $(shell git rev-parse --show-toplevel)
-include $(ROOT_DIR)/config/settings
INCLUDES = -Iinclude/jericho -Icurl -Icomet/include
LIBDIRS = -Lbuild/lib
LIBS = -lpthread -lssl -lcrypto -lprizm
EXE = ${BUILD_DIR}/bin/jericho
SHARED = ${BUILD_DIR}/lib/libjericho.so

SOURCES = $(filter-out src/ssh/*.cc, $(wildcard src/*.cc) $(wildcard src/*/*.cc))
SOURCES := $(filter-out src/ssh/*.cc, $(SOURCES))
OBJFILES_CC = $(notdir $(SOURCES:.cc=.o))
OBJFILES_ALL = $(notdir $(OBJFILES_CC:.tpp=.o))
OBJFILES = $(notdir $(OBJFILES_ALL:.cpp=.o))
OBJDIR = $(BUILD_DIR)/obj
OBJECTS = $(addprefix $(OBJDIR)/, $(OBJFILES))
-include $(DEP_DIR)/env

all: $(EXE)
tpool: thread_pool
lib: $(SHARED)

$(SHARED): $(BUILD_DIR) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -shared $(LIBDIRS) $(LIBS) $(OBJECTS) -o $@

$(EXE): $(BUILD_DIR) $(SHARED)
	$(CXX) $(CXXFLAGS) -Wl,-rpath,$(BUILD_DIR)/lib $(LIBDIRS) $(INCLUDES) -ljericho -o jericho src/main.cc $(LIBS) $(FLAGS)

# $(SHARED): $(BUILD_DIR) $(OBJECTS)
# 	@echo "Building shared library..."
# 	$(CXX) $(CXXFLAGS) -shared $$(LIBS) $(OBJECTS) -o $@
# 	@echo "\033[1;32mPrizm library built.\033[0m"

thread_pool: $(BUILD_DIR) $(SHARED)
	$(CXX) $(CXXFLAGS) -Wl,-rpath,$(BUILD_DIR)/lib $(LIBDIRS) -L$(BUILD_DIR)/lib $(INCLUDES) $(filter-out src/ssh/*.cc, $(SOURCES)) $(LIBS) -o $@ $(FLAGS)

$(OBJDIR)/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: src/*/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p ${BUILD_DIR}/obj
	mkdir -p ${BUILD_DIR}/lib
	mkdir -p ${BUILD_DIR}/bin

clean:
	rm -f ${BUILD_DIR}/lib/*
	rm -f ${BUILD_DIR}/obj/*
	rm -f ${BUILD_DIR}/bin/*