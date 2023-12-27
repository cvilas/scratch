/**************************************************************************************************
*
* \file Prototype.cpp
* \brief C++ Training - Example for the Prototype Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Discuss the advantages and disadvantages of the given implementation of the classical
*       prototype design pattern.
*
**************************************************************************************************/

#include <memory>
#include <cstdlib>
#include <iostream>


//--Prototype.h------------------------------------------------------------------------------------

class Prototype
{
 public:
   virtual ~Prototype() = default;
   virtual std::unique_ptr<Prototype> clone() const = 0;
};


//--ConcretePrototype1.h---------------------------------------------------------------------------

class ConcretePrototype1 : public Prototype
{
 public:
   std::unique_ptr<Prototype> clone() const override;
};


//--ConcretePrototype1.cpp-------------------------------------------------------------------------

std::unique_ptr<Prototype> ConcretePrototype1::clone() const
{
   std::cout << " ConcretePrototype1::clone()\n";
   return std::make_unique<ConcretePrototype1>();
}


//--ConcretePrototype2.h---------------------------------------------------------------------------

class ConcretePrototype2 : public Prototype
{
 public:
   std::unique_ptr<Prototype> clone() const override;
};


//--ConcretePrototype2.cpp-------------------------------------------------------------------------

std::unique_ptr<Prototype> ConcretePrototype2::clone() const
{
   std::cout << " ConcretePrototype2::clone()\n";
   return std::make_unique<ConcretePrototype2>();
}


//--Main.cpp---------------------------------------------------------------------------------------

int main()
{
   std::unique_ptr<Prototype> pp1( std::make_unique<ConcretePrototype1>() );
   std::unique_ptr<Prototype> pp2( std::make_unique<ConcretePrototype2>() );

   std::unique_ptr<Prototype> pp3( pp1->clone() );
   std::unique_ptr<Prototype> pp4( pp2->clone() );

   return EXIT_SUCCESS;
}
