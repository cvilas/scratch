//==============================================================================
/// \file        JpegCompressor.cpp
//============================================================================== 

#include "JpegCompressor.h"
#include <jpeglib.h>

//==============================================================================
	// Custom destination manager for memory to memory compression.
	// See IJG documentation for details.
	// None of this code is mine -Vilas
	//==============================================================================
	typedef struct 
	{
		struct jpeg_destination_mgr pub;	/* base class */
		JOCTET* buffer;						/* buffer start address */
		unsigned long bufsize;				/* size of buffer */
		size_t datasize;					/* final size of compressed data */
		unsigned long* outsize;				/* user pointer to datasize */
		int errcount;						/* counts up write errors due to buffer overruns */
	} memory_destination_mgr;

	typedef memory_destination_mgr* mem_dest_ptr;

	// This function is called by the library before any data gets written
	METHODDEF(void) init_destination (j_compress_ptr cinfo)
	{
		mem_dest_ptr dest = (mem_dest_ptr)cinfo->dest;

		dest->pub.next_output_byte = dest->buffer;	/* set destination buffer */
		dest->pub.free_in_buffer = dest->bufsize;	/* input buffer size */
		dest->datasize = 0;							/* reset output size */
		dest->errcount = 0;							/* reset error count */
	}

	// This function is called by the library if the buffer fills up
	//
	// I just reset destination pointer and buffer size here.
	// Note that this behavior, while preventing seg faults
	// will lead to invalid output streams as data is over-written.
	METHODDEF(boolean) empty_output_buffer (j_compress_ptr cinfo)
	{
		mem_dest_ptr dest = (mem_dest_ptr)cinfo->dest;
		dest->pub.next_output_byte = dest->buffer;
		dest->pub.free_in_buffer = dest->bufsize;
		++dest->errcount; /* need to increase error count */
	
		return TRUE;
	}

	// Usually the library wants to flush output here.
	//
	// I will calculate output buffer size here.
	// Note that results become incorrect, once
	// empty_output_buffer was called.
	// This situation is notified by errcount.
	METHODDEF(void) term_destination (j_compress_ptr cinfo)
	{
		mem_dest_ptr dest = (mem_dest_ptr)cinfo->dest;
		dest->datasize = dest->bufsize - dest->pub.free_in_buffer;
		if (dest->outsize) *dest->outsize += (int)dest->datasize;
	}

	// Override the default destination manager initialization
	// provided by jpeglib. Since we want to use memory-to-memory
	// compression, we need to use our own destination manager.
	GLOBAL(void) jpeg_memory_dest (j_compress_ptr cinfo, JOCTET* buffer, unsigned long bufsize, unsigned long* outsize)
	{
		mem_dest_ptr dest;

		/* first call for this instance - need to setup */
		if (cinfo->dest == 0) 
		{
			cinfo->dest = (struct jpeg_destination_mgr *)
				(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				sizeof (memory_destination_mgr));
		}

		dest = (mem_dest_ptr) cinfo->dest;
		dest->bufsize = bufsize;
		dest->buffer = buffer;
		dest->outsize = outsize;

		/* set method callbacks */
		dest->pub.init_destination = init_destination;
		dest->pub.empty_output_buffer = empty_output_buffer;
		dest->pub.term_destination = term_destination;
	}

	//============================================================================== 
	// private data for compressor
	struct JpegCompressorP
	//============================================================================== 
	{
		//--------------------
		JpegCompressorP() 
		//--------------------
		{ 
			cinfo_.err = jpeg_std_error(&jerr_); // setup normal JPEG error routines, then override error_exit (see example.c in IJG docs)
			jerr_.error_exit = JpegCompressorP::onError;

			jpeg_create_compress(&cinfo_);

			// set these before calling set_defaults
			cinfo_.in_color_space = JCS_GRAYSCALE;
			cinfo_.input_components = 1;

			// set defaults
			jpeg_set_defaults(&cinfo_);
		}
		
		//--------------------
		~JpegCompressorP()
		//--------------------
		{
			jpeg_destroy_compress(&cinfo_);
		}
		
		//--------------------
		static void onError(j_common_ptr cinfo) // replaces the standard error_exit method in the jpeg standard error handler.
		//--------------------
		{		
			// Display the error message. 
			(*cinfo->err->output_message) (cinfo);

			// notify 
			throw Generic::CException(-1, "[JpegCompressorP] JPEG compressor error. See last error message.");
		}

		struct jpeg_compress_struct cinfo_;
		struct jpeg_error_mgr		jerr_;
	};

	//============================================================================== 
	JpegCompressor::JpegCompressor(int quality)
	//============================================================================== 
	: pImpl_( new JpegCompressorP )
	{
		if( quality < 1 ) { quality = 1; }
		if( quality > 100 ) { quality = 100; }
		jpeg_set_quality(&pImpl_->cinfo_, quality, TRUE);
	}

	//------------------------------------------------------------------------------
	JpegCompressor::~JpegCompressor()
	//------------------------------------------------------------------------------
	{
		delete pImpl_;
	}

	//------------------------------------------------------------------------------
	bool JpegCompressor::pack(const Vision::CPixmapGray& in, unsigned char* pBuffer, unsigned long bufLen, unsigned long &dataLen)
	//------------------------------------------------------------------------------
	{
		unsigned char *pImgData = (unsigned char *) in.getPointer(0);
		if( pImgData == NULL )
		{
			return false;
		}

		dataLen = 0;

		jpeg_memory_dest(&pImpl_->cinfo_, pBuffer, bufLen, &dataLen);

		pImpl_->cinfo_.image_width = in.getWidth();
		pImpl_->cinfo_.image_height = in.getHeight();

		jpeg_start_compress(&pImpl_->cinfo_, TRUE);
		int rowStride = pImpl_->cinfo_.image_width;

		JSAMPROW rowPtr[1];
		while (pImpl_->cinfo_.next_scanline < pImpl_->cinfo_.image_height) 
		{
			rowPtr[0] = &pImgData[pImpl_->cinfo_.next_scanline * rowStride];
			jpeg_write_scanlines(&pImpl_->cinfo_, rowPtr, 1);
		}

		jpeg_finish_compress(&pImpl_->cinfo_);

		return (dataLen < bufLen);
	}

	//============================================================================== 
	// private data for decompressor
	//============================================================================== 
	struct JpegDecompressorP
	{
		//--------------------
		JpegDecompressorP() 
		//--------------------
		{ 
			cinfo_.err = jpeg_std_error(&jerr_); // setup normal JPEG error routines, then override error_exit (see example.c in IJG docs)
			jerr_.error_exit = JpegDecompressorP::onError;

			jpeg_create_decompress(&cinfo_);
		}
		
		//--------------------
		~JpegDecompressorP()
		//--------------------
		{
			jpeg_destroy_decompress(&cinfo_);
		}
		
		//--------------------
		static void onError(j_common_ptr cinfo) // replaces the standard error_exit method in the jpeg standard error handler.
		//--------------------
		{		
			// Display the error message. 
			(*cinfo->err->output_message) (cinfo);

			// notify 
			throw Generic::CException(-1, "[JpegDecompressorP] JPEG decompressor error. See last error message.");
		}

		struct jpeg_decompress_struct	cinfo_;
		struct jpeg_error_mgr			jerr_;
	};

	//============================================================================== 
	JpegDecompressor::JpegDecompressor()
	//============================================================================== 
	: pImpl_( new JpegDecompressorP )
	{
	}

	//------------------------------------------------------------------------------
	JpegDecompressor::~JpegDecompressor()
	//------------------------------------------------------------------------------
	{
		delete pImpl_;
	}

	//------------------------------------------------------------------------------
	bool JpegDecompressor::unpack(unsigned char* pData, unsigned long dataLen, Vision::CPixmapGray& out)
	//------------------------------------------------------------------------------
	{
		unsigned char *pOutData = (unsigned char *) out.getPointer(0);
		if( pOutData == NULL )
		{
			std::cout << "[JpegDecompressor::unpack] Output image buffer not initialised" << std::endl << std::flush;
			return false;
		}
		
		jpeg_mem_src(&pImpl_->cinfo_, pData, dataLen);

		(void) jpeg_read_header(&pImpl_->cinfo_, TRUE);

		if( (pImpl_->cinfo_.image_width != out.getWidth()) ||
			(pImpl_->cinfo_.image_height != out.getHeight()) ||
			(pImpl_->cinfo_.out_color_space != JCS_GRAYSCALE) )
		{
			std::cout << "[JpegDecompressor::unpack] Output image buffer parameters incorrect" << std::endl << std::flush;
			return false;
		}

		// todo: set decompression options

		(void) jpeg_start_decompress(&pImpl_->cinfo_);
		int rowStride = pImpl_->cinfo_.output_width * pImpl_->cinfo_.output_components;

		JSAMPROW rowPtr[1];
		while( pImpl_->cinfo_.output_scanline < pImpl_->cinfo_.output_height )
		{
			rowPtr[0] = &pOutData[pImpl_->cinfo_.output_scanline * rowStride];
			(void) jpeg_read_scanlines(&pImpl_->cinfo_, rowPtr, 1);
		}

		jpeg_finish_decompress(&pImpl_->cinfo_);

		return true;
	}


