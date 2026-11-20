/*
@Filename: Matrix.h
@Author: Ivan Ling (ivan.ling@soton.ac.uk)
@Version: 1.0

@Date: 27/10/2025
@copyright Copyright (c) 2025
*/

#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

class Matrix
{
private: 
	std::vector<std::vector<double>> data;
	int row;
	int col;

public:	
	//Constructor
	Matrix(int r, int c, bool isRandom = false);
	Matrix(const std::vector<std::vector<double>>& d);
	

	// Getters
    int getRows() const;
    int getCol() const;
    std::vector<std::vector<double>> getData() const;

	
	//Utility
	void print() const;
	Matrix transpose() const;

	//Operators
	Matrix operator+(const Matrix& other) const;
	Matrix operator-(const Matrix& other) const;
	Matrix operator*(const Matrix& other) const;   //matrix multiplication
	Matrix operator*(double scalar) const;
	Matrix elementWiseMultiply(const Matrix& other) const;

	//Static Helpers
	static double sigmoid(double x);
	static double dsigmoid(double x);

};

#endif // MATRIX_H