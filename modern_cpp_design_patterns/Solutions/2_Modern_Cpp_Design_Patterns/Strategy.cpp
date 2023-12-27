/**************************************************************************************************
*
* \file Strategy.cpp
* \brief C++ Training - Programming Task for the Strategy Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
**************************************************************************************************/

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>


class Shape
{
 public:
   Shape() = default;

   virtual ~Shape() {}

   virtual void draw() const = 0;
};


struct Draw
{
   template< typename T >
   void operator()( const T& v ) const
   {
      draw( v );
   }
};


class Circle : public Shape
{
 public:
   using DrawStrategy = std::function<void(const Circle&)>;

   Circle( double r, DrawStrategy d = Draw{} ) : Shape(), radius_( r ), draw_( d ) {}

   ~Circle() {}

   void draw() const override { draw_( *this ); }

   double radius() const { return radius_; }

 private:
   double radius_;
   DrawStrategy draw_;
};

void draw( const Circle& circle )
{
   std::cout << "circle: radius=" << circle.radius() << std::endl;
}


class Square : public Shape
{
 public:
   using DrawStrategy = std::function<void(const Square&)>;

   Square( double s, DrawStrategy d = Draw{} ) : Shape(), side_( s ), draw_( d ) {}

   ~Square() {}

   void draw() const override { draw_( *this ); }

   double side() const { return side_; }

 private:
   double side_;
   DrawStrategy draw_;
};

void draw( Square const& square )
{
   std::cout << "square: side=" << square.side() << std::endl;
}


using Shapes = std::vector< std::unique_ptr<Shape> >;

void draw( Shapes const& shapes )
{
   for( auto const& shape : shapes )
   {
      shape->draw();
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

