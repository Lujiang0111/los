VERSION_X=1
VERSION_Y=0
VERSION_Z=0
PROJECT=los

ifneq (,$(wildcard ../../../../../../../../compile.dep))
	include ../../../../../../../../compile.dep
else
	LINK_RELEASE_WL_ADD=,-z,relro,-z,now,-z,noexecstack
	COMPILE_RELEASE_ITEM=-fstack-protector-all -fPIC -g
endif

#prebuild depends libs
#index 0 is project name
#index 1 is version
#index 2 is copy include flag
#index 3 is copy lib flag
#index 4 is whether exactly match version or not
PREBUILD_BASELIB_PATH=../../../../../../../../Versions/Baselib
PREBUILD_BASELIBS="fmt 9.0.0 true true false"
POSTBUILD_BASELIB_PATH=../../../../../../../../Versions/Baselib

INCLUDE=-I../../../../include \
-I../../../../internal \
-I../../../../deps/include \
-I../../../../deps/include/fmt

WARNINGS=-Wall -Wno-unused-function
LIBS=-L../../../../deps/lib -Wl,-rpath-link=../../../../deps/lib -lpthread -lfmt
DEFINES=

CC:=gcc
CXX:=g++
VERSION=$(VERSION_X).$(VERSION_Y).$(VERSION_Z)
REAL_NAME=lib$(PROJECT).so.$(VERSION)
SONAME=lib$(PROJECT).so.$(VERSION_X)
TARGET=../../../../bin/$(REAL_NAME)
TARGET_RELEASE=../../../../bin/release/$(REAL_NAME)
SRC_DIRS=../../../../src
CSRC=$(shell find $(SRC_DIRS) -name "*.c")
COBJ:=$(CSRC:.c=.o)
CPPSRC=$(shell find $(SRC_DIRS) -name "*.cpp")
CPPOBJ:=$(CPPSRC:.cpp=.o)
CCSRC=$(shell find $(SRC_DIRS) -name "*.cc")
CCOBJ:=$(CCSRC:.cc=.o)

.PHONY: all clean

all: postbuild

release: postbuild_release

prebuild:
	rm -rf ../../../../deps
	bash prebuild.sh $(PREBUILD_BASELIB_PATH) $(PREBUILD_BASELIBS)

prebuild_release:
	rm -rf ../../../../deps
	bash prebuild.sh $(PREBUILD_BASELIB_PATH) $(PREBUILD_BASELIBS) release

mainbuild: prebuild
	@$(MAKE) --no-print-directory $(TARGET)

mainbuild_release: prebuild_release
	@$(MAKE) --no-print-directory $(TARGET_RELEASE)

postbuild: mainbuild
	bash postbuild.sh $(POSTBUILD_BASELIB_PATH) $(PROJECT) $(VERSION) $(TARGET)

postbuild_release: mainbuild_release
	bash postbuild.sh $(POSTBUILD_BASELIB_PATH) $(PROJECT) $(VERSION) $(TARGET_RELEASE) release

$(TARGET): $(COBJ) $(CPPOBJ) $(CCOBJ)
	$(CXX) -shared -Wl,-soname,$(SONAME) $(WARNINGS) -g -o $(TARGET) $(COBJ) $(CPPOBJ) $(CCOBJ) $(LIBS)

$(TARGET_RELEASE): $(COBJ) $(CPPOBJ) $(CCOBJ)
	$(CXX) -shared -Wl,-soname,$(SONAME)$(LINK_RELEASE_WL_ADD) $(WARNINGS) -g -o $(TARGET_RELEASE) $(COBJ) $(CPPOBJ) $(CCOBJ) $(LIBS)
	strip -s $(TARGET_RELEASE)

%.o: %.c
	$(CC) $(DEFINES) $(WARNINGS) $(COMPILE_RELEASE_ITEM) $(INCLUDE) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(DEFINES) $(WARNINGS) $(COMPILE_RELEASE_ITEM) $(INCLUDE) -o $@ -c $< -std=c++11

%.o: %.cc
	$(CXX) $(DEFINES) $(WARNINGS) $(COMPILE_RELEASE_ITEM) $(INCLUDE) -o $@ -c $< -std=c++11

clean:
	-rm -f $(COBJ) $(CPPOBJ) $(CCOBJ) $(TARGET) $(TARGET_RELEASE)
