/**************************************************************************************************
*
* \file TypeErasure.cpp
* \brief C++ Training - Programming Task for Type Erasure
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Implement the 'Shape' class by means of Type Erasure. 'Shape' may require all types to
*       provide a free 'draw()' function that draws them to the screen.
*
**************************************************************************************************/

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>


class Shape
{
 public:
   template< typename T >
   Shape( T const& x ) : pimpl( new Model<T>( x ) ) {}

   ~Shape() = default;
   Shape( Shape const& s ) : pimpl( s.pimpl->clone() ) {}
   Shape( Shape&& s ) = default;
   Shape& operator=( const Shape& s ) { Shape tmp( s ); std::swap( pimpl, tmp.pimpl ); return *this; }
   Shape& operator=( Shape&& s ) = default;


 private:
   friend void draw( Shape const& shape )
   {
      shape.pimpl->do_draw();
   }

   struct Concept
   {
      virtual ~Concept() {}
      virtual void do_draw() const = 0;
      virtual Concept* clone() const = 0;  // Prototype design pattern
   };

   template< typename T >
   struct Model : Concept
   {
      Model( T const& value ) : object( value ) {}
      void do_draw() const override { draw( object ); }
      Concept* clone() const override { return new Model( object ); }
      T object;
   };

   std::unique_ptr<Concept> pimpl;
};


using Shapes = std::vector<Shape>;

void draw( Shapes const& shapes )
{
   for( auto const& shape : shapes )
   {
      draw( shape );
   }
}


struct Circle
{
   double radius;
};

void draw( Circle const& circle )
{
   std::cout << "circle: radius=" << circle.radius << std::endl;
}


struct Square
{
   double side;
};

void draw( Square const& square )
{
   std::cout << "square: side=" << square.side << std::endl;
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

