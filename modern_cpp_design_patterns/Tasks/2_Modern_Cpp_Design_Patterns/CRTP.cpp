/**************************************************************************************************
*
* \file CRTP.cpp
* \brief C++ Training - Example for the CRTP Design Pattern
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Implement a common output operator for both 'DynamicVector' and 'StaticVector' by means
*       of the CRTP design pattern.
*
**************************************************************************************************/

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>


//--DynamicVector.h--------------------------------------------------------------------------------

template< typename T >
class DynamicVector
{
 public:
   DynamicVector() = default;
   DynamicVector( std::initializer_list<T> init )
      : values_( begin(init), end(init) )
   {}

   size_t size() const noexcept { return values_.size(); }

   T&       operator[]( size_t index )       noexcept { return values_[index]; }
   const T& operator[]( size_t index ) const noexcept { return values_[index]; }

 private:
   std::vector<T> values_;
};

template< typename T >
std::ostream& operator<<( std::ostream& os, const DynamicVector<T>& vector )
{
   const size_t size( vector.size() );

   os << "(";
   for( size_t i=0UL; i<size; ++i ) {
      std::cout << " " << vector[i];
   }
   os << " )";

   return os;
}


//--StaticVector.h---------------------------------------------------------------------------------

template< typename T, size_t Size >
class StaticVector
{
 public:
   StaticVector() = default;
   StaticVector( std::initializer_list<T> init )
   {
      std::copy( begin(init), end(init), begin(values_) );
   }

   size_t size() const noexcept { return values_.size(); }

   T&       operator[]( size_t index )       noexcept { return values_[index]; }
   const T& operator[]( size_t index ) const noexcept { return values_[index]; }

 private:
   std::array<T,Size> values_;
};

template< typename T, size_t Size >
std::ostream& operator<<( std::ostream& os, const StaticVector<T,Size>& vector )
{
   const size_t size( vector.size() );

   os << "(";
   for( size_t i=0UL; i<size; ++i ) {
      std::cout << " " << vector[i];
   }
   os << " )";

   return os;
}


//--Main.cpp---------------------------------------------------------------------------------------

int main()
{
   const DynamicVector<int> a{ 1, 2, 3 };
   const StaticVector<int,4UL> b{ 4, 5, 6, 7 };

   std::cerr << "\n"
             << " a = " << a << "\n"
             << " b = " << b << "\n"
             << "\n";

   return EXIT_SUCCESS;
}
