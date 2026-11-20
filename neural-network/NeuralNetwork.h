// NeuralNetwork.h
#ifndef NEURALNETWORK_H
#define NEURALNETWORK_H

#include "Matrix.h"
#include <vector>
#include <functional>

// Supported activation functions
enum class Activation {
    SIGMOID,
    RELU,
    TANH
};

class NeuralNetwork {
public:
    struct Layer {
        Matrix weights;
        Matrix bias;
        Activation act;
        Layer(int input_size, int output_size, Activation a = Activation::SIGMOID);
    };

private:
    std::vector<Layer> layers;
    double learning_rate;

    // Activation and derivative wrappers
    Matrix activate(const Matrix& z, Activation act) const;
    Matrix dactivate(const Matrix& a, Activation act) const;

public:
    NeuralNetwork(double lr = 0.1);
    void addLayer(int input_size, int output_size, Activation act = Activation::SIGMOID);
    Matrix predict(const Matrix& input) const;
    double train(const std::vector<Matrix>& inputs, const std::vector<Matrix>& targets, int epochs);
    double calculateLoss(const Matrix& output, const Matrix& target) const;
};

// Helper activation functions
double relu(double x);
double drelu(double x);
double tanh_(double x);
double dtanh(double x);

#endif