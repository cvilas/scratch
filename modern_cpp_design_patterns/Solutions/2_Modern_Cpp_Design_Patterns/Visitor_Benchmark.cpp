/**************************************************************************************************
*
* \file Visitor_Benchmark.cpp
* \brief C++ Training - Programming Task for the Visitor Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
**************************************************************************************************/

#define BENCHMARK_ENUM_SOLUTION 1
#define BENCHMARK_OO_SOLUTION 1
#define BENCHMARK_VISITOR_SOLUTION 1
#define BENCHMARK_STD_VARIANT_SOLUTION 1
#define BENCHMARK_MPARK_VARIANT_SOLUTION 1
#define BENCHMARK_BOOST_VARIANT_SOLUTION 0


#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <variant>
#include <vector>
#if BENCHMARK_MPARK_VARIANT_SOLUTION
#  include "mpark/variant.hpp"
#endif
#if BENCHMARK_BOOST_VARIANT_SOLUTION
#  include <boost/variant/variant.hpp>
#endif


struct Vector3D
{
   double x{};
   double y{};
   double z{};
};

Vector3D operator+( const Vector3D& a, const Vector3D& b )
{
   return Vector3D{ a.x+b.x, a.y+b.y, a.z+b.z };
}


#if BENCHMARK_ENUM_SOLUTION
namespace enum_solution {

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


   struct Circle : public Shape
   {
      Circle( double rad )
         : Shape( circle )
         , radius( rad )
         , center()
      {}

      ~Circle() {}

      double radius;
      Vector3D center;
   };

   void translate( Circle& c, const Vector3D& v )
   {
      c.center = c.center + v;
   }


   struct Square : public Shape
   {
      Square( double s )
         : Shape( square )
         , side( s )
         , center()
      {}

      ~Square() {}

      double side;
      Vector3D center;
   };

   void translate( Square& s, const Vector3D& v )
   {
      s.center = s.center + v;
   }


   using Shapes = std::vector< std::unique_ptr<Shape> >;

   void translate( Shapes& shapes, const Vector3D& v )
   {
      for( const auto& s : shapes )
      {
         switch ( s->type )
         {
            case circle:
               translate( static_cast<Circle&>( *s.get() ), v );
               break;
            case square:
               translate( static_cast<Square&>( *s.get() ), v );
               break;
         }
      }
   }

} // namespace enum_solution
#endif


#if BENCHMARK_OO_SOLUTION
namespace object_oriented_solution {

   struct Shape
   {
      Shape()
      {}

      virtual ~Shape() {}

      virtual void translate( const Vector3D& v ) = 0;
   };


   struct Circle : public Shape
   {
      Circle( double rad )
         : Shape()
         , radius( rad )
         , center()
      {}

      ~Circle() {}

      void translate( const Vector3D& v ) override
      {
         center = center + v;
      }

      double radius;
      Vector3D center;
   };


   struct Square : public Shape
   {
      Square( double s )
         : Shape()
         , side( s )
         , center()
      {}

      ~Square() {}

      void translate( const Vector3D& v ) override
      {
         center = center + v;
      }

      double side;
      Vector3D center;
   };


   using Shapes = std::vector< std::unique_ptr<Shape> >;

   void translate( Shapes& shapes, const Vector3D& v )
   {
      for( const auto& s : shapes )
      {
         s->translate( v );
      }
   }

} // namespace object_oriented_solution
#endif


#if BENCHMARK_VISITOR_SOLUTION
namespace visitor_solution {

   struct Circle;
   struct Square;


   struct Visitor
   {
      virtual ~Visitor() = default;

      virtual void visit( Circle& ) const = 0;
      virtual void visit( Square& ) const = 0;
   };


   struct Shape
   {
      Shape() = default;

      virtual ~Shape() {}

      virtual void accept( const Visitor& v ) = 0;
   };


   struct Circle : public Shape
   {
      Circle( double r )
         : Shape{}
         , radius{ r }
      {}

      ~Circle() {}

      void accept( const Visitor& v ) override { v.visit( *this ); }

      double radius{};
      Vector3D center{};
   };


   struct Square : public Shape
   {
      Square( double s )
         : Shape{}
         , side{ s }
      {}

      ~Square() {}

      void accept( const Visitor& v ) override { v.visit( *this ); }

      double side{};
      Vector3D center{};
   };


   struct Translate : public Visitor
   {
      Translate( const Vector3D& vec ) : v{ vec } {}
      void visit( Circle& c ) const override { c.center = c.center + v; }
      void visit( Square& s ) const override { s.center = s.center + v; }
      Vector3D v{};
   };


   using Shapes = std::vector< std::unique_ptr<Shape> >;

   void translate( Shapes const& shapes, const Vector3D& v )
   {
      for( auto const& shape : shapes )
      {
         shape->accept( Translate{ v } );
      }
   }

} // namespace visitor_solution
#endif


#if BENCHMARK_STD_VARIANT_SOLUTION
namespace std_variant_solution {

   struct Circle
   {
      double radius{};
      Vector3D center{};
   };


   struct Square
   {
      double side{};
      Vector3D center{};
   };


   using Shape = std::variant<Circle,Square>;

   struct Translate
   {
      void operator()( Circle& c ) const { c.center = c.center + v; }
      void operator()( Square& s ) const { s.center = s.center + v; }
      Vector3D v{};
   };

   void translate( Shape& s, const Vector3D& v )
   {
      std::visit( Translate{ v }, s );
   }


   using Shapes = std::vector<Shape>;

   void translate( Shapes& shapes, const Vector3D& v )
   {
      for( auto& shape : shapes )
      {
         translate( shape, v );
      }
   }

} // namespace std_variant_solution
#endif


#if BENCHMARK_MPARK_VARIANT_SOLUTION
namespace mpark_variant_solution {

   struct Circle
   {
      double radius{};
      Vector3D center{};
   };


   struct Square
   {
      double side{};
      Vector3D center{};
   };


   using Shape = mpark::variant<Circle,Square>;

   struct Translate
   {
      void operator()( Circle& c ) const { c.center = c.center + v; }
      void operator()( Square& s ) const { s.center = s.center + v; }
      Vector3D v{};
   };

   void translate( Shape& s, const Vector3D& v )
   {
      mpark::visit( Translate{ v }, s );
   }


   using Shapes = std::vector<Shape>;

   void translate( Shapes& shapes, const Vector3D& v )
   {
      for( auto& shape : shapes )
      {
         translate( shape, v );
      }
   }

} // namespace mpark_variant_solution
#endif


#if BENCHMARK_BOOST_VARIANT_SOLUTION
namespace boost_variant_solution {

   struct Circle
   {
      double radius{};
      Vector3D center{};
   };


   struct Square
   {
      double side{};
      Vector3D center{};
   };


   using Shape = boost::variant<Circle,Square>;

   struct Translate
   {
      void operator()( Circle& c ) const { c.center = c.center + v; }
      void operator()( Square& s ) const { s.center = s.center + v; }
      Vector3D v{};
   };

   void translate( Shape& s, const Vector3D& v )
   {
      boost::apply_visitor( Translate{ v }, s );
   }


   using Shapes = std::vector<Shape>;

   void translate( Shapes& shapes, const Vector3D& v )
   {
      for( auto& shape : shapes )
      {
         translate( shape, v );
      }
   }

} // namespace boost_variant_solution
#endif


int main()
{
   const size_t N    ( 100UL );
   const size_t steps( 2500000UL );

   std::random_device rd{};
   const unsigned int seed( rd() );

   std::mt19937 rng{};
   std::uniform_real_distribution<double> dist( 0.0, 1.0 );

   std::cout << std::endl;

#if BENCHMARK_ENUM_SOLUTION
   {
      using namespace enum_solution;

      rng.seed( seed );

      Shapes shapes;

      for( size_t i=0UL; i<N; ++i ) {
         if( dist( rng ) < 0.5 )
            shapes.push_back( std::make_unique<Circle>( dist( rng ) ) );
         else
            shapes.push_back( std::make_unique<Square>( dist( rng ) ) );
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         translate( shapes, Vector3D{ dist( rng ), dist( rng ) } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      std::cout << " Enum solution runtime           : " << seconds << "s\n";
   }
#endif

#if BENCHMARK_OO_SOLUTION
   {
      using namespace object_oriented_solution;

      rng.seed( seed );

      Shapes shapes;

      for( size_t i=0UL; i<N; ++i ) {
         if( dist( rng ) < 0.5 )
            shapes.push_back( std::make_unique<Circle>( dist( rng ) ) );
         else
            shapes.push_back( std::make_unique<Square>( dist( rng ) ) );
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         translate( shapes, Vector3D{ dist( rng ), dist( rng ) } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      std::cout << " OO solution runtime             : " << seconds << "s\n";
   }
#endif

#if BENCHMARK_VISITOR_SOLUTION
   {
      using namespace visitor_solution;

      rng.seed( seed );

      Shapes shapes;

      for( size_t i=0UL; i<N; ++i ) {
         if( dist( rng ) < 0.5 )
            shapes.push_back( std::make_unique<Circle>( dist( rng ) ) );
         else
            shapes.push_back( std::make_unique<Square>( dist( rng ) ) );
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         translate( shapes, Vector3D{ dist( rng ), dist( rng ) } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      std::cout << " Classic visitor solution runtime: " << seconds << "s\n";
   }
#endif

#if BENCHMARK_STD_VARIANT_SOLUTION
   {
      using namespace std_variant_solution;

      rng.seed( seed );

      Shapes shapes;

      for( size_t i=0UL; i<N; ++i ) {
         if( dist( rng ) < 0.5 )
            shapes.push_back( Circle{ dist( rng ) } );
         else
            shapes.push_back( Square{ dist( rng ) } );
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         translate( shapes, Vector3D{ dist( rng ), dist( rng ) } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      std::cout << " std::variant solution runtime   : " << seconds << "s\n";
   }
#endif

#if BENCHMARK_MPARK_VARIANT_SOLUTION
   {
      using namespace mpark_variant_solution;

      rng.seed( seed );

      Shapes shapes;

      for( size_t i=0UL; i<N; ++i ) {
         if( dist( rng ) < 0.5 )
            shapes.push_back( Circle{ dist( rng ) } );
         else
            shapes.push_back( Square{ dist( rng ) } );
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         translate( shapes, Vector3D{ dist( rng ), dist( rng ) } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      std::cout << " mpark::variant solution runtime : " << seconds << "s\n";
   }
#endif

#if BENCHMARK_BOOST_VARIANT_SOLUTION
   {
      using namespace boost_variant_solution;

      rng.seed( seed );

      Shapes shapes;

      for( size_t i=0UL; i<N; ++i ) {
         if( dist( rng ) < 0.5 )
            shapes.push_back( Circle{ dist( rng ) } );
         else
            shapes.push_back( Square{ dist( rng ) } );
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         translate( shapes, Vector3D{ dist( rng ), dist( rng ) } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      std::cout << " boost::variant solution runtime : " << seconds << "s\n";
   }
#endif

   std::cout << std::endl;

   return EXIT_SUCCESS;
}

