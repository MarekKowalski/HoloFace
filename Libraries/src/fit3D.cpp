#include "Fit3D.h"
#include <vector>

#define M_PI 3.14

namespace FaceProcessing 
{
	ModelFitter::ModelFitter(const Platform::Array<float>^ mean3DShape, Windows::Foundation::Collections::IVector<Windows::Foundation::Collections::IVector<float>^>^ blendshapes,
		const Platform::Array<int>^ idxs2D, const Platform::Array<int>^ idxs3D, const Platform::Array<float>^ K)
	{
		int nVertices = mean3DShape->Length / 3;
		int nBlendshapes = blendshapes->Size;

		this->mean3DShape = Mat<float>(nVertices, 3);
		this->blendshapes.resize(nBlendshapes);
		this->blendshapesSelected.resize(nBlendshapes);

		this->idxs2D = Mat<int>(idxs2D->Length, 1);
		this->idxs3D = Mat<int>(idxs2D->Length, 1);
		for (int i = 0; i < this->idxs2D.height; i++)
			this->idxs2D[i][0] = idxs2D[i];
		for (int i = 0; i < this->idxs3D.height; i++)
			this->idxs3D[i][0] = idxs3D[i];

		for (int i = 0; i < nVertices; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				this->mean3DShape[i][j] = mean3DShape[i * 3 + j];
			}
		}
		this->mean3DShapeSelected = this->mean3DShape.subsetRows(this->idxs3D);

		for (int k = 0; k < nBlendshapes; k++)
		{
			this->blendshapes[k] = Mat<float>(nVertices, 3);
			for (int i = 0; i < nVertices; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					this->blendshapes[k][i][j] = blendshapes->GetAt(k)->GetAt(i * 3 + j);
				}
			}

			this->blendshapesSelected[k] = this->blendshapes[k].subsetRows(this->idxs3D);
		}

		this->K = Mat<float>(3, 3);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				this->K[i][j] = K[i * 3 + j];
			}
		}

		this->K.print();
	}

	void ModelFitter::Fit3D(const Platform::Array<float>^ landmarks,
		float* rotX, float* rotY, float* rotZ,
		float* tX, float* tY, float* tZ,
		Platform::Array<float>^* blendshapeWeights, int nIters)
	{
		int nLandmarks = landmarks->Length / 2;
		Mat<float> matLandmarks(nLandmarks, 2);

		for (int i = 0; i < nLandmarks; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				matLandmarks[i][j] = landmarks[i *2 + j];
			}
		}

		matLandmarks = matLandmarks.subsetRows(idxs2D);

		d3Pose pose = fit3D(matLandmarks, mean3DShapeSelected, blendshapesSelected, K, lastPose, nIters);

		Mat<float> rotVec = pose.rotVec();
		*rotX = rotVec.data[0];
		*rotY = rotVec.data[1];
		*rotZ = rotVec.data[2];

		*tX = pose.t3.data[0];
		*tY = pose.t3.data[1];
		*tZ = pose.t3.data[2];

		*blendshapeWeights = ref new Platform::Array<float>(pose.weightsW3.height);

		for (int i = 0; i < pose.weightsW3.height; i++)
		{
			(*blendshapeWeights)[i] = pose.weightsW3.data[i];
		}
		
		lastPose = pose;
	}

	void ModelFitter::ResetPose()
	{
		lastPose = d3Pose();
	}

	d3Pose fit3D(Mat<float>&initShape, Mat<float> &mean3, std::vector<Mat<float>> &modes3, Mat<float> &K, d3Pose &initPose, int maxIter)
	{
		d3Pose curPose;
		Mat<float> curShape3;

		//no initial pose
		if (initPose.weightsW3.height == 0)
		{
			curPose.R = 0;
			curPose.weightsW3 = 0;
			curShape3 = mean3;

			curPose.t3 = getInitialT(initShape, mean3, K, 1.2f);


			curPose.weightsW3 = Mat<float>(modes3.size(), 1);
			curPose.weightsW3 = 0;

			Mat<float> tempVec(3, 1);
			tempVec.data[1] = -3.14f;
			curPose.R = vectorToRotation(tempVec);
		}
		//initial pose provided
		else
		{
			curPose = initPose;
			curShape3 = mean3;

			for (unsigned int i = 0; i < modes3.size(); i++)
			{
				Mat<float> temp = modes3[i] * curPose.weightsW3.data[i];
				curShape3 = curShape3 + temp;
			}
		}

		Mat<float> J, H;
		Mat<float> err3;
		for (int it = 0; it < maxIter; it++)
		{
			get3DGaussNewton(mean3, modes3, initShape, curShape3, curPose, K, J, H);

			err3 = initShape - projectShape(curPose, K, curShape3);
			err3 = err3.flatten();

			Mat<float> direction;
			Mat<float> gradient;
			gradient = J * err3.trans();

			direction = H.solve(gradient);

			for (int j = 0; j < 10; j++)
			{
				d3Pose tempPose = curPose;
				updatePose(tempPose, direction);
				curShape3 = mean3;

				for (unsigned int i = 0; i < modes3.size(); i++)
				{
					Mat<float> temp = modes3[i] * tempPose.weightsW3.data[i];
					curShape3 = curShape3 + temp;
				}

				Mat<float> err3Test = initShape - projectShape(tempPose, K, curShape3);
				err3Test = err3Test.flatten();

				//sufficient decrease condition, 10e-4 as in L-BFGS-B
				float suffDecrease = (direction.trans() * gradient)[0][0] * 10e-4f;
				if (err3Test.norm() + suffDecrease < err3.norm())
				{
					curPose = tempPose;
					break;
				}
				else
					direction = direction / 2;
			}
		}
		return curPose;
	}

	Mat<float> rotationGradient(d3Pose &curPose, Mat<float> &K, Mat<float> &cur3DShape, int axis)
	{
		float increment = 10e-4f;
		Mat<float> tempRotVec = curPose.rotVec();
		tempRotVec.data[axis] += increment;
		Mat<float> tempR = vectorToRotation(tempRotVec);
		d3Pose tempPose = curPose;
		tempPose.R = tempR;

		Mat<float> originalProjection = projectShape(curPose, K, cur3DShape);
		Mat<float> incrementProjection = projectShape(tempPose, K, cur3DShape);

		return (incrementProjection - originalProjection) / increment;
	}

	Mat<float> translationGradient(d3Pose &curPose, Mat<float> &K, Mat<float> &cur3DShape, int axis)
	{
		float increment = 10e-4f;
		d3Pose tempPose = curPose;
		tempPose.t3.data[axis] += increment;

		Mat<float> originalProjection = projectShape(curPose, K, cur3DShape);
		Mat<float> incrementProjection = projectShape(tempPose, K, cur3DShape);

		return (incrementProjection - originalProjection) / increment;
	}

	Mat<float> blendshapeGradient(d3Pose &curPose, Mat<float> &K, Mat<float> &cur3DShape, Mat<float> &blendshape)
	{
		float increment = 10e-4f;
		Mat<float> originalProjection = projectShape(curPose, K, cur3DShape);
		Mat<float> incrementProjection = projectShape(curPose, K, cur3DShape + blendshape * increment);

		return (incrementProjection - originalProjection) / increment;
	}

	void get3DGaussNewton(Mat<float> &mean3, std::vector<Mat<float>> &modes3, Mat<float> &curShape,
		Mat<float> &curShape3, d3Pose &curPose, Mat<float> &K,
		Mat<float> &SDf2, Mat<float> &H3)
	{
		std::vector<Mat<float>> Sdf(modes3.size() + 6, Mat<float>(curShape.height, curShape.width));
		int addToIndex = 0;

		for (unsigned int i = 0; i < modes3.size(); i++)
		{
			Sdf[i + addToIndex] = blendshapeGradient(curPose, K, curShape3, modes3[i]);
		}

		addToIndex += modes3.size();

		Sdf[addToIndex + 0] = rotationGradient(curPose, K, curShape3, 0);
		Sdf[addToIndex + 1] = rotationGradient(curPose, K, curShape3, 1);
		Sdf[addToIndex + 2] = rotationGradient(curPose, K, curShape3, 2);
		addToIndex += 3;

		Sdf[addToIndex + 0] = translationGradient(curPose, K, curShape3, 0);
		Sdf[addToIndex + 1] = translationGradient(curPose, K, curShape3, 1);
		Sdf[addToIndex + 2] = translationGradient(curPose, K, curShape3, 2);
		addToIndex += 3;

		SDf2.setSize(Sdf.size(), Sdf[0].width * Sdf[0].height);
		Mat<float> flat;
		SDf2 = 0;
		for (int i = 0; i < SDf2.height; i++)
		{
			flat = Sdf[i].flatten();
			for (int j = 0; j < flat.width; j++)
				SDf2[i][j] = flat.data[j];
		}

		H3 = (SDf2 * SDf2.trans());
	}

	void updatePose(d3Pose &pose, Mat<float> &direction)
	{
		int startInd = 0;

		pose.weightsW3 += direction.subMat(startInd, startInd + pose.weightsW3.height, 0, direction.width);

		startInd += pose.weightsW3.height;

		Mat<float> rotVect = pose.rotVec();
		rotVect.data[0] += direction.data[startInd + 0];
		rotVect.data[1] += direction.data[startInd + 1];
		rotVect.data[2] += direction.data[startInd + 2];
		pose.R = vectorToRotation(rotVect);
		startInd += 3;

		pose.t3.data[0] += direction.data[startInd + 0];
		pose.t3.data[1] += direction.data[startInd + 1];
		pose.t3.data[2] += direction.data[startInd + 2];
	}

	Mat<float> getInitialR(Mat<float> &init, Mat<float> &shape3)
	{
		Mat<float> P;
		Mat<float> zeros(init.height, 1);
		zeros = 0;

		Mat<float> init3 = init.hstack(zeros);
		Mat<float> shape3Centered = shape3 - shape3.meanRow();
		Mat<float> init3Centered = init3 - init3.meanRow();
		float s = init3Centered.norm(0, 3, 0, 2) / shape3Centered.norm(0, 3, 0, 2);

		Mat<float> shape3CenteredScaled = shape3Centered * s;
		Mat<float> intermediateMat = init3Centered.trans() * shape3CenteredScaled;

		Mat<float> U, D, V;

		intermediateMat.svd(U, D, V);
		Mat<float> R;
		R = U*V;

		return R;
	}

	Mat<float> getInitialT(Mat<float> &init, Mat<float> &shape3, Mat<float> &K, float dist)
	{
		Mat<float> initShapePt = init.subMat(0, 1, 0, 2).trans();
		Mat<float> initShapePtHomo = initShapePt.vstack(Mat<float>::ones(1, 1));
		Mat<float> ray = K.inv() * initShapePtHomo;
		ray = ray / ray.norm();

		Mat<float> shape3Pt = shape3.subMat(0, 1, 0, 3).trans();

		return ray * dist - shape3Pt;
	}

	Mat<float> getInitialP(Mat<float> &init, Mat<float> &shape3, float *scale)
	{
		Mat<float> P;
		Mat<float> zeros(init.height, 1);
		zeros = 0;

		Mat<float> init3 = init.hstack(zeros);
		Mat<float> shape3b = shape3 - shape3.meanRow();
		Mat<float> init3b = init3 - init3.meanRow();
		float s = init3b.norm(0, 3, 0, 2) / shape3b.norm(0, 3, 0, 2);
		shape3b = shape3b * s;

		shape3b = init3b.trans() * shape3b;

		Mat<float> U, D, V;

		shape3b.svd(U, D, V);
		Mat<float> R;
		R = U*V;


		P.setSize(2, 3);

		P.copyRow(R, 0, 0);
		P.copyRow(R, 1, 1);

		if (scale != NULL)
			*scale = s;

		return P;
	}

	Mat<float> projectShape(d3Pose &pose, Mat<float> &K, Mat<float> &shape3D)
	{
		Mat<float> projShapeHomo = (K * (pose.R * shape3D.trans() + pose.t3)).trans();
		Mat<float> projShape(projShapeHomo.height, 2);

		for (int i = 0; i < projShapeHomo.height; i++)
		{
			projShape[i][0] = projShapeHomo[i][0] / projShapeHomo[i][2];
			projShape[i][1] = projShapeHomo[i][1] / projShapeHomo[i][2];
		}

		return projShape;
	}

	Mat<float> rotationToVector(Mat<float> &R)
	{
		Mat<float> v(3, 1);
		float angle = acos((R.trace() - 1) / 2);

		if (abs(angle) < 10e-5)
			return v;

		v[0][0] = R[2][1] - R[1][2];
		v[1][0] = R[0][2] - R[2][0];
		v[2][0] = R[1][0] - R[0][1];
		v = v * (angle / (2 * sin(angle)));

		if (v.norm() > 3.14f)
		{
			v = v * ((v.norm() - 2.0f * 3.14f) / v.norm());
		}

		return v;
	}

	Mat<float> vectorToRotation(Mat<float> &v)
	{
		if (v.norm() > 3.14f)
		{
			v = v * ((v.norm() - 2.0f * 3.14f) / v.norm());
		}

		Mat<float> unitV = v / v.norm();
		float angle = v.norm();

		Mat<float> K(3, 3);
		K[0][1] = -unitV[2][0];
		K[0][2] = unitV[1][0];
		K[1][0] = unitV[2][0];
		K[1][2] = -unitV[0][0];
		K[2][0] = -unitV[1][0];
		K[2][1] = unitV[0][0];

		Mat<float> R = Mat<float>::eye(3) + K * sin(angle) + (K * K) * (1 - cos(angle));

		return R;
	}

	d3Pose::d3Pose()
	{
		R.setSize(3, 3);
		R = 0;
		t3.setSize(3, 1);
		t3 = 0;
		weightsW3.setSize(0, 0);
	}

	Mat<float> d3Pose::rotVec()
	{
		return rotationToVector(R);
	}
}