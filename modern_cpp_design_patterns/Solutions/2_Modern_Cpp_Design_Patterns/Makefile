#==================================================================================================
#
#  Makefile for the chapter "Modern C++ Design Patterns" of the C++ Training
#
#  Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
#
#  This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
#  context of the C++ training or with explicit agreement by Klaus Iglberger.
#
#==================================================================================================


# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17


# Setting the source and binary files
SRC = $(wildcard *.cpp)
BIN = $(SRC:.cpp=)


# Rules
default: Command CRTP Decorator Decorator_Benchmark ExpressionTemplates Function Observer \
         Ranges Strategy Strategy_Benchmark TypeErasure TypeErasure_dyno Visitor \
         Visitor_Benchmark

Command: Command.cpp
	$(CXX) $(CXXFLAGS) -o Command Command.cpp

CRTP: CRTP.cpp
	$(CXX) $(CXXFLAGS) -o CRTP CRTP.cpp

Decorator: Decorator.cpp
	$(CXX) $(CXXFLAGS) -o Decorator Decorator.cpp

Decorator_Benchmark: Decorator_Benchmark.cpp
	$(CXX) $(CXXFLAGS) -o Decorator_Benchmark Decorator_Benchmark.cpp

ExpressionTemplates: ExpressionTemplates.cpp
	$(CXX) $(CXXFLAGS) -o ExpressionTemplates ExpressionTemplates.cpp

Function: Function.cpp
	$(CXX) $(CXXFLAGS) -o Function Function.cpp

Observer: Observer.cpp
	$(CXX) $(CXXFLAGS) -o Observer Observer.cpp

Ranges: Ranges.cpp
	$(CXX) $(CXXFLAGS) -o Ranges Ranges.cpp

Strategy: Strategy.cpp
	$(CXX) $(CXXFLAGS) -o Strategy Strategy.cpp

Strategy_Benchmark: Strategy_Benchmark.cpp
	$(CXX) $(CXXFLAGS) -o Strategy_Benchmark Strategy_Benchmark.cpp

TypeErasure: TypeErasure.cpp
	$(CXX) $(CXXFLAGS) -o TypeErasure TypeErasure.cpp

TypeErasure_dyno: TypeErasure_dyno.cpp
	$(CXX) $(CXXFLAGS) -o TypeErasure_dyno TypeErasure_dyno.cpp

Visitor: Visitor.cpp
	$(CXX) $(CXXFLAGS) -o Visitor Visitor.cpp

Visitor_Benchmark: Visitor_Benchmark.cpp
	$(CXX) $(CXXFLAGS) -o Visitor_Benchmark Visitor_Benchmark.cpp

clean:
	@$(RM) $(BIN)


# Setting the independent commands
.PHONY: default clean
