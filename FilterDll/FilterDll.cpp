#include "stdafx.h"  
#include "FilterDll.h"  



namespace FilterLibrary
{
	DicomImage* Filter::LoadDcmDataSet(std::string file_path)
	{

		DcmFileFormat fileformat;
		OFCondition oc = fileformat.loadFile(file_path.c_str());                    //读取Dicom图像    
		if (!oc.good())     //判断Dicom文件是否读取成功    
		{
			std::cout << "file Load error" << std::endl;
			return nullptr;
		}
		DcmDataset *dataset = fileformat.getDataset();                              //得到Dicom的数据集    
		E_TransferSyntax xfer = dataset->getOriginalXfer();                          //得到传输语法    

		OFString patientname;
		dataset->findAndGetOFString(DCM_PatientName, patientname);                   //获取病人姓名    

		unsigned short bit_count(0);
		dataset->findAndGetUint16(DCM_BitsStored, bit_count);                        //获取像素的位数 bit    

		OFString isRGB;
		dataset->findAndGetOFString(DCM_PhotometricInterpretation, isRGB);           //DCM图片的图像模式    

		unsigned short img_bits(0);
		dataset->findAndGetUint16(DCM_SamplesPerPixel, img_bits);                    //单个像素占用多少byte    
		Img_bitCount = (int)img_bits;

		OFString framecount;
		dataset->findAndGetOFString(DCM_NumberOfFrames, framecount);             //DCM图片的帧数    


		//DicomImage* img_xfer = new DicomImage(xfer, 0, 0, 1);                     //由传输语法得到图像的帧    

		unsigned short m_width;                                                     //获取图像的窗宽高    
		unsigned short m_height;
		dataset->findAndGetUint16(DCM_Rows, m_height);
		dataset->findAndGetUint16(DCM_Columns, m_width);

		/////////////////////////////////////////////////////////////////////////    
		const char* transferSyntax = NULL;
		fileformat.getMetaInfo()->findAndGetString(DCM_TransferSyntaxUID, transferSyntax);       //获得传输语法字符串    
		string losslessTransUID = "1.2.840.10008.1.2.4.70";
		string lossTransUID = "1.2.840.10008.1.2.4.51";
		string losslessP14 = "1.2.840.10008.1.2.4.57";
		string lossyP1 = "1.2.840.10008.1.2.4.50";
		string lossyRLE = "1.2.840.10008.1.2.5";
		if (transferSyntax == losslessTransUID || transferSyntax == lossTransUID ||
			transferSyntax == losslessP14 || transferSyntax == lossyP1)
		{
			DJDecoderRegistration::registerCodecs();
			dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);                       //对压缩的图像像素进行解压    
			DJDecoderRegistration::cleanup();
		}
		else if (transferSyntax == lossyRLE)
		{
			DcmRLEDecoderRegistration::registerCodecs();
			dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
			DcmRLEDecoderRegistration::cleanup();
		}
		else
		{
			dataset->chooseRepresentation(xfer, NULL);
		}

		DicomImage* m_dcmImage = new DicomImage((DcmObject*)dataset, xfer); //利用dataset生成DicomImage，需要上面的解压方法；    

		return m_dcmImage;
	}

	std::vector<cv::Mat> Filter::GetImageFromDcmDataSet(DicomImage* m_dcmImage)
	{
		std::vector<cv::Mat> output_img;              //输出图像向量  
		int framecount(m_dcmImage->getFrameCount()); //获取这个文件包含的图像的帧数  
		for (int k = 0; k < framecount; k++)
		{
			unsigned char *pixelData = (unsigned char*)(m_dcmImage->getOutputData(8, k, 0)); //获得8位的图像数据指针    
			if (pixelData != NULL)
			{
				int m_height = m_dcmImage->getHeight();
				int m_width = m_dcmImage->getWidth();
				cout << "高度：" << m_height << "，长度" << m_width << endl;
				if (3 == Img_bitCount)
				{
					cv::Mat dst2(m_height, m_width, CV_8UC3, cv::Scalar::all(0));
					for (int i = 0; i < m_height; i++)
					{
						for (int j = 0; j < m_width; j++)
						{
							dst2.at<cv::Vec3b>(i, j)[0] = *(pixelData + i * m_width * 3 + j * 3 + 2);   //B channel    
							dst2.at<cv::Vec3b>(i, j)[1] = *(pixelData + i * m_width * 3 + j * 3 + 1);   //G channel    
							dst2.at<cv::Vec3b>(i, j)[2] = *(pixelData + i * m_width * 3 + j * 3);       //R channel    
						}
					}
					output_img.push_back(dst2);
				}
				else if (1 == Img_bitCount)
				{
					cv::Mat dst2(m_height, m_width, CV_8UC1, cv::Scalar::all(0));
					uchar* data = nullptr;
					for (int i = 0; i < m_height; i++)
					{
						data = dst2.ptr<uchar>(i);
						for (int j = 0; j < m_width; j++)
						{
							data[j] = *(pixelData + i * m_width + j);
						}
					}
					output_img.push_back(dst2);
				}

				/*cv::imshow("image", dst2);
				cv::waitKey(0);*/
			}
		}

		return output_img;
	}

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
	Bitmap* Filter::ReadDCMFile(std::string fileName)
	{
		DicomImage *m_dcmImage;
		m_dcmImage = LoadDcmDataSet(fileName);

		vector<cv::Mat> images;
		images = GetImageFromDcmDataSet(m_dcmImage);
		m_mat = images[0];
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