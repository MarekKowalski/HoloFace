#include "tree.h"
#include "hog.h"
#include <sstream>


Mat<float> Forest::predict(Mat<float> &X, int maxDepth)
{
	Mat<float> sumOfPredictions;

	for (int i = 0; i < nTrees; i++)
	{
		int leafNode = getLeafNode(X, i, maxDepth);
		Mat<float> prediction = values[i].subMat(leafNode, leafNode + 1, 0, values[i].width);
		if (i == 0)
			sumOfPredictions = prediction;
		else
			sumOfPredictions += prediction;
	}

	return sumOfPredictions / (float)nTrees;
}

int Forest::getLeafNode(Mat<float> &X, int treeIdx, int maxDepth)
{
	int node = 0, feature;
	float threshold;
	//indices of the left and right child nodes
	int left = childrenLeft[treeIdx][node];
	int right = childrenRight[treeIdx][node];
	float val;

	int curDepth = 0;
	//iterates until a leaf node is reached
	while (left != -1 && right != -1)
	{
		feature = features[treeIdx][node];
		threshold = thresholds[treeIdx][node];

		//value at node		
		//val = (X * models[treeIdx][node])[0][0];
		val = X.BLASdot(models[treeIdx][node]);
		if (val <= threshold)
			node = left;
		else
			node = right;

		left = childrenLeft[treeIdx][node];
		right = childrenRight[treeIdx][node];

		curDepth++;
		if (maxDepth != 0 && curDepth >= maxDepth)
			break;
	}

	return node;
}

vector<Forest> loadForests(const char *filename, int &nOutputs)
{
	nOutputs = 0;
	vector<Forest> forests;

	FILE *handle = fopen(filename, "rb");
	int nForests;

	fread(&nForests, sizeof(int), 1, handle);

	for (int i = 0; i < nForests; i++)
	{
		Forest temp;
		int h, w, nValues, featureSize;

		fread(&h, sizeof(int), 1, handle);
		fread(&w, sizeof(int), 1, handle);
		fread(&nValues, sizeof(int), 1, handle);
		fread(&featureSize, sizeof(int), 1, handle);

		temp.nTrees = h;

		temp.childrenLeft = Mat<int>(h, w);
		temp.childrenRight = Mat<int>(h, w);
		temp.features = Mat<int>(h, w);
		temp.thresholds = Mat<float>(h, w);
		temp.leafIndexMaps = Mat<int>(h, w);
		temp.values = vector<Mat<float>>(temp.nTrees);

		fread(temp.childrenLeft.data, sizeof(int), w * h, handle);
		fread(temp.childrenRight.data, sizeof(int), w * h, handle);
		fread(temp.features.data, sizeof(int), w * h, handle);
		fread(temp.thresholds.data, sizeof(int), w * h, handle);
		fread(temp.leafIndexMaps.data, sizeof(int), w * h, handle);

		for (int j = 0; j < temp.nTrees; j++)
		{
			temp.values[j] = Mat<float>(w, nValues);
			fread(temp.values[j].data, sizeof(float), w * nValues, handle);
		}

		temp.models.resize(h);
		for (int j = 0; j < h; j++)
		{
			temp.models[j].resize(w);
			for (int k = 0; k < w; k++)
			{
				Mat<float> tempModel(featureSize, 1);
				fread(tempModel.data, sizeof(float), featureSize, handle);
				temp.models[j][k] = tempModel;
			}
		}

		temp.sumLeafNodes = Mat<int>(h + 1, 1);
		int sum = 0;

		temp.sumLeafNodes[0][0] = 0;
		for (int j = 0; j < h; j++)
		{
			for (int k = 0; k < w; k++)
			{
				if (temp.leafIndexMaps[j][k] > 0)
					sum++;
			}

			sum += 1;
			temp.sumLeafNodes[j + 1][0] = sum;
		}

		forests.push_back(temp);

		nOutputs += h;
	}

	return forests;
}



TreeFeature::TreeFeature()
{
}

TreeFeature::TreeFeature(string inDir, int blockSize, int cellSize, int nIters)
{
	this->blockSize = blockSize;
	this->cellSize = cellSize;

	for (int i = 0; i < nIters; i++)
	{
		nOutputs.push_back(0);

		string filename = inDir + "forests" + std::to_string((long long)i) + ".bin";
		forests.push_back(loadForests(filename.c_str(), nOutputs[nOutputs.size() - 1]));
		cout << "Iteration loaded" << endl;
	}
}

void TreeFeature::getBinaryFeature(Mat<unsigned char> &img, Mat<float> &S, Mat<int> &res, Mat<float> &baseFeature, int iter)
{
	//HOG descriptor extraction
	baseFeature = hog::HOG(img, S, blockSize, cellSize);

	vector<Mat<float>> featureVect(baseFeature.height, Mat<float>(1, baseFeature.width));

	for (int i = 0; i < baseFeature.height; i++)
	{
		memcpy(featureVect[i].data, baseFeature.data + i * baseFeature.width, baseFeature.width * sizeof(float));
	}

	vector<int> sums(S.height, 0);
	vector<vector<int>> ns(S.height);
	for (int i = 0; i < S.height; i++)
	{
		if (i > 0)
			sums[i] = sums[i - 1] + forests[iter][i - 1].sumLeafNodes[forests[iter][i].nTrees][0];
		ns[i] = vector<int>(forests[iter][i].nTrees, 0);
		for (int j = 0; j < forests[iter][i].nTrees; j++)
		{
			if (i == 0 && j == 0)
				continue;
			else if (j == 0)
				ns[i][j] = ns[i - 1][ns[i - 1].size() - 1] + 1;
			else
				ns[i][j] = ns[i][j - 1] + 1;
		}
	}

#pragma omp parallel for
	for (int i = 0; i < S.height; i++)
	{
		for (int j = 0; j < forests[iter][i].nTrees; j++)
		{
			int node = 0, feature;
			float threshold;
			//indices of the left and right child nodes
			int left = forests[iter][i].childrenLeft[j][node];
			int right = forests[iter][i].childrenRight[j][node];
			float val;

			//iterates until a leaf node is reached
			while (left != -1 && right != -1)
			{
				feature = forests[iter][i].features[j][node];
				threshold = forests[iter][i].thresholds[j][node];

				//value at node				
				val = featureVect[feature].BLASdot(forests[iter][i].models[j][node]);
				if (val <= threshold)
					node = left;
				else
					node = right;

				left = forests[iter][i].childrenLeft[j][node];
				right = forests[iter][i].childrenRight[j][node];
			}

			//the leaf number corresponding to the final node is found
			node = forests[iter][i].leafIndexMaps[j][node];
			//the number of leaf nodes in the previous trees of current forest is added
			node += forests[iter][i].sumLeafNodes[j][0];
			//the number of leaf nodes in all previous forests is added
			node += sums[i];
			//node now contains the leaf number that is unique for all trees and for all points

			//the output feature consists of unique indices of leaf nodes reached by each tree
			res[ns[i][j]][0] = node;
		}
	}
}
