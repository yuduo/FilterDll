#pragma once

#include "stdafx.h"

//#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "dcmtk/config/osconfig.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/dcmjpeg/djdecode.h"
#include "dcmtk/dcmdata/dcrledrg.h"
#include "dcmtk/dcmimage/diregist.h"

#include <objidl.h>
#include <windows.h>
using namespace std;
#include <gdiplus.h>
using namespace Gdiplus;
using namespace cv;
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#define MATHLIBRARY_EXPORTS
#ifdef MATHLIBRARY_EXPORTS  
#define MATHLIBRARY_API __declspec(dllexport)   
#else  
#define MATHLIBRARY_API __declspec(dllimport)   
#endif  

namespace FilterLibrary
{
	// This class is exported from the MathLibrary.dll  
	class Filter
	{
	
		
	public:
		// read file
		MATHLIBRARY_API Bitmap* ReadFile(std::string fileName);//tif
		MATHLIBRARY_API Bitmap* ReadDCMFile(std::string fileName);
		MATHLIBRARY_API Bitmap* ConvertToBitMap(cv::Mat & i_Mat);
		cv::Mat m_mat;

		MATHLIBRARY_API Bitmap* ToMedia(int i);//i 1~50
		MATHLIBRARY_API Bitmap* ToMean(int i);//i 1~50
		MATHLIBRARY_API Bitmap* ToLaplacian();
		MATHLIBRARY_API Bitmap* ToGaussian(int i);//i 1~50
		MATHLIBRARY_API Bitmap* ToBilateral(int i);//i 1~50
		MATHLIBRARY_API Bitmap* ToSobel();
		MATHLIBRARY_API Bitmap* ToFilter2D(int i);//i 1~20
		MATHLIBRARY_API Bitmap* ToBlur(int i);//i 1~50
		MATHLIBRARY_API Bitmap* ToHistogram(int sigma,int median);//
		MATHLIBRARY_API Bitmap* ToContrast(int alpha, int beta);//
		MATHLIBRARY_API Bitmap* ToThreshold(int value);//value 0~255
		MATHLIBRARY_API Bitmap* ToCLAHE();
		MATHLIBRARY_API Bitmap* ToMin(int size);
		MATHLIBRARY_API Bitmap* ToMax(int size);
		MATHLIBRARY_API Bitmap* ToBright(int alpha,int beta);//0~500,0~200
		
		MATHLIBRARY_API Bitmap* ToGamma(int gamma);//0~200
		MATHLIBRARY_API Bitmap* ToOpening(int kernel);//0~20
		MATHLIBRARY_API Bitmap* ToClosing(int kernel);//0~20

		int Img_bitCount;
		DicomImage* LoadDcmDataSet(std::string file_path);
		std::vector<cv::Mat> GetImageFromDcmDataSet(DicomImage* m_dcmImage);
	};
}
