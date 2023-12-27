/**************************************************************************************************
*
* \file TypeErasure_dyno.cpp
* \brief C++ Training - Programming Task for Type Erasure
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Implement the 'Shape' class by means of the 'dyno' library (https://github.com/ldionne/dyno).
*       'Shape' may require all types to provide a free 'draw()' function that draws them to the
*       screen.
*
**************************************************************************************************/

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <dyno.hpp>
using namespace dyno::literals;


struct ShapeConcept : decltype( dyno::requires_(
   dyno::CopyConstructible{},
   dyno::MoveConstructible{},
   dyno::Destructible{},
   "draw"_s = dyno::function<void (const dyno::T&)>
)) { };

template <typename T>
auto const dyno::default_concept_map<ShapeConcept, T> = dyno::make_concept_map(
   "draw"_s = []( const T& self ) { draw( self ); }
);

class Shape
{
 public:
   template <typename T>
   Shape( T x ) : poly_{x} { }

   void draw() const
   { poly_.virtual_( "draw"_s )( poly_ ); }

 private:
   dyno::poly<ShapeConcept> poly_;
};

void draw( const Shape& shape )
{
   shape.draw();
}


using Shapes = std::vector<Shape>;

void draw( const Shapes& shapes )
{
   for( const auto& shape : shapes )
   {
      draw( shape );
   }
}


struct Circle
{
   double radius;
};

void draw( const Circle& circle )
{
   std::cout << "circle: radius=" << circle.radius << '\n';
}


struct Square
{
   double side;
};

void draw( const Square& square )
{
   std::cout << "square: side=" << square.side << '\n';
}


int main()
{
   Shapes shapes{};

   shapes.push_back( Circle{ 2.3 } );
   shapes.push_back( Square{ 1.2 } );
   shapes.push_back( Circle{ 4.1 } );

   draw( shapes );

   return EXIT_SUCCESS;
}

