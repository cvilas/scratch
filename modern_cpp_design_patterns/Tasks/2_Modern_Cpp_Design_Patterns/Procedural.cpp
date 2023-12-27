/**************************************************************************************************
*
* \file Procedural.cpp
* \brief C++ Training - Example for a procedural approach to draw shapes
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

enum ShapeType
{
   circle,
   square
};

struct Shape
{
   Shape( ShapeType t )
      : type( t )
   {}

   virtual ~Shape() {}

   ShapeType type;
};


//--Circle.h---------------------------------------------------------------------------------------

struct Circle : public Shape
{
   Circle( double rad )
      : Shape( circle )
      , radius( rad )
      , center()
   {}

   ~Circle() {}

   double radius;
   Point center;
};

void DrawCircle( const Circle* );


//--Circle.cpp-------------------------------------------------------------------------------------

void DrawCircle( const Circle* c )
{
   std::cout << "circle: radius=" << c->radius << "\n";
}


//--Square.h---------------------------------------------------------------------------------------

struct Square : public Shape
{
   Square( double s )
      : Shape( square )
      , side( s )
      , center()
   {}

   ~Square() {}

   double side;
   Point center;
};

void DrawSquare( const Square* );


//--Square.cpp-------------------------------------------------------------------------------------

void DrawSquare( const Square* s )
{
   std::cout << "square: side=" << s->side << "\n";
}


//--DrawAllShapes.h--------------------------------------------------------------------------------

void DrawAllShapes( const std::vector<Shape*>& shapes );


//--DrawAllShapes.cpp------------------------------------------------------------------------------

void DrawAllShapes( const std::vector<Shape*>& shapes )
{
   for( const Shape* s : shapes )
   {
      switch ( s->type )
      {
         case circle:
            DrawCircle( static_cast<const Circle*>( s ) );
            break;
         case square:
            DrawSquare( static_cast<const Square*>( s ) );
            break;
      }
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

