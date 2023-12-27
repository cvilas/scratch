/**************************************************************************************************
*
* \file Function.cpp
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

template< typename Fn >
class Function;

template< typename R, typename... Args >
class Function<R(Args...)>
{
 public:
   template< typename Fn >
   Function( Fn fn ) : pimpl_( new Model<Fn>( fn ) ) {}

   Function( Function const& f ) : pimpl_( f.pimpl_->clone() ) {}
   Function& operator=( Function f ) { std::swap( pimpl_, f.pimpl_ ); return *this; }

   Function( Function&& f ) : pimpl_( f.pimpl_ ) { f.pimpl_ = nullptr; }
   Function& operator=( Function&& f ) { delete pimpl_; pimpl_ = f.pimpl_; f.pimpl_ = nullptr; return *this; }

   ~Function() { delete pimpl_; }

   R operator()( Args... args ) { return (*pimpl_)( std::forward<Args>( args )... ); }

 private:
   class Concept
   {
    public:
      virtual ~Concept() = default;
      virtual R operator()( Args... ) const = 0;
      virtual Concept* clone() const = 0;
   };

   template< typename Fn >
   class Model : public Concept
   {
    public:
      explicit Model( Fn fn )
         : fn_( fn )
      {}

      R operator()( Args... args ) const override { return fn_( std::forward<Args>( args )... ); }
      Concept* clone() const override { return new Model( fn_ ); }

    private:
      Fn fn_;
   };

   Concept* pimpl_;
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
      Function<int(void)> f( foo );
      auto const res = f();
      std::cerr << "\n res = " << res << "\n\n";
   }

   {
      Function<int(void)> f( Foo{} );
      auto const res = f();
      std::cerr << "\n res = " << res << "\n\n";
   }

   {
      Function<int(void)> f( [](){ return 3; } );
      auto const res = f();
      std::cerr << "\n res = " << res << "\n\n";
   }

   return EXIT_SUCCESS;
}
