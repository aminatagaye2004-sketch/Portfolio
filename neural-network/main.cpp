// showcase.cpp
#include <iostream>
#include <vector>
#include "NeuralNetwork.h"

// Binary input representation for numbers 0-15
Matrix inputFromInt(int n) {
    std::vector<std::vector<double>> d(1, std::vector<double>(4));
    d[0][0] = (n >> 3) & 1;
    d[0][1] = (n >> 2) & 1;
    d[0][2] = (n >> 1) & 1;
    d[0][3] = (n >> 0) & 1;
    return Matrix(d);
}

// One-hot encoding target
Matrix targetFromInt(int n) {
    std::vector<std::vector<double>> d(1, std::vector<double>(16, 0.0));
    d[0][n] = 1.0;
    return Matrix(d);
}

int main() {
    // Build dataset
    std::vector<Matrix> inputs, targets;
    for (int i = 0; i < 16; ++i) {
        inputs.push_back(inputFromInt(i));
        targets.push_back(targetFromInt(i));
    }

    // Create user-friendly network
    NeuralNetwork nn(0.05);
    nn.addLayer(4, 16, Activation::RELU);      // hidden
    nn.addLayer(16, 16, Activation::SIGMOID);  // output

    // Train
    double loss = nn.train(inputs, targets, 3000);
    std::cout << "Final loss after training: " << loss << "\n\n";

    // Test in lab screenshot format (Section 3.4 style)
    for (int i = 0; i < 16; ++i) {
        Matrix out = nn.predict(inputFromInt(i));
        auto data = out.getData()[0];
        int pred = 0;
        for (int j = 1; j < 16; ++j)
            if (data[j] > data[pred]) pred = j;

        // Lab-style output (from p.7)
        std::cout << "**********************************************\n";
        std::cout << "Let's test it out!\n";
        std::cout << i << "\n";
        std::cout << ((i >> 3) & 1) << ((i >> 2) & 1)
                  << ((i >> 1) & 1) << ((i >> 0) & 1) << "\n";
        std::cout << "******************** OUTPUT *******************\n";
        for (int j = 0; j < 16; ++j) {
            std::cout << (j + 1) << "---" << data[j] << "\n";
        }
        std::cout << "**********************************************\n";
        std::cout << "Input " << ((i >> 3) & 1) << ((i >> 2) & 1)
                  << ((i >> 1) & 1) << ((i >> 0) & 1)
                  << " -> Prediction: " << pred << "\n\n";
    }

    return 0;
}