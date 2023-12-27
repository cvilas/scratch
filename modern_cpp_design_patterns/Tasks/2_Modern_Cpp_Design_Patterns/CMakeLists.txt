#==================================================================================================
#
#  CMakeLists for the chapter "Modern C++ Design Patterns" of the C++ Training
#
#  Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
#
#  This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
#  context of the C++ training or with explicit agreement by Klaus Iglberger.
#
#==================================================================================================

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

add_executable(Command
   Command.cpp
   )

add_executable(CRTP
   CRTP.cpp
   )

add_executable(Decorator
   Decorator.cpp
   )

add_executable(ExpressionTemplates
   ExpressionTemplates.cpp
   )

add_executable(Function
   Function.cpp
   )

add_executable(ObjectOriented
   ObjectOriented.cpp
   )

add_executable(Observer
   Observer.cpp
   )

add_executable(Procedural
   Procedural.cpp
   )

add_executable(Prototype
   Prototype.cpp
   )

add_executable(Ranges
   Ranges.cpp
   )

add_executable(Singleton
   Singleton.cpp
   )

add_executable(StateMachine
   StateMachine.cpp
   )

add_executable(Strategy
   Strategy.cpp
   )

add_executable(TypeErasure
   TypeErasure.cpp
   )

add_executable(Visitor
   Visitor.cpp
   )

set_target_properties(
   Command
   CRTP
   Decorator
   ExpressionTemplates
   Function
   ObjectOriented
   Observer
   Procedural
   Prototype
   Ranges
   Singleton
   StateMachine
   Strategy
   TypeErasure
   Visitor
   PROPERTIES
   FOLDER "2_Modern_Cpp_Design_Patterns"
   )
