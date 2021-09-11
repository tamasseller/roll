OUTPUT = rpc-tool
all: $(OUTPUT)

OBJDIR = .o

SOURCES += main.cpp
SOURCES += Dump.cpp
SOURCES += Serialize.cpp
SOURCES += CodeGen.cpp

SOURCES += ast/ContractParser.cpp
SOURCES += ast/ContractFormatter.cpp
SOURCES += ast/ContractTextCodec.cpp

SOURCES += gen/Generator.cpp
SOURCES += gen/cpp/Cpp.cpp
SOURCES += gen/cpp/CppCommon.cpp
SOURCES += gen/cpp/CppSymGen.cpp
SOURCES += gen/cpp/CppTypeGen.cpp
SOURCES += gen/cpp/CppStructSerdes.cpp
SOURCES += gen/cpp/CppProxyCommon.cpp
SOURCES += gen/cpp/CppClientProxy.cpp
SOURCES += gen/cpp/CppServerProxy.cpp

GENDIR = .gen
CLEAN_EXTRA += $(GENDIR)
SOURCES += $(GENDIR)/rpcLexer.cpp $(GENDIR)/rpcParser.cpp

Ast.cpp: Ast.h

Ast.h: $(GENDIR)/rpcParser.h $(GENDIR)/rpcLexer.h

LIBS += antlr4-runtime

INCLUDE_DIRS += .
INCLUDE_DIRS += ..
INCLUDE_DIRS += $(GENDIR)
INCLUDE_DIRS += /usr/include/antlr4-runtime

CXXFLAGS += -O0 -g3
CXXFLAGS += --std=c++17
CXXFLAGS += -fno-inline
CXXFLAGS += -fmax-errors=5
CXXFLAGS += -Wall -Werror -Wno-unused-value -Wreturn-local-addr  -Wno-attributes 

CXXFLAGS += -rdynamic
CXXFLAGS += --coverage
LDFLAGS += -rdynamic
LIBS += gcov 

LD=$(CXX)

COVROOT = ..

include ../cli-base/mod.mk
include ../ultimate-makefile/Makefile.ultimate

$(abspath $(GENDIR))/%Lexer.cpp $(abspath $(GENDIR))/%Parser.cpp \
$(abspath $(GENDIR))/%Lexer.h $(abspath $(GENDIR))/%Parser.h: %.g4
	@printf "\e[1;32mAntlr $^ -> $@\e[0m\n"
	@antlr4 -o $(GENDIR) $^ -no-listener -no-visitor -Dlanguage=Cpp
