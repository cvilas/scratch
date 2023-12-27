/**************************************************************************************************
*
* \file Visitor.cpp
* \brief C++ Training - Programming Task for the Visitor Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Refactor the classical Visitor solution by a value semantics based solution. Note that
*       the general behavior should remain unchanged.
*
**************************************************************************************************/

#include <cstdlib>
#include <iostream>
#include <variant>
#include <vector>


struct Circle
{
   double radius;
};


struct Square
{
   double side;
};


using Shape = std::variant<Circle,Square>;

struct Draw
{
   void operator()( const Circle& c ) const { std::cout << "circle: radius=" << c.radius << std::endl; }
   void operator()( const Square& s ) const { std::cout << "square: side=" << s.side << std::endl; }
};

void draw( const Shape& s )
{
   std::visit( Draw{}, s );
}


using Shapes = std::vector<Shape>;

void draw( Shapes const& shapes )
{
   for( auto const& shape : shapes )
   {
      draw( shape );
   }
}


int main()
{
   Shapes shapes;

   shapes.push_back( Circle{ 2.3 } );
   shapes.push_back( Square{ 1.2 } );
   shapes.push_back( Circle{ 4.1 } );

   draw( shapes );

   return EXIT_SUCCESS;
}

