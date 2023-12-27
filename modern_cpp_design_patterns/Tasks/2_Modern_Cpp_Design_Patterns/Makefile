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
default: Command CRTP Decorator ExpressionTemplates Function ObjectOriented Observer \
         Procedural Prototype Ranges StateMachine Singleton Strategy TypeErasure Visitor

Command: Command.cpp
	$(CXX) $(CXXFLAGS) -o Command Command.cpp

CRTP: CRTP.cpp
	$(CXX) $(CXXFLAGS) -o CRTP CRTP.cpp

Decorator: Decorator.cpp
	$(CXX) $(CXXFLAGS) -o Decorator Decorator.cpp

ExpressionTemplates: ExpressionTemplates.cpp
	$(CXX) $(CXXFLAGS) -o ExpressionTemplates ExpressionTemplates.cpp

Function: Function.cpp
	$(CXX) $(CXXFLAGS) -o Function Function.cpp

ObjectOriented: ObjectOriented.cpp
	$(CXX) $(CXXFLAGS) -o ObjectOriented ObjectOriented.cpp

Observer: Observer.cpp
	$(CXX) $(CXXFLAGS) -o Observer Observer.cpp

Procedural: Procedural.cpp
	$(CXX) $(CXXFLAGS) -o Procedural Procedural.cpp

Prototype: Prototype.cpp
	$(CXX) $(CXXFLAGS) -o Prototype Prototype.cpp

Ranges: Ranges.cpp
	$(CXX) $(CXXFLAGS) -o Ranges Ranges.cpp

Singleton: Singleton.cpp
	$(CXX) $(CXXFLAGS) -o Singleton Singleton.cpp

StateMachine: StateMachine.cpp
	$(CXX) $(CXXFLAGS) -o StateMachine StateMachine.cpp

Strategy: Strategy.cpp
	$(CXX) $(CXXFLAGS) -o Strategy Strategy.cpp

TypeErasure: TypeErasure.cpp
	$(CXX) $(CXXFLAGS) -o TypeErasure TypeErasure.cpp

Visitor: Visitor.cpp
	$(CXX) $(CXXFLAGS) -o Visitor Visitor.cpp

clean:
	@$(RM) $(BIN)


# Setting the independent commands
.PHONY: default clean
