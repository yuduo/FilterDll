#pragma once

#include "stdafx.h"

//#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <objidl.h>
#include <windows.h>
using namespace std;
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

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
		MATHLIBRARY_API Bitmap* ReadFile(std::string fileName);
		MATHLIBRARY_API Bitmap* ConvertToBitMap(cv::Mat & i_Mat);
		cv::Mat m_mat;

		Bitmap* ToMedia(int i);
		Bitmap* ToMean(int i);
		Bitmap* ToLaplacian();
		Bitmap* ToGaussian(int i);
		Bitmap* ToBilateral(int i);
		Bitmap* ToSobel();
	};
}
