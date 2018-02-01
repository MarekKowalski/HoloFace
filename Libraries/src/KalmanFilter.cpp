#include "KalmanFilter.h"

using namespace FaceProcessing;

Kalman::Kalman()
{
}

Kalman::Kalman(int nStateDims, int nMeasurements)
{
	x = Mat<float>(nStateDims, 1);
	F = Mat<float>(nStateDims, nStateDims);
	Q = Mat<float>(nStateDims, nStateDims);
	R = Mat<float>(nMeasurements, nMeasurements);
	H = Mat<float>(nMeasurements, nStateDims);

	P = Mat<float>(nStateDims, nStateDims);
}

Mat<float> Kalman::predict()
{
	x = F * x;
	P = F * P * F.trans() + Q;

	return x;
}

Mat<float> Kalman::update(const Mat<float> &z)
{
	Mat<float> y = z - H * x;
	Mat<float> S = R + H * P * H.trans();
	Mat<float> K = P * H.trans() * S.inv();
	x = x + K * y;
	P = P - K * S * K.trans();

	return x;
}

void Kalman::reset()
{
	x = 0;
	P = 0;
}

PositionFilter::PositionFilter(float sigmaA, float sigmaM, float decay)
{
	this->sigmaA = sigmaA;
	this->sigmaM = sigmaM;
	this->decay = decay;
	initialized = false;

	kalman = Kalman(3, 1);
	kalman.F = Mat<float>::eye(3);
	kalman.H[0][0] = 1.0f;
	kalman.R[0][0] = pow(sigmaM, 2);
}

float PositionFilter::Step(float measurement, float deltaT)
{
	if (!initialized)
	{
		kalman.x[0][0] = measurement;
		initialized = true;

		return measurement;
	}

	UpdateDeltaT(deltaT);
	kalman.predict();

	Mat<float> matMeasurement(1, 1);
	matMeasurement[0][0] = measurement;
	Mat<float> state = kalman.update(matMeasurement);

	return state[0][0];
}

float PositionFilter::GetFuturePosition(float futureT)
{
	UpdateDeltaT(futureT);
	Mat<float> futureState = kalman.F * kalman.x;

	return futureState[0][0];
}

float PositionFilter::GetFutureDisplacement(float futureT)
{
	UpdateDeltaT(futureT);
	Mat<float> state = kalman.x;
	state[0][0] = 0.0f;
	Mat<float> futureState = kalman.F * state;

	return futureState[0][0];
}

void PositionFilter::Reset()
{
	initialized = false;
	kalman.reset();
}

void PositionFilter::UpdateDeltaT(float deltaT)
{
	kalman.F[0][1] = deltaT;
	kalman.F[0][2] = 0.5f * pow(deltaT, 2);
	kalman.F[1][2] = deltaT;
	kalman.F[2][2] = GetAlpha(deltaT);

	kalman.Q[2][2] = pow(sigmaA * pow(1 - GetAlpha(deltaT), 2), 2);
}

float PositionFilter::GetAlpha(float deltaT)
{
	return exp(-deltaT / decay);
}