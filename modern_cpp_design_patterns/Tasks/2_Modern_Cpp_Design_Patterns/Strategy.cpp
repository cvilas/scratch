/**************************************************************************************************
*
* \file Strategy_Cpp11.cpp
* \brief C++ Training - Programming Task for the Strategy Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
*
* Task: Refactor the classical Strategy solution by a value semantics based solution. Note that
*       the general behavior should remain unchanged.
*
**************************************************************************************************/

#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>


class Circle;
class Square;

class DrawStrategy
{
 public:
   virtual ~DrawStrategy() {}

   virtual void draw( const Circle& circle ) const = 0;
   virtual void draw( const Square& square ) const = 0;
};


class Shape
{
 public:
   Shape() = default;

   virtual ~Shape() {}

   virtual void draw() const = 0;
};


class Circle : public Shape
{
 public:
   Circle( double r, std::unique_ptr<DrawStrategy>&& d )
      : Shape()
      , radius_( r )
      , draw_( std::move(d) )
   {}

   ~Circle() {}

   void draw() const override { draw_->draw( *this ); }

   double radius() const { return radius_; }

 private:
   double radius_;
   std::unique_ptr<DrawStrategy> draw_;
};


class Square : public Shape
{
 public:
   Square( double s, std::unique_ptr<DrawStrategy>&& d )
      : Shape()
      , side_( s )
      , draw_( std::move(d) )
   {}

   ~Square() {}

   void draw() const override { draw_->draw( *this ); }

   double side() const { return side_; }

 private:
   double side_;
   std::unique_ptr<DrawStrategy> draw_;
};


class TestDrawStrategy : public DrawStrategy
{
 public:
   virtual ~TestDrawStrategy() {}

   void draw( const Circle& circle ) const override
   {
      std::cout << "circle: radius=" << circle.radius() << std::endl;
   }

   void draw( const Square& square ) const override
   {
      std::cout << "square: side=" << square.side() << std::endl;
   }
};




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

   shapes.push_back( std::make_unique<Circle>( 2.3, std::make_unique<TestDrawStrategy>() ) );
   shapes.push_back( std::make_unique<Square>( 1.2, std::make_unique<TestDrawStrategy>() ) );
   shapes.push_back( std::make_unique<Circle>( 4.1, std::make_unique<TestDrawStrategy>() ) );

   draw( shapes );

   return EXIT_SUCCESS;
}

