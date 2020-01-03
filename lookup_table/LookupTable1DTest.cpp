//==============================================================================
/// \file        LookupTableTest.cpp
/// \brief       Example test program for LookupTable class
//==============================================================================

#include "LookupTable1D.h"
#include <fstream>
#include <iostream>

//==============================================================================
int LookupTable1DTest(int argc, char** argv)
//==============================================================================
{
    // create a LUT for airpseed versus specific thrust
    // air-speed (ktas)        [0    10   20   30   40   50   60   70   80]; 
    // specific thrust (lb/hp) [7.46 6.83 6.20 5.57 4.83 4.31 3.68 3.26 2.84];
    
    LookupTable1D<double, double> table(0, 7.46, 10, 6.83); 
    
    // add remaining data points
    table.insertDataPoint(20, 6.20);
    table.insertDataPoint(30, 5.57);
    table.insertDataPoint(50, 4.31);
    table.insertDataPoint(40, 4.83); //!< data doesn't necessarily have to be inserted in order.
    table.insertDataPoint(60, 3.68);
    table.insertDataPoint(70, 3.26);
    table.insertDataPoint(80, 2.84);

    // create a file with data.
    std::ofstream fs;
    fs.open("LookupTable1DTest_output.txt");
    
    if( !fs.is_open() )
    {
        std::cout << "Unable to open output file" << std::endl;
        return -1;
    }
    
    double tas = 0;
    while( tas <= 100 )
    {
        fs << tas << "\t" << table.lookup(tas) << std::endl;
        tas += 1;
    }
    
    fs.close();
   
    return 0;
}

