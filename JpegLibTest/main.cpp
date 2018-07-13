#include "jpegCompressor.h"

int main(int argc, char** argv)
{
	Vision::CPixmapGray imageIn, imageOut;
	
	unsigned char* pBuffer = new unsigned char [640*480];

	imageIn.loadPixmap("img_1328023892984.pgm");

	JpegCompressor zipper(100);

	unsigned long dataLen = 0;
	zipper.pack(imageIn, pBuffer, 640*480, dataLen);

	FILE* fp = fopen("img_1328023892984.jpg", "wb+");
	fwrite(pBuffer, 1, dataLen, fp);
	fclose(fp);

	JpegDecompressor unzipper;
	imageOut.create(640, 480);
	unzipper.unpack(pBuffer, dataLen, imageOut);
	
	imageOut.savePixmap("img_recovered.pgm");
	
	delete [] pBuffer;
	return 0;
}
