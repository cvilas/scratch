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
   virtual ~Item() = default;
   virtual Dollars price() const = 0;
};


//---- <ConferenceFee.h> --------------------------------------------------------------------------

class ConferenceFee
   : public Item
{
 public:
   Dollars price() const override {
      return Dollars{ 999 };
   }
};


//---- <CppBook.h> --------------------------------------------------------------------------------

class CppBook
   : public Item
{
 public:
   Dollars price() const override {
      return Dollars{ 19 };
   }
};


//---- <DiscountItem.h> ---------------------------------------------------------------------------

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


//---- <TaxedItem.h> ------------------------------------------------------------------------------

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


//---- <Main.cpp> ---------------------------------------------------------------------------------

int main()
{
   std::unique_ptr<Item> item1(
      std::make_unique<TaxedItem>(
         std::make_unique<DiscountItem>(
            std::make_unique<ConferenceFee>() ) ) );

   std::unique_ptr<Item> item2(
      std::make_unique<TaxedItem>(
         std::make_unique<CppBook>() ) );

   std::cerr << "\n"
             << " Total price of item1 = " << item1->price() << "\n"
             << " Total price of item2 = " << item2->price() << "\n"
             << "\n";

   return EXIT_SUCCESS;
}

