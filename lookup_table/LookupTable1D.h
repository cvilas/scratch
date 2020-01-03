//==============================================================================
/// \file        LookupTable1D.h
/// \brief       A one-dimensional lookup table 
//==============================================================================

#ifndef LOOKUPTABLE1D_H
#define	LOOKUPTABLE1D_H

#include <vector>
#include <algorithm>

//==============================================================================
/// \class LookupTable1D
/// \brief A 1D look-up table (LUT)
/// 
/// Provides a 1-dimensional look-up table (LUT). The user sets independent-dependent 
/// variable pairs into the table, and for a given value of independent variable, 
/// the class will compute the dependent variable by interpolation. Upon performing
/// a look-up, if the value of independent variable falls above or below the limits
/// set by data points in the LUT, the returned value is just the upper or lower limit, 
/// respectively; i.e. no extrapolation is performed.
///
/// Example Program:
/// \include LookupTable1DTest.cpp
//==============================================================================

template <class I, class D>
class LookupTable1D 
{
public:
    
    /// Create a LUT. At least two data points are required for this
    /// to be sensible. More data-pairs can be inserted later if required.
    /// \param (x0, y0) Data-pair 1, x is independent variable.
    /// \param (x1, y1) Data-pair 2, x is indepenedent variable.
    LookupTable1D(const I& x0, const D& y0, const I& x1, const D& y1);
    
    ~LookupTable1D();
    
    /// Insert a new data point into the LUT.
    /// \param x (input) Independent variable
    /// \param y (input) Dependent variable.
    void insertDataPoint(const I& x, const D& y);
    
    /// Look up for y, given x. If x is falls above or below the set of 
    /// data points in the LUT, the returned value is just the upper or lower 
    /// limit, respectively; i.e. no extrapolation is performed. If x falls
    /// within boundaries set in the table, a value computed by interpolating
    /// between closest data points on either side of x is returned.
    /// \param x (input) Independent variable.
    /// \return interpolated value for the dependent variable.
    D lookup(const I& x);
    
private:
    struct CompareI //!< my comparison functor
    {
        bool operator()(const std::pair<I,D>& lhs, const I& rhs) { return lhs.first < rhs; } 
    };
    
    std::vector< std::pair<I,D> > dataPoints_;
    
}; // LookupTable



    
//------------------------------------------------------------------------------
template <class I, class D>
LookupTable1D<I,D>::LookupTable1D(const I& x0, const D& y0, const I& x1, const D& y1)
//------------------------------------------------------------------------------
{
    insertDataPoint(x0, y0);
    insertDataPoint(x1, y1);
}

//------------------------------------------------------------------------------
template <class I, class D>
LookupTable1D<I,D>::~LookupTable1D()
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
template <class I, class D>
void LookupTable1D<I,D>::insertDataPoint(const I& x, const D& y)
//------------------------------------------------------------------------------
{
    typename std::vector< std::pair<I,D> >::const_iterator itEnd = dataPoints_.end();
    typename std::vector< std::pair<I,D> >::iterator it = std::lower_bound( dataPoints_.begin(), dataPoints_.end(), x, CompareI() );
    
    if( it != itEnd )
    {
        dataPoints_.insert( it, std::pair<I,D>(x, y) );
    }
    else
    {
        dataPoints_.push_back( std::pair<I,D>(x, y) );
    }
}

//------------------------------------------------------------------------------
template <class I, class D>
D LookupTable1D<I,D>::lookup(const I& x)
//------------------------------------------------------------------------------
{
    typename std::vector< std::pair<I,D> >::const_iterator itBegin = dataPoints_.begin();
    typename std::vector< std::pair<I,D> >::const_iterator itEnd = dataPoints_.end();
    typename std::vector< std::pair<I,D> >::const_iterator it = std::lower_bound( dataPoints_.begin(), dataPoints_.end(), x, CompareI() );
    
    // if 'x' is below lower limit, clip to lower limit (don't extrapolate)
    if (it == itBegin )
    {
        return dataPoints_.front().second;
    }
    
    // if 'x' is above upper limit, clip to upper limit (don't extrapolate)
    else if( it == itEnd )
    {
        return dataPoints_.back().second;
    }

    // interpolate
    else
    {
        // y = y0 + [ (x - x0) * (y1 - y0)/(x1 - x0)]
        return (it-1)->second + (x - (it-1)->first) * ( it->second - (it-1)->second ) / ( it->first - (it-1)->first );
    }
}

#endif	// LOOKUPTABLE1D_H

