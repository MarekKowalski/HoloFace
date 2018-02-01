#pragma once

#include "mat.h"

namespace FaceProcessing {

	/// 3D shape pose parameters
	struct d3Pose
	{
		d3Pose();

		Mat<float> rotVec();

		Mat<float> R;
		Mat<float> t3;
		Mat<float> weightsW3;

	};

	public ref class ModelFitter sealed
	{
	public:
		ModelFitter(const Platform::Array<float>^ mean3DShape, Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IVector<float>^>^ blendshapes,
			const Platform::Array<int>^ idxs2D, const Platform::Array<int>^ idxs3D, const Platform::Array<float>^ K);
		void Fit3D(const Platform::Array<float>^ landmarks,
			float* rotX, float* rotY, float* rotZ,
			float* tX, float* tY, float* tZ,
			Platform::Array<float>^* blendshapeWeights, int nIters);
		void ResetPose();

	private:
		Mat<float> mean3DShape;
		Mat<float> mean3DShapeSelected;
		std::vector<Mat<float>> blendshapes;
		std::vector<Mat<float>> blendshapesSelected;
		Mat<int> idxs2D, idxs3D;
		Mat<float> K;

		d3Pose lastPose;
	};



	d3Pose fit3D(Mat<float> &initShape, Mat<float> &mean3, std::vector<Mat<float>> &modes3, Mat<float> &K, d3Pose &initPose, int maxIter = 10);

	void get3DGaussNewton(Mat<float> &mean3, std::vector<Mat<float>> &modes3, Mat<float> &curShape,
		Mat<float> &curShape3, d3Pose &curPose, Mat<float> &K,
		Mat<float> &SDf2, Mat<float> &H3);

	void updatePose(d3Pose &pose, Mat<float> &direction);
	Mat<float> getInitialR(Mat<float> &init, Mat<float> &shape3);
	Mat<float> getInitialT(Mat<float> &init, Mat<float> &shape3, Mat<float> &K, float dist);
	Mat<float> getInitialP(Mat<float> &init, Mat<float> &shape3, float *scale = NULL);


	Mat<float> projectShape(d3Pose &pose, Mat<float> &K, Mat<float> &shape3D);

	Mat<float> cross(Mat<float> v1, Mat<float> v2);
	Mat<float> rotationToVector(Mat<float> &R);
	Mat<float> vectorToRotation(Mat<float> &v);
}