OUTPUT = rpc-tool

OBJDIR = .o

SOURCES += main.cpp
SOURCES += Ast.cpp
SOURCES += DumpMode.cpp

GENDIR = .gen
CLEAN_EXTRA += $(GENDIR)
SOURCES += $(GENDIR)/rpcLexer.cpp $(GENDIR)/rpcParser.cpp
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

#all: $(OUTPUT)
	
include ../ultimate-makefile/Makefile.ultimate

$(abspath $(GENDIR))/%Lexer.cpp $(abspath $(GENDIR))/%Parser.cpp \
$(abspath $(GENDIR))/%Lexer.h $(abspath $(GENDIR))/%Parser.h: %.g4
	@printf "\e[1;32mAntlr $^ -> $@\e[0m\n"
	@antlr4 -o $(GENDIR) $^ -no-listener -no-visitor -Dlanguage=Cpp
