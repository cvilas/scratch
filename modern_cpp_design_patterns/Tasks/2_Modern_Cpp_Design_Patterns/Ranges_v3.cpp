/**************************************************************************************************
*
* \file Ranges_v3.cpp
* \brief C++ Training - Example for the ranges_v3 library
*
* Copyright (C) 2015-2020 Klaus Iglberger - All Rights Reserved
*
* This file is part of the C++ training by Klaus Iglberger. The file may only be used in the
* context of the C++ training or with explicit agreement by Klaus Iglberger.
*
* Task: Modify the given example of the ranges_v3 library to compute the first four squares of
*       odd numbers.
*
**************************************************************************************************/

#include <iostream>
#include <vector>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>


int main()
{
   std::vector<int> numbers{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

   auto evenNumbers = numbers | ranges::views::filter([](int n){ return n % 2 == 0; })
                              | ranges::views::transform([](int n) { return n * 2; })
                              | ranges::views::take( 3UL );

   std::cout << "\n (";
   for( int i : evenNumbers )
      std::cout << " " << i;
   std::cout << " )\n\n";

   return EXIT_SUCCESS;
}
