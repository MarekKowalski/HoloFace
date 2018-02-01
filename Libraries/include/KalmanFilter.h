#pragma once

#include "mat.h"

namespace FaceProcessing
{
	class Kalman
	{
	public:
		Mat<float> x;
		Mat<float> F, H, Q, R;
		Mat<float> P;

		Kalman();
		Kalman(int nStateDims, int nMeasurements);
		Mat<float> predict();
		Mat<float> update(const Mat<float> &z);
		void reset();
	};

	public ref class PositionFilter sealed
	{
	public:
		PositionFilter(float sigmaA, float sigmaM, float decay);
		float Step(float x, float deltaT);
		float GetFuturePosition(float futureT);
		float GetFutureDisplacement(float futureT);
		void Reset();
	private:
		void UpdateDeltaT(float deltaT);
		float GetAlpha(float deltaT);

		bool initialized;
		Kalman kalman;

		float sigmaA, sigmaM, decay;
	};
}