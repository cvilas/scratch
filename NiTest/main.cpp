// Test program: Reads Xtion sensor, displays RGB and depth images
// Todo: Replace hardcoded creation of nodes with XML file based configuration.
// 	 See http://openni.org/Documentation/ProgrammerGuide.html

#include <iostream>
#include <XnOS.h>
#include <XnCppWrapper.h>
#ifdef _MSC_VER
typedef UINT16 uint16_t;
#else
#include <inttypes.h>
#endif
#include <math.h>
#include <QtGui/QApplication>
#include <QPixmap>
#include <QImage>
#include <QLabel>

using namespace std;

class DataCapture
{
public:
    DataCapture();
    ~DataCapture();
    bool initialise();
    bool startDataCapture();
    bool stopDataCapture();
    bool captureOne();
    char* getDepthImage() { return pDepthData_; }
    char* getRgbImage() { return pRgbData_; }
private:
    xn::Context context_;
    xn::DepthGenerator depthGen_;
    xn::DepthMetaData depthMd_; // needn't be a class member
    xn::ImageGenerator imageGen_;
    xn::ImageMetaData imageMd_; // needn't be a class member
    char* pDepthData_;
    char* pRgbData_;
};

DataCapture::DataCapture() : pDepthData_(0), pRgbData_(0)
{
}

DataCapture::~DataCapture()
{
    if( pDepthData_ ) delete [] pDepthData_;
    if( pRgbData_ ) delete [] pRgbData_;
}

bool DataCapture::initialise()
{
    context_.Shutdown();

    XnStatus rc = context_.Init(); 
    if( rc != XN_STATUS_OK )
    {
        std::cout << "Init: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    rc = depthGen_.Create(context_);
    if( rc != XN_STATUS_OK )
    {
        std::cout << "depthGen.Create: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    rc = imageGen_.Create(context_);
    if( rc != XN_STATUS_OK )
    {
        std::cout << "imageGen.Create: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    rc = imageGen_.SetPixelFormat(XN_PIXEL_FORMAT_RGB24);
    if( rc != XN_STATUS_OK )
    {
        std::cout << "SetPixelFormat: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    XnMapOutputMode imgMode;
    imgMode.nXRes = 640; // XN_VGA_X_RES
    imgMode.nYRes = 480; // XN_VGA_Y_RES
    imgMode.nFPS = 30;
    rc = imageGen_.SetMapOutputMode(imgMode);
    if( rc != XN_STATUS_OK )
    {
        std::cout << "image SetMapOutputMode: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    rc = depthGen_.SetMapOutputMode(imgMode);
    if( rc != XN_STATUS_OK )
    {
        std::cout << "depth SetMapOutputMode: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    depthGen_.GetMetaData(depthMd_);
    std::cout << "Depth offset " << depthMd_.XOffset() << " " << depthMd_.YOffset() << std::endl;

    // set the depth image viewpoint
    depthGen_.GetAlternativeViewPointCap().SetViewPoint(imageGen_);

    // read off the depth camera field of view.  This is the FOV corresponding to
    // the IR camera viewpoint, regardless of the alternative viewpoint settings.
    XnFieldOfView fov;
    rc = depthGen_.GetFieldOfView(fov);
    std::cout << "Fov: " << fov.fHFOV << " " << fov.fVFOV << std::endl;

    pDepthData_ = new char [640 * 480];
    pRgbData_ = new char [640 * 480 * 3];

    return true;
}

bool DataCapture::startDataCapture()
{
    XnStatus rc = context_.StartGeneratingAll();
    if( rc != XN_STATUS_OK )
    {
        std::cout << "StartGeneratingAll: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    return true;
}

bool DataCapture::stopDataCapture()
{
    context_.StopGeneratingAll();
    context_.Shutdown(); // do once. should probably be in uninitialise()
    return true;
}

bool DataCapture::captureOne()
{
    XnStatus rc = context_.WaitAndUpdateAll(); // want this to be WaitOneUpdateAll(RGB image)
    if( rc != XN_STATUS_OK )
    {
        std::cout << "WaitAndUpdateAll: " << xnGetStatusString(rc) << std::endl;
        return false;
    }

    // grab image
    imageGen_.GetMetaData(imageMd_);
    const XnRGB24Pixel* rgbData = imageMd_.RGB24Data();
    for( unsigned int i = 0; i < 640 * 480; ++i )
    {
        pRgbData_[3*i] = rgbData->nRed;
        pRgbData_[3*i + 1] = rgbData->nGreen;
        pRgbData_[3*i + 2] = rgbData->nBlue;
        ++rgbData;
    }

    // grab depth image
    depthGen_.GetMetaData(depthMd_);
    const uint16_t* pDepthDataU16 = depthMd_.Data();
    for( int i = 0; i < 640 * 480; ++i)
    {
        uint16_t d = pDepthDataU16[i];
        if( d != 0 )
        {
            pDepthData_[i] = (d * 255)/2048;
        }
        else
        {
            pDepthData_[i] = 0; // should be NAN
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    DataCapture cap;
    if( !cap.initialise() )
    {
        std::cout << "initialise() failed" << std::endl;
        return -1;
    }

    if( !cap.startDataCapture() )
    {
        std::cout << "startDataCapture() failed" << std::endl;
        return -1;
    }

    QImage rgb( (unsigned char*)cap.getRgbImage(), 640, 480, QImage::Format_RGB888);
    QImage depth( (unsigned char*)cap.getDepthImage(), 640, 480, QImage::Format_Indexed8);
    for(int i = 0; i < 256; ++i)
    {
        QColor c(i,i,i);
        depth.setColor ( i, c.rgb() ); //build color table
    }

    QLabel rgbLabel;
    QLabel depthLabel;

    rgbLabel.resize(640, 480);
    rgbLabel.show();

    depthLabel.resize(640, 480);
    depthLabel.show();

    while( 1 )
    {
        if( !cap.captureOne() )
        {
            std::cout << "captureOne failed" << std::endl;
            break;
        }

        rgbLabel.setPixmap( QPixmap::fromImage(rgb) );
        rgbLabel.update();

        depthLabel.setPixmap( QPixmap::fromImage(depth) );
        depthLabel.update();

        app.processEvents();

        // close any window to exit
        if( !depthLabel.isVisible() || !rgbLabel.isVisible() )
        {
            break;
        }
    }

    cap.stopDataCapture();
    std::cout << "Bye!" << std::endl;
    return 0;
}

