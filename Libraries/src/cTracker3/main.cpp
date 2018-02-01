#include <stdio.h>
#include <string>
#include <opencv2/opencv.hpp>

#include "mat.h"
#include "utils.h"
#include "FaceAlignment.h"
#include "FaceInitialization.h"

#include "process_video.h"
#include "FileUtils.h"

#include "fit3D.h"


using namespace std;

#define RANKING_OK 1500

void toMat(cv::Mat &img, Mat<unsigned char> &img2)
{
	for (int i = 0; i < img2.height; i++)
	{
		for (int j = 0; j < img2.width; j++)
		{
			img2.mat[i][j] = (img.at<cv::Vec3b>(i, j)[0] + img.at<cv::Vec3b>(i, j)[1] + img.at<cv::Vec3b>(i, j)[2]) / 3;			
		}
	}
}

cv::Mat toOpenCV(Mat<unsigned char> img)
{
	cv::Mat res(img.height, img.width, CV_8UC1);
	for (int i = 0; i < img.height; i++)
	{
		for (int j = 0; j < img.width; j++)
			res.at<char>(i, j) = img.mat[i][j];// * 255;
	}

	return res;
}

void drawShape(Mat<float> &shape, cv::Mat &img, cv::Scalar color)
{
	for (int i = 0; i < shape.height; i++)
	{
		cv::Point center((int)shape.mat[i][0], (int)shape.mat[i][1]);
		cv::circle(img, center, 2, color);
	}

	for (int i = 0; i < 16; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}

	for (int i = 17; i < 21; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}

	for (int i = 22; i < 26; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}

	for (int i = 27; i < 30; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}

	for (int i = 31; i < 35; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}

	for (int i = 36; i < 41; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}
	cv::Point pt1(shape[36][0], shape[36][1]);
	cv::Point pt2(shape[41][0], shape[41][1]);
	cv::line(img, pt1, pt2, color);

	for (int i = 42; i < 47; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}
	pt1 = cv::Point(shape[42][0], shape[42][1]);
	pt2 = cv::Point(shape[47][0], shape[47][1]);
	cv::line(img, pt1, pt2, color);

	for (int i = 48; i < 59; i++)
	{
		cv::Point pt1(shape[i][0], shape[i][1]);
		cv::Point pt2(shape[i + 1][0], shape[i + 1][1]);
		cv::line(img, pt1, pt2, color);
	}
	pt1 = cv::Point(shape[48][0], shape[48][1]);
	pt2 = cv::Point(shape[59][0], shape[59][1]);
	cv::line(img, pt1, pt2, color);
}

void camera(APR &apr, APR3D &apr3d, FaceAlignment &alignment)
{
	bool reset = true;
	cv::VideoCapture capture(0);
	//capture.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	//capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	cv::Size s = cv::Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT));
	cv::VideoWriter writer("out.avi", 0, 25, s);


	cv::CascadeClassifier faceCascade;

	faceCascade.load("../data/models/haarcascade_frontalface_alt2.xml");

	Mat<float> fitted;
	cv::Mat img;
	capture.read(img);

	int frameNum = 0;

	float counter = 0;
	float ranking = 0;

	//shake reduction
	Mat<float> pos, prev_pos;
	float distance;
	int wait = 0;
	const int max_n_frames = 3;
	int n_frames = max_n_frames;

	deque<Mat<float>> queue(n_frames);
	float face_size, thr;
	//end shake reduction

	while (true)
	{
		frameNum++;
		if (frameNum % 20 == 0)
		{
			std::cout << "Time: " << counter / (20 * (double)(CLOCKS_PER_SEC / 1000)) << " ms" << std::endl;
			counter = 0;
		}

		capture.read(img);
		writer.write(img);

		cv::Mat imgGray;
		cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

		if (reset)
		{
			std::vector<cv::Rect> faces;
			faceCascade.detectMultiScale(imgGray, faces, 1.2, 3, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(50, 50));

			if (faces.size() != 0)
			{
				Mat<unsigned char> img2(img.rows, img.cols, imgGray.data);

				MyRectangle rect;
				rect.x = faces[0].x;
				rect.y = faces[0].y;
				rect.width = faces[0].width;
				rect.height = faces[0].height;

				fitted = apr.fit(img2, rect, 2);
				fitted = apr3d.fit(img2, fitted, 1);
				fitted = alignment.fitWithAffine(img2, fitted, 4);
				ranking = alignment.getRanking(img2, fitted);

				if (ranking < RANKING_OK)
					reset = false;

				//shake reduction
				prev_pos = fitted.meanRow();
				n_frames = max_n_frames;

				for (int i = 0; i < max_n_frames; i++)
					queue[i] = fitted;
				//end shake reduction

				drawShape(fitted, img, cv::Scalar(0, 0, 255));
			}
		}
		else
		{
			Mat<unsigned char> img2(img.rows, img.cols, imgGray.data);

			fitted = apr.fit(img2, fitted, 2);
			//drawShape(fitted, img, cv::Scalar(255, 0, 0));
			fitted = apr3d.fit(img2, fitted, 1);
			//drawShape(fitted, img, cv::Scalar(0, 255, 0));

			//fit3D::d3Pose pose;
			//pose = fit3D::fit3D(fitted, apr3d.meanShape3D, vector<Mat<float>>(), pose);
			//fitted = apr3d.meanShape3D * pose.P.trans() * pose.scale + pose.t3;

			fitted = alignment.fitWithAffine(img2, fitted, 4);
			ranking = alignment.getRanking(img2, fitted);

			if (ranking > RANKING_OK)
				reset = true;

			//shake reduction
			pos = fitted.meanRow();
			distance = sqrt(pow(pos.data[0] - prev_pos.data[0], 2.0) + pow(pos.data[1] - prev_pos.data[1], 2.0));

			Mat<float> max_row = fitted.maxRow();
			Mat<float> min_row = fitted.minRow();

			face_size = sqrt(pow(max_row.data[0] - min_row.data[0], 2) + pow(max_row.data[1] - min_row.data[1], 2));
			thr = face_size / 80.0f;
			if (thr<2.0f) thr = 2.0f;

			if (distance > thr)
			{
				wait = 0;
				n_frames = 1;
			}
			else
			{
				wait++;
				if (wait > 2)
				{
					n_frames = min(max_n_frames, n_frames + 1);
				}
			}

			queue.pop_back();
			queue.push_front(fitted);

			Mat<float> new_fitted(fitted.height, 2);

			for (int i = 0; i < n_frames; i++)
				new_fitted = new_fitted + queue[i];

			new_fitted = new_fitted / n_frames;

			fitted = new_fitted;

			prev_pos = pos;
			//end shake reduction

			drawShape(fitted, img, cv::Scalar(0, 0, 255));
		}


		//printf("Ranking: %f\n", ranking);

		cv::imshow("test", img);
		char key = 0xFF & cv::waitKey(1);
		if (key == 27)
			break;
		else if (key == 32)
			reset = true;
	}
	capture.release();
	writer.release();
}

void perfTest(APR &apr, APR3D &apr3d, FaceAlignment &tracker)
{
	getchar();
	cv::Mat img = cv::imread("../z dyplomu.jpg");
	Mat<uchar> img2(img.rows, img.cols);
	toMat(img, img2);
	Mat<float> shape = tracker.meanS;
	Mat<float> fitted;
	Mat<float> offset(1, 2);
	offset[0][0] = 125;
	offset[0][1] = 210;
	shape = shape + offset;

	for (int i = 0; i < 100; i++)
	{
		fitted = apr.fit(img2, shape, 2);
		fitted = apr3d.fit(img2, fitted, 1);
		fitted = tracker.fitWithAffine(img2, fitted, 4);
		//cout << tracker.getRanking(img2, fitted) << endl;
	}

	drawShape(fitted, img, cv::Scalar(0, 0, 255));
	drawShape(shape, img, cv::Scalar(255, 0, 0));

	cv::imshow("Test", img);
	cv::waitKey();

}



int main()
{
	//feature.trans().print();

	APR init("../data/models/APR2/", 2);
	APR3D init3D("../data/models/APR3D2/", 1);
	FaceAlignment alignment("../data/models/face tracking small/", 4);
	//perfTest2(init, init3D, alignment);
	camera(init, init3D, alignment);



	return 0;
}