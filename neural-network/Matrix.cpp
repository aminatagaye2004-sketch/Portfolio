/*
@Filename: Matrix.cpp
@Author: Ivan Ling (ivan.ling@soton.ac.uk)
@Version: 1.0

@Date: 27/10/2025
@copyright Copyright (c) 2025
*/

#include "Matrix.h"
#include <random>   // For rand() and RAND_MAX
#include <cassert>
#include <cmath>


//Constructor
Matrix::Matrix(int r, int c, bool isRandom): row(r), col(c){
	data.resize(r, std::vector<double>(c, 0.0)); // Initialize to zeros
	 if (isRandom) {
		 // 'static' means these are created only once and reused for efficiency
		static std::random_device rd; // Create a random device to seed the random number generator
		static std::mt19937 gen(rd());  // Create a Mersenne Twister random number generator, seeded with the random device
		static std::uniform_real_distribution<>dis(0.0, 1.0);  // Create a uniform distribution that generates numbers between 0.0 and 1.0
		for(int i=0; i<r; ++i){
			for(int j=0; j< c; ++j){
				data[i][j]=dis(gen); // Assign a random number to the current matrix element
			}	
		}
	}
}

Matrix::Matrix(const std::vector<std::vector<double>>& d)
    : data(d), row(static_cast<int>(d.size())), col(d.empty() ? 0 : static_cast<int>(d[0].size())) {
    // Optional: validate that all rows have the same length
    for (const auto& r : d) {
        if (static_cast<int>(r.size()) != col) {
			throw std::invalid_argument("All rows must have the same number of columns.");	
        }
    }  
}


//getters
int Matrix::getRows() const { return row; }
int Matrix::getCol() const { return col; }
vector<vector<double>> Matrix::getData() const { return data; }



//Print the Matrix
void Matrix::print() const{
	for(const auto& row:data){
		for(double val: row){
			std::cout<<val<<"";
		}
		std::cout<<"\n";
	}

}


//Transpose
Matrix Matrix::transpose() const
{
	Matrix result(col, row);
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            result.data[j][i] = data[i][j];
        }
    }
    return result;
}


//Matrix Addition
Matrix Matrix::operator+(const Matrix& other) const{
	assert (row==other.row && col==other.col);
	Matrix result(row,col);
	for(int i=0; i<row; ++i){
		for(int j=0; j<col; ++j){
			result.data[i][j]= data[i][j] + other.data[i][j];
		}
	}

	return result;
}


//Matrix Susbtraction
Matrix Matrix::operator-(const Matrix& other) const
{
	assert (row==other.row && col==other.col);
	Matrix result(row,col);
	for(int i=0; i<row; ++i){
		for(int j=0; j<col; ++j){
			result.data[i][j]= data[i][j] - other.data[i][j];
		}
	}

	return result;
}

//Matrix multiplication (A * B)
Matrix Matrix::operator*(const Matrix& other) const
{
	assert(col==other.row); // A.cols == B.rows
	Matrix result(row, other.col);
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < other.col; ++j) {
            for (int k = 0; k < col; ++k) {
                result.data[i][j] += data[i][k] * other.data[k][j];
            }
        }
    }
    return result;

}

//Scalar multiplication
Matrix Matrix::operator*(double scalar) const
{
	Matrix result(row, col);
	for(int i=0; i<row; ++i){
		for(int j=0; j<col; ++j){
			result.data[i][j]= data[i][j]*scalar;

		}
	}
	return result;
}

// Element-wise multiplication
Matrix Matrix::elementWiseMultiply(const Matrix& other) const
{
	assert(row==other.row && col==other.col);
	Matrix result(row,col);
	for (int i=0; i<row; ++i){
        for (int j = 0; j < col; ++j) {
            result.data[i][j] = data[i][j] * other.data[i][j];
        }
    }
    return result;
	
}

// Static activation functions
double Matrix::sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

double Matrix::dsigmoid(double x) {
    double s = sigmoid(x);
    return s * (1.0 - s);
}
