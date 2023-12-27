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

add_executable(Decorator_Benchmark
   Decorator_Benchmark.cpp
   )

add_executable(ExpressionTemplates
   ExpressionTemplates.cpp
   )

add_executable(Function
   Function.cpp
   )

add_executable(Observer
   Observer.cpp
   )

add_executable(Ranges
   Ranges.cpp
   )

add_executable(Strategy
   Strategy.cpp
   )

add_executable(Strategy_Benchmark
   Strategy_Benchmark.cpp
   )

add_executable(TypeErasure
   TypeErasure.cpp
   )

add_executable(TypeErasure_dyno
   TypeErasure_dyno.cpp
   )

add_executable(Visitor
   Visitor.cpp
   )

add_executable(Visitor_Benchmark
   Visitor_Benchmark.cpp
   )

set_target_properties(
   Command
   CRTP
   Decorator
   Decorator_Benchmark
   ExpressionTemplates
   Function
   Observer
   Ranges
   Strategy
   Strategy_Benchmark
   TypeErasure
   TypeErasure_dyno
   Visitor
   Visitor_Benchmark
   PROPERTIES
   FOLDER "2_Modern_Cpp_Design_Patterns"
   )
