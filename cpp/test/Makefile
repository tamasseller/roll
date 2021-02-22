OUTPUT = rpc-cpp-test

SOURCES += main.cpp
# SOURCES += TestSignatureGeneration.cpp
# SOURCES += TestSerDes.cpp
# SOURCES += TestCore.cpp
SOURCES += TestEndpoint.cpp
SOURCES += pet/ubiquitous/PrintfWriter.cpp

INCLUDE_DIRS += .
INCLUDE_DIRS += ..

CXXFLAGS += -O0 -g3
CXXFLAGS += --std=c++17
CXXFLAGS += -fno-inline
CXXFLAGS += -fmax-errors=5
CXXFLAGS += -rdynamic
CXXFLAGS += --coverage

LDFLAGS += -rdynamic
LIBS += gcov

LD=$(CXX)

COVROOT = ..

include pet/mod.mk
include ultimate-makefile/Makefile.ultimate