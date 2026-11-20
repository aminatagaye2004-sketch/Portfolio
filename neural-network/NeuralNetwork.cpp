// NeuralNetwork.cpp
#include "NeuralNetwork.h"
#include <cassert>
#include <cmath>

// --- Activation functions ---
double relu(double x) { return x > 0 ? x : 0; } // rectified linear unit, it outputs the input directly if it is positive; otherwise, it will output zero
double drelu(double x) { return x > 0 ? 1 : 0; } // derivative of ReLU, it is 1 for positive input and 0 otherwise
double tanh_(double x) { return std::tanh(x); } // hyperbolic tangent function, it outputs values between -1 and 1
double dtanh(double x) {                        // derivative of tanh
    double t = tanh_(x);
    return 1.0 - t * t;
}

// --- Layer constructor ---
// Initializes weights randomly and biases to zero
NeuralNetwork::Layer::Layer(int input_size, int output_size, Activation a)
    : weights(input_size, output_size, true), bias(1, output_size, false), act(a) {}

// --- NeuralNetwork constructor ---
// Initializes learning rate
NeuralNetwork::NeuralNetwork(double lr) : learning_rate(lr) {}

// --- Add a new layer ---
void NeuralNetwork::addLayer(int input_size, int output_size, Activation act) {
    if (!layers.empty()) {
        int last_out = layers.back().weights.getCol();
        if (input_size != last_out) {

            // Auto-correct: use last output as input
            input_size = last_out;
        }
    }
    layers.emplace_back(input_size, output_size, act);
}

// --- Apply activation ---
// Applies the specified activation function element-wise 
Matrix NeuralNetwork::activate(const Matrix& z, Activation act) const {
    auto data = z.getData();
    int rows = z.getRows();
    int cols = z.getCol();
    std::vector<std::vector<double>> out(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double val = data[i][j];
            switch (act) {
                case Activation::SIGMOID: out[i][j] = Matrix::sigmoid(val); break;
                case Activation::RELU:    out[i][j] = relu(val); break;
                case Activation::TANH:    out[i][j] = tanh_(val); break;
            }
        }
    }
    return Matrix(out);
}

// --- Derivative ---
// Applies the derivative of the specified activation function element-wise
Matrix NeuralNetwork::dactivate(const Matrix& a, Activation act) const {
    auto data = a.getData();
    int rows = a.getRows();
    int cols = a.getCol();
    std::vector<std::vector<double>> out(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double val = data[i][j];
            switch (act) {
                case Activation::SIGMOID: out[i][j] = Matrix::dsigmoid(val); break;
                case Activation::RELU:    out[i][j] = drelu(val); break;
                case Activation::TANH:    out[i][j] = dtanh(val); break;
            }
        }
    }
    return Matrix(out);
}

// --- Prediction ---
// Feeds input through the network to get output
Matrix NeuralNetwork::predict(const Matrix& input) const {
    Matrix current = input;
    for (const auto& layer : layers) {
        Matrix z = current * layer.weights + layer.bias;
        current = activate(z, layer.act);
    }
    return current;
}

// --- Loss ---
// Mean Squared Error between output and target
double NeuralNetwork::calculateLoss(const Matrix& output, const Matrix& target) const {
    auto o = output.getData();
    auto t = target.getData();
    double loss = 0.0;
    int n = o[0].size();
    for (int i = 0; i < n; ++i) {
        double diff = o[0][i] - t[0][i];
        loss += diff * diff;
    }
    return loss / n;
}

// --- Training ---
double NeuralNetwork::train(const std::vector<Matrix>& inputs,
                            const std::vector<Matrix>& targets,
                            int epochs) {
    double final_loss = 0.0;
    for (int e = 0; e < epochs; ++e) {
        final_loss = 0.0;
        for (size_t i = 0; i < inputs.size(); ++i) {
            // Forward pass
            std::vector<Matrix> activations;
            activations.push_back(inputs[i]);
            Matrix current = inputs[i];
            for (const auto& layer : layers) {
                Matrix z = current * layer.weights + layer.bias;
                current = activate(z, layer.act);
                activations.push_back(current);
            }

            // Loss
            final_loss += calculateLoss(current, targets[i]);

            // Backprop
            Matrix error = current - targets[i];
            for (int l = static_cast<int>(layers.size()) - 1; l >= 0; --l) {
                Matrix deriv = dactivate(activations[l + 1], layers[l].act);
                Matrix gradient = error.elementWiseMultiply(deriv) * learning_rate;

                // Update bias
                layers[l].bias = layers[l].bias - gradient;

                // Delta weights
                Matrix inputT = activations[l].transpose();
                Matrix deltaW = inputT * gradient;
                layers[l].weights = layers[l].weights - deltaW;

                // Propagate error
                if (l > 0) {
                    Matrix weightsT = layers[l].weights.transpose();
                    error = gradient * weightsT;
                }
            }
        }
        final_loss /= inputs.size();
    }
    return final_loss;
}