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
	Bitmap* Filter::ToFilter2D(int i)
	{
		
		cv::Mat kernel = cv::Mat::ones(i, i, CV_32F) / (float)(i*i);
		filter2D(m_mat, m_mat, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToBlur(int i)
	{
		blur(m_mat, m_mat, cv::Size(i, i));
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToHistogram(int sigma, int median)
	{
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToContrast(int alpha, int beta)
	{
		for (int y = 0; y < m_mat.rows; y++) {
			for (int x = 0; x < m_mat.cols; x++) {
				for (int c = 0; c < 3; c++) {
					m_mat.at<uchar>(y, x) =
						cv::saturate_cast<uchar>(alpha*(m_mat.at<uchar>(y, x)) + beta);
				}
			}
		}
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToThreshold(int value)
	{
		threshold(m_mat, m_mat, value, 255, 0);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToCLAHE()
	{
		cv::Mat image;
		cv::Mat lab_image;
		cvtColor(m_mat, image, cv::COLOR_GRAY2RGB, 3);
		cvtColor(image, lab_image, CV_BGR2Lab);

		// 提取L通道
		vector<cv::Mat> lab_planes(3);
		split(lab_image, lab_planes);

		// CLAHE 算法
		Ptr<CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		Mat dst;
		clahe->apply(lab_planes[0], dst);
		dst.copyTo(lab_planes[0]);
		merge(lab_planes, lab_image);

		// 恢复RGB图像
		Mat image_clahe;
		cvtColor(lab_image, image_clahe, CV_Lab2BGR);
		cvtColor(image_clahe, m_mat, cv::COLOR_RGB2GRAY, 1);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToMin(int size)
	{
		int erosion_type;
		{ erosion_type = MORPH_RECT; }
		/*else if (erosion_elem == 1) { erosion_type = MORPH_CROSS; }
		else if (erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }*/

		Mat element = getStructuringElement(erosion_type,
			cv::Size(2 * size + 1, 2 * size + 1),
			cv::Point(size, size));

		/// Apply the erosion operation
		erode(m_mat, m_mat, element);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToMax(int size)
	{
		int dilation_type;
		{ dilation_type = MORPH_RECT; }
		/*else if (dilation_elem == 1) { dilation_type = MORPH_CROSS; }
		else if (dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }*/

		Mat element = getStructuringElement(dilation_type,
			cv::Size(2 * size + 1, 2 * size + 1),
			cv::Point(size, size));
		/// Apply the dilation operation
		dilate(m_mat, m_mat, element);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToBright(int alpha, int beta)
	{
		double alpha_value = alpha / 100.0;
		int beta_value = beta - 100;
		

		m_mat.convertTo(m_mat, -1, alpha_value, beta_value);

		
		return ConvertToBitMap(m_mat);
	}
	
	Bitmap* Filter::ToGamma(int gamma)
	{
		double gamma_value = gamma / 100.0;
		//! [changing-contrast-brightness-gamma-correction]
		Mat lookUpTable(1, 256, CV_8U);
		uchar* p = lookUpTable.ptr();
		for (int i = 0; i < 256; ++i)
			p[i] = saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);

		Mat res = m_mat.clone();
		LUT(m_mat, lookUpTable, m_mat);
		//! [changing-contrast-brightness-gamma-correction]


		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToOpening(int kernel)
	{
		int operation = 0 + 2;

		Mat element = getStructuringElement(0, cv::Size(2 * kernel + 1, 2 * kernel + 1), cv::Point(kernel, kernel));

		/// Apply the specified morphology operation
		morphologyEx(m_mat, m_mat, operation, element);
		return ConvertToBitMap(m_mat);
	}
	Bitmap* Filter::ToClosing(int kernel)
	{
		int operation = 1 + 2;

		Mat element = getStructuringElement(0, cv::Size(2 * kernel + 1, 2 * kernel + 1), cv::Point(kernel, kernel));

		/// Apply the specified morphology operation
		morphologyEx(m_mat, m_mat, operation, element);
		return ConvertToBitMap(m_mat);
	}
}