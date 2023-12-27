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
#include <memory>
#include <vector>


class Circle;
class Square;

class Visitor
{
 public:
   // Rule of 6 (see C.21)
   Visitor() = default;
   virtual ~Visitor() = default;
   Visitor( const Visitor& ) = default;
   Visitor& operator=( const Visitor& ) = default;
   Visitor( Visitor&& ) = default;
   Visitor& operator=( Visitor&& ) = default;

   virtual void visit( const Circle& ) const = 0;
   virtual void visit( const Square& ) const = 0;
};




class Shape
{
 public:
   // Rule of 6 (see C.21)
   Shape() = default;
   virtual ~Shape() = default;
   Shape( const Shape& ) = default;
   Shape& operator=( const Shape& ) = default;
   Shape( Shape&& ) = default;
   Shape& operator=( Shape&& ) = default;

   virtual void accept( const Visitor& v ) = 0;
};


class Circle : public Shape
{
 public:
   Circle( double r )
      : Shape()
      , radius_( r )
   {}

   ~Circle() {}

   void accept( const Visitor& v ) override { v.visit( *this ); }

   double radius() const { return radius_; }

 private:
   double radius_;
};


class Square : public Shape
{
 public:
   Square( double s )
      : Shape()
      , side_( s )
   {}

   ~Square() {}

   void accept( const Visitor& v ) override { v.visit( *this ); }

   double side() const { return side_; }

 private:
   double side_;
};




class Draw : public Visitor
{
 public:
   void visit( const Circle& c ) const override { std::cout << "circle: radius=" << c.radius() << "\n"; }
   void visit( const Square& s ) const override { std::cout << "square: side=" << s.side() << "\n"; }
};




using Shapes = std::vector< std::unique_ptr<Shape> >;

void draw( Shapes const& shapes )
{
   for( auto const& shape : shapes )
   {
      shape->accept( Draw{} );
   }
}


int main()
{
   Shapes shapes;

   shapes.push_back( std::make_unique<Circle>( 2.3 ) );
   shapes.push_back( std::make_unique<Square>( 1.2 ) );
   shapes.push_back( std::make_unique<Circle>( 4.1 ) );

   draw( shapes );

   return EXIT_SUCCESS;
}
