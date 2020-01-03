//==============================================================================
/// \file        LookupTable.h
//==============================================================================

#ifndef LOOKUPTABLE_H
#define	LOOKUPTABLE_H

#include <vector>
#include <algorithm>
#include <iostream>

//==============================================================================
/// \class LookupTable
/// \brief 
/// 
//==============================================================================

template <class X, class Y>
class LookupTable 
{
public:
    LookupTable(const X& x0, const Y& y0, const X& x1, const Y& y1);
    ~LookupTable();
    
    void insertDataPoint(const X& x, const Y& y);
    Y lookup(const X& in);
    
private:
    struct CompareX
    {
        bool operator()(const std::pair<X,Y>& lhs, const X& rhs) { return lhs.first < rhs; } 
    };
    std::vector< std::pair<X,Y> > dataPoints_;
    
}; // LookupTable



    
//------------------------------------------------------------------------------
template <class X, class Y>
LookupTable<X,Y>::LookupTable(const X& x0, const Y& y0, const X& x1, const Y& y1)
//------------------------------------------------------------------------------
{
    insertDataPoint(x0, y0);
    insertDataPoint(x1, y1);
}

//------------------------------------------------------------------------------
template <class X, class Y>
LookupTable<X,Y>::~LookupTable()
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
template <class X, class Y>
void LookupTable<X,Y>::insertDataPoint(const X& x, const Y& y)
//------------------------------------------------------------------------------
{
    typename std::vector< std::pair<X,Y> >::const_iterator itEnd = dataPoints_.end();
    typename std::vector< std::pair<X,Y> >::iterator it = std::lower_bound( dataPoints_.begin(), dataPoints_.end(), x, CompareX() );
    
    if( it != itEnd )
    {
        dataPoints_.insert( it, std::pair<X,Y>(x, y) );
    }
    else
    {
        dataPoints_.push_back( std::pair<X,Y>(x, y) );
    }
}

//------------------------------------------------------------------------------
template <class X, class Y>
Y LookupTable<X,Y>::lookup(const X& x)
//------------------------------------------------------------------------------
{
    typename std::vector< std::pair<X,Y> >::const_iterator itBegin = dataPoints_.begin();
    typename std::vector< std::pair<X,Y> >::const_iterator itEnd = dataPoints_.end();
    typename std::vector< std::pair<X,Y> >::const_iterator it = std::lower_bound( dataPoints_.begin(), dataPoints_.end(), x, CompareX() );
    
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
        return (it-1)->second + (x - (it-1)->first) * ( it->second - (it-1)->second ) / ( it->first - (it-1)->first );
    }
}

#endif	// LOOKUPTABLE_H

