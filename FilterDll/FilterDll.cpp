#include "stdafx.h"  
#include "FilterDll.h"  

namespace FilterLibrary
{
	Bitmap* Filter::ConvertToBitMap(cv::Mat & i_Mat)
	{
		
		PixelFormat e_Format;
		switch (i_Mat.channels())
		{
		case 1: e_Format = PixelFormat8bppIndexed; break;
		case 3: e_Format = PixelFormat24bppRGB;    break;
		case 4: e_Format = PixelFormat32bppARGB;   break;
		default: throw L"Image format not supported.";
		}

		// Create Bitmap with own memory
		Bitmap* pi_Bmp = new Bitmap(i_Mat.cols, i_Mat.rows, e_Format);

		BitmapData i_Data;
		Gdiplus::Rect k_Rect(0, 0, i_Mat.cols, i_Mat.rows);
		if (Ok != pi_Bmp->LockBits(&k_Rect, ImageLockModeWrite, e_Format, &i_Data))
		{
			delete pi_Bmp;
			throw L"Error locking Bitmap.";
		}

		if (i_Mat.elemSize1() == 1) // 1 Byte per channel (8 bit gray scale palette)
		{
			BYTE* u8_Src = i_Mat.data;
			BYTE* u8_Dst = (BYTE*)i_Data.Scan0;

			int s32_RowLen = i_Mat.cols * i_Mat.channels(); // != i_Mat.step !!

			// The Windows Bitmap format requires all rows to be DWORD aligned (always!)
			// while OpenCV by default stores bitmap data sequentially.
			for (int R = 0; R < i_Mat.rows; R++)
			{
				memcpy(u8_Dst, u8_Src, s32_RowLen);
				u8_Src += i_Mat.step;    // step may be e.g 3729
				u8_Dst += i_Data.Stride; // while Stride is 3732
			}
		}
		else // i_Mat may contain e.g. float data (CV_32F -> 4 Bytes per pixel grayscale)
		{
			int s32_Type;
			switch (i_Mat.channels())
			{
			case 1: s32_Type = CV_8UC1; break;
			case 3: s32_Type = CV_8UC3; break;
			default: throw L"Image format not supported.";
			}

			CvMat i_Dst;
			cvInitMatHeader(&i_Dst, i_Mat.rows, i_Mat.cols, s32_Type, i_Data.Scan0, i_Data.Stride);

			CvMat i_Img = i_Mat;
			cvConvertImage(&i_Img, &i_Dst, 0);
		}

		pi_Bmp->UnlockBits(&i_Data);

		
		return pi_Bmp;
	}

	Bitmap* Filter::ReadFile(std::string fileName)
	{

		m_mat = cv::imread(fileName);
		return ConvertToBitMap(m_mat);
	}


	Bitmap* Filter::ToMedia(int i)
	{

		medianBlur(m_mat, m_mat, i);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToMean(int i)
	{

		blur(m_mat, m_mat, cv::Size(i, i));
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToLaplacian()
	{

		Laplacian(m_mat, m_mat, CV_64F);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToGaussian(int i)
	{

		// Gaussian smoothing
		GaussianBlur(m_mat, m_mat, cv::Size(i, i), 0, 0);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToBilateral(int i)
	{

		// Bilateral smoothing
		bilateralFilter(m_mat, m_mat, i, i * 2, i * 2);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToSobel()
	{

		Sobel(m_mat, m_mat, CV_64F, 1, 0);
		return ConvertToBitMap(m_mat);
	}
}