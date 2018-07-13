//==============================================================================
/// \file        JpegCompressor.h
//============================================================================== 
#ifndef JPEGCOMPRESSOR_H
#define	JPEGCOMPRESSOR_H

#include "ImageCompressor.h"

    
    //==============================================================================
    /// \class JpegCompressor
    /// \brief Image compression using jpeg library
    //==============================================================================
    class JpegCompressor : public ImageCompressor
    {
    public:

		/// Constructor. 
		/// \param quality Compresion quality (0 - 100)
		JpegCompressor(int quality);
		virtual ~JpegCompressor();
		bool pack(const Vision::CPixmapGray& in, unsigned char* pBuffer, unsigned long bufLen, unsigned long &dataLen);

	private:
		struct JpegCompressorP* pImpl_; // private data

    }; // JpegCompressor

    //==============================================================================
    /// \class JpegDecompressor
    /// \brief Image decompression using jpeg library
    //==============================================================================
	class JpegDecompressor : public ImageDecompressor
	{
	public:
		JpegDecompressor();
		virtual ~JpegDecompressor();
		bool unpack(unsigned char* pData, unsigned long dataLen, Vision::CPixmapGray& out);

	private:
		struct JpegDecompressorP* pImpl_;
	}; // JpegDecompressor

#endif	/* JPEGCOMPRESSOR_H */

