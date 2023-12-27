/**************************************************************************************************
*
* \file ObjectOriented.cpp
* \brief C++ Training - Example for an object-oriented approach to draw shapes
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Evaluate the given design with respect to changeability and extensibility.
*
**************************************************************************************************/

#include <iostream>
#include <vector>


//--Point.h----------------------------------------------------------------------------------------

struct Point
{
   Point()
      : x( 0.0 )
      , y( 0.0 )
   {}

   double x;
   double y;
};


//--Shape.h----------------------------------------------------------------------------------------

struct Shape
{
   Shape()
   {}

   virtual ~Shape() {}

   virtual void Draw() const = 0;
};


//--Circle.h---------------------------------------------------------------------------------------

struct Circle : public Shape
{
   Circle( double rad )
      : Shape()
      , radius( rad )
      , center()
   {}

   ~Circle() {}

   void Draw() const override;

   double radius;
   Point center;
};


//--Circle.cpp-------------------------------------------------------------------------------------

void Circle::Draw() const
{
   std::cout << "circle: radius=" << radius << "\n";
}


//--Square.h---------------------------------------------------------------------------------------

struct Square : public Shape
{
   Square( double s )
      : Shape()
      , side( s )
      , center()
   {}

   ~Square() {}

   void Draw() const override;

   double side;
   Point center;
};


//--Square.cpp-------------------------------------------------------------------------------------

void Square::Draw() const
{
   std::cout << "square: side=" << side << "\n";
}


//--DrawAllShapes.h--------------------------------------------------------------------------------

void DrawAllShapes( const std::vector<Shape*>& shapes );


//--DrawAllShapes.cpp------------------------------------------------------------------------------

void DrawAllShapes( const std::vector<Shape*>& shapes )
{
   for ( const Shape* s : shapes )
   {
      s->Draw();
   }
}


//--Main.cpp---------------------------------------------------------------------------------------

int main()
{
   using Shapes = std::vector<Shape*>;

   // Creating some shapes
   Shapes shapes;
   shapes.push_back( new Circle( 2.0 ) );
   shapes.push_back( new Square( 1.5 ) );
   shapes.push_back( new Circle( 4.2 ) );

   // Drawing all shapes
   DrawAllShapes( shapes );

   // Deleting all shapes
   for ( const Shape* s : shapes ) {
      delete s;
   }
}

