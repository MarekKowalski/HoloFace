/** \file tree.h
This is a header file for the Forest container struct and the TreeFeature class, which contains the declations of all its members and methods.
*/
#pragma once

#include "utils.h"


///Defines a forest of regression trees.
struct Forest
{
	int nTrees; ///< Number of trees in the forest.
	Mat<int> childrenLeft; ///< For each node contains an index of the node to it's left, negative if input node is a leaf.
	Mat<int> childrenRight; ///< For each node contains an index of the node to it's right, negative if input node is a leaf.
	Mat<int> features; ///< For each node contains the index of the feature that the node will use, in current version all 0.
	Mat<float> thresholds; ///< A decision threshold for each node, if the value at node is larger, the right subtree will be chosen, left otherwise.
	Mat<int> leafIndexMaps; ///< Maps the node index to it's corresponding leaf index, negative if node is not an index.
	vector<Mat<float>> values;
	Mat<int> sumLeafNodes; ///< A cumulative sum of leaf nodes in each tree, the last element thus contains the sum of leafs in all trees.
	vector<vector<Mat<float>>> models; ///< For each node contains a vector (as Mat<float) representing a linear decision function for that node.

	Mat<float> predict(Mat<float> &X, int maxDepth=0);
	int getLeafNode(Mat<float> &X, int treeIdx, int maxDepth=0);
};

///Loads a forests of regression trees corresponding to one alignment iteration.
/**
@param filename name of the file containing the forest data
*/
vector<Forest> loadForests(const char *filename, int &nOutputs);

///Contains all the tree related model data and provides functionality for binary feature generation.
class TreeFeature
{
public:
	vector<int> nOutputs; ///< The total number of leafs in all trees for all points at a given iteration.

	TreeFeature(); ///< An empty default constructor.
	///Loads a model consisting of nIters and nPoints from a provided directory.
	/**
	@param inDir a path to a directory contatining the model files
	@param nIters number of iterations of the model to be loaded
	@param nPoints number of points used by the model to be loaded
	*/
	TreeFeature(string inDir, int blockSize, int cellSize, int nIters);

	///Extracts HOG descriptors, performs tree travel and forms the tree induced binary features.
	/**
	@param img input image
	@param S current estimate of face characteristic points' locations
	@param res container for the output feature
	@param iter number of current fitting iteration
	*/
	void getBinaryFeature(Mat<unsigned char> &img, Mat<float> &S, Mat<int> &res, Mat<float> &baseFeature, int iter);
private:
	int blockSize, cellSize;

	vector<vector<Forest>> forests; ///< A container for the forests in each iteration for each point.
};

