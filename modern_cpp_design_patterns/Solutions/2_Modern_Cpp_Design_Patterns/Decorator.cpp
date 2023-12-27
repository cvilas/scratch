/**************************************************************************************************
*
* \file Decorator.cpp
* \brief C++ Training - Programming Task for the Decorator Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
*
* Task: Refactor the classical Decorator solution by a value semantics based solution. Note that
*       the general behavior should remain unchanged.
*
**************************************************************************************************/

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <type_traits>


//---- <Dollars.h> --------------------------------------------------------------------------------

struct Dollars
{
   uint64_t value;
};

template< typename T, std::enable_if< std::is_arithmetic<T>::value >* = nullptr >
Dollars operator*( Dollars dollars, T factor )
{
   return Dollars{ static_cast<uint64_t>( dollars.value * factor ) };
}

std::ostream& operator<<( std::ostream& os, Dollars dollars )
{
   return os << dollars.value;
}


//---- <Item.h> -----------------------------------------------------------------------------------

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


//---- <ConferenceFee.h> --------------------------------------------------------------------------

class ConferenceFee
{
 public:
   Dollars price() const {
      return Dollars{ 999 };
   }
};


//---- <CppBook.h> --------------------------------------------------------------------------------

class CppBook
{
 public:
   Dollars price() const {
      return Dollars{ 19 };
   }
};


//---- <DiscountItem.h> ---------------------------------------------------------------------------

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


//---- <TaxedItem.h> ------------------------------------------------------------------------------

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


//---- <Main.cpp> ---------------------------------------------------------------------------------

int main()
{
   Item item1( TaxedItem( DiscountItem( ConferenceFee{} ) ) );
   Item item2( TaxedItem( CppBook{} ) );

   std::cerr << "\n"
             << " Total price of item1 = " << item1.price() << "\n"
             << " Total price of item2 = " << item2.price() << "\n"
             << "\n";

   return EXIT_SUCCESS;
}

