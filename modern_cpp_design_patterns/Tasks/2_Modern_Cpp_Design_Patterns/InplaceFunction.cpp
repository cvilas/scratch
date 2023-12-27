/**************************************************************************************************
*
* \file InplaceFunction.cpp
* \brief C++ Training - Programming Task for Type Erasure
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
**************************************************************************************************/

#include <cstdlib>
#include <iostream>


//--Function.h-------------------------------------------------------------------------------------

template< typename Fn, size_t N >
class Function;

template< typename R, typename... Args, size_t N >
class Function<R(Args...),N>
{
 public:
   template< typename Fn >
   Function( Fn fn )
      : pimpl_( reinterpret_cast<Concept*>( buffer ) )
   {
      static_assert( sizeof(Fn) <= N, "Given type is too large" );
      new (pimpl_) Model<Fn>( fn );
   }

   Function( Function const& f )
      : pimpl_( f.pimpl_->clone( pimpl_ ) )
   {}

   Function& operator=( Function f )
   {
      pimpl_->~Concept();
      f.pimpl_->clone( pimpl_ );
      return *this;
   }

   ~Function() { pimpl_->~Concept(); }

   R operator()( Args... args ) { return (*pimpl_)( std::forward<Args>( args )... ); }

 private:
   class Concept
   {
    public:
      virtual ~Concept() = default;
      virtual R operator()( Args... ) const = 0;
      virtual void clone( Concept* memory ) const = 0;
   };

   template< typename Fn >
   class Model : public Concept
   {
    public:
      explicit Model( Fn fn )
         : fn_( fn )
      {}

      R operator()( Args... args ) const override { return fn_( std::forward<Args>( args )... ); }
      void clone( Concept* memory ) const override { new (memory) Model( fn_ ); }

    private:
      Fn fn_;
   };

   Concept* pimpl_;

   char buffer[N+8UL];
};


//--Main.cpp---------------------------------------------------------------------------------------


int foo()
{
   return 1;
}

struct Foo {
   int operator()() const {
      return 2;
   }
};


int main()
{
   {
      Function<int(void),8UL> f( foo );
      auto const res = f();
      std::cerr << "\n res = " << res << "\n\n";
   }

   {
      Function<int(void),8UL> f( Foo{} );
      auto const res = f();
      std::cerr << "\n res = " << res << "\n\n";
   }

   {
      Function<int(void),8UL> f( [](){ return 3; } );
      auto const res = f();
      std::cerr << "\n res = " << res << "\n\n";
   }

   return EXIT_SUCCESS;
}
