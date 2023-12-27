/**************************************************************************************************
*
* \file Decorator_Benchmark.cpp
* \brief C++ Training - Programming Task for the Decorator Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
**************************************************************************************************/

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <type_traits>
#include <vector>


struct Dollars
{
   uint64_t value;
};

Dollars& operator+=( Dollars& lhs, Dollars rhs )
{
   lhs.value += rhs.value;
   return lhs;
}

Dollars operator+( Dollars lhs, Dollars rhs )
{
   return Dollars{ lhs.value + rhs.value };
}

template< typename T, std::enable_if< std::is_arithmetic<T>::value >* = nullptr >
Dollars operator*( Dollars dollars, T factor )
{
   return Dollars{ static_cast<uint64_t>( dollars.value * factor ) };
}

std::ostream& operator<<( std::ostream& os, Dollars dollars )
{
   return os << dollars.value;
}


namespace classic_solution {

   class Item
   {
    public:
      virtual ~Item() = default;
      virtual Dollars price() const = 0;
   };


   class ConferenceFee
      : public Item
   {
    public:
      Dollars price() const override {
         return Dollars{ 999 };
      }
   };


   class CppBook
      : public Item
   {
    public:
      Dollars price() const override {
         return Dollars{ 19 };
      }
   };


   class DiscountItem
      : public Item
   {
    public:
      DiscountItem( std::unique_ptr<Item>&& item )
         : item_( std::move(item) )
      {}

      Dollars price() const override {
         return item_->price() * 0.8;
      }

    private:
      std::unique_ptr<Item> item_;
   };


   class TaxedItem
      : public Item
   {
    public:
      TaxedItem( std::unique_ptr<Item>&& item )
         : item_( std::move(item) )
      {}

      Dollars price() const override {
         return item_->price() * 1.19;
      }

    private:
      std::unique_ptr<Item> item_;
   };

} // namespace classic_solution


namespace type_erasure_solution {

   class Item
   {
    public:
      template< typename T >
      Item( const T& item ) : pimpl_( std::make_unique<Model<T>>( item ) ) {}

      template< typename T >
      Item( T&& item ) : pimpl_( std::make_unique<Model<T>>( std::move(item) ) ) {}

      Item( const Item& item ) : pimpl_( item.pimpl_->clone() ) {}
      Item( Item&& ) = default;

      ~Item() = default;

      Item& operator=( const Item& item ) { pimpl_ = item.pimpl_->clone(); return *this; }
      Item& operator=( Item&& item ) = default;

      Dollars price() const { return pimpl_->price(); }

    private:
      struct Concept
      {
         virtual ~Concept() = default;
         virtual Dollars price() const = 0;
         virtual std::unique_ptr<Concept> clone() const = 0;
      };

      template< typename T >
      struct Model : public Concept
      {
         explicit Model( const T& item ) : item_( item ) {}
         explicit Model( T&& item ) : item_( std::move(item) ) {}
         Dollars price() const override { return item_.price(); }
         std::unique_ptr<Concept> clone() const override { return std::make_unique<Model<T>>( item_ ); }
         T item_;
      };

      std::unique_ptr<Concept> pimpl_;
   };


   class ConferenceFee
   {
    public:
      Dollars price() const {
         return Dollars{ 999 };
      }
   };


   class CppBook
   {
    public:
      Dollars price() const {
         return Dollars{ 19 };
      }
   };


   class DiscountItem
   {
    public:
      DiscountItem( Item item )
         : item_( std::move(item) )
      {}

      Dollars price() const {
         return item_.price() * 0.8;
      }

    private:
      Item item_;
   };


   class TaxedItem
   {
    public:
      TaxedItem( Item item )
         : item_( std::move(item) )
      {}

      Dollars price() const {
         return item_.price() * 1.19;
      }

    private:
      Item item_;
   };

} // type_erasure_solution


int main()
{
   const size_t N    ( 1000UL );
   const size_t steps( 150000UL );

   std::random_device rd;
   std::mt19937 rng( rd() );
   std::uniform_real_distribution<double> dist( 0.0, 1.0 );

   {
      using namespace classic_solution;

      rng.seed( 1UL );

      std::vector< std::unique_ptr<Item> > items;
      Dollars total{};

      for( size_t i=0UL; i<N; ++i ) {
         const double rand = dist( rng );
         if( rand < 0.25 ) {
            items.push_back( std::make_unique<TaxedItem>(
                                std::make_unique<DiscountItem>(
                                   std::make_unique<ConferenceFee>() ) ) );
         }
         else if( rand < 0.5 ) {
            items.push_back( std::make_unique<TaxedItem>(
                                std::make_unique<ConferenceFee>() ) );
         }
         else if( rand < 0.75 ) {
            items.push_back( std::make_unique<TaxedItem>(
                                std::make_unique<DiscountItem>(
                                   std::make_unique<CppBook>() ) ) );
         }
         else {
            items.push_back( std::make_unique<TaxedItem>(
                                std::make_unique<CppBook>() ) );
         }
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         total += std::accumulate( items.begin(), items.end(), Dollars{}, []( Dollars accu, const auto& item ) { return accu + item->price(); } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      assert( total.value != 0.0 );

      std::cout << "\n Classic solution runtime    : " << seconds << "s\n";
   }

   {
      using namespace type_erasure_solution;

      rng.seed( 1UL );

      std::vector<Item> items;
      Dollars total{};

      for( size_t i=0UL; i<N; ++i ) {
         const double rand = dist( rng );
         if( rand < 0.25 ) {
            items.push_back( TaxedItem( DiscountItem( ConferenceFee{} ) ) );
         }
         else if( rand < 0.5 ) {
            items.push_back( TaxedItem( ConferenceFee{} ) );
         }
         else if( rand < 0.75 ) {
            items.push_back( TaxedItem( DiscountItem( CppBook{} ) ) );
         }
         else {
            items.push_back( TaxedItem( CppBook{} ) );
         }
      }

      std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
      start = std::chrono::high_resolution_clock::now();

      for( size_t s=0UL; s<steps; ++s ) {
         total += std::accumulate( items.begin(), items.end(), Dollars{}, []( Dollars accu, const auto& item ) { return accu + item.price(); } );
      }

      end = std::chrono::high_resolution_clock::now();
      const std::chrono::duration<double> elapsedTime( end - start );
      const double seconds( elapsedTime.count() );

      assert( total.value != 0.0 );

      std::cout << " Type erased solution runtime: " << seconds << "s\n";
   }

   return EXIT_SUCCESS;
}

