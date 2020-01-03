//==============================================================================
/// \file        LookupTable2D.h
//==============================================================================

#ifndef LOOKUPTABLE2D_H
#define	LOOKUPTABLE2D_H

//==============================================================================
/// \class LookupTable2D
/// \brief 
/// 
//==============================================================================

template <class I1, class I2, class D, int dimI1, int dimI2>
class LookupTable2D 
{
public:
    LookupTable2D();
    virtual ~LookupTable2D();
    void insertDataPoint(const I1& i1, const I2& i2, const D& d);
    D lookup(const I1& i1, const I2& i2);
private:
}; // LookupTable

#endif	// LOOKUPTABLE2D_H

