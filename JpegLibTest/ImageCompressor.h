//==============================================================================
/// \file        ImageCompressor.h
//============================================================================== 

#ifndef IMAGECOMPRESSOR_H
#define	IMAGECOMPRESSOR_H

#include "Pixmap.h"

    
    //==============================================================================
    /// \class ImageCompressor
    /// \brief Image compression class
    //==============================================================================
    class ImageCompressor
    {
    public:
		ImageCompressor() {}
		virtual ~ImageCompressor() {}
		
		/// compress image
		/// \param in input image
		/// \param pOutBuffer user's buffer that will contain compressed image on return. 
		/// \param pOutBufLen legnth of the user buffer.
		/// \param outDataLen number of bytes filled in the output buffer 
		/// \return true on success. False if 
		///			- output buffer length is insufficient
		///			- input image is invalid
		virtual bool pack(const Vision::CPixmapGray& in, unsigned char* pOutBuffer, unsigned long outBufLen, unsigned long &outDataLen) = 0;

    }; // ImageCompressor
    
	class ImageDecompressor
	{
	public:
		ImageDecompressor() {}
		virtual ~ImageDecompressor() {}
		
		/// Expand compressed image
		/// \param pData input buffer containing compressed data
		/// \param dataLen length of compressed data in bytes.
		/// \param out uncompressed output image. 
		/// NOTE: 'out' must contain sufficient space to accomodate the image. Method
		/// does no memor allocation
		/// \return true on success. False if 
		///			- output buffer length is insufficient
		///			- input image is invalid
		virtual bool unpack(unsigned char* pData, unsigned long dataLen, Vision::CPixmapGray& out) = 0;

	}; // ImageDecompressor


#endif	/* IMAGECOMPRESSOR_H */

