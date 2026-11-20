#ifndef RESISTOR_NETWORK_ANALYSER_H
#define RESISTOR_NETWORK_ANALYSER_H

#include <vector>
#include <string>

class ResistorNetworkAnalyser {
private:
    enum class CompType { resistor, voltage, current };

    struct Component {
        CompType type;
        std::string name;
        unsigned int n1, n2;
        double value;
    };

    std::vector<Component> components;
    unsigned int nR = 0, nV = 0, nI = 0, nN = 0;

    // Private helper: solve linear system using Gaussian elimination
    std::vector<double> solveLinearSystem(
        std::vector<std::vector<double>>& A,
        std::vector<double>& b
    );

public:
    // Load circuit from netlist file (e.g., "example1.cir")
    bool loadFromFile(const std::string& filename);

    // Perform Modified Nodal Analysis and print results
    void analyse();

    // Optional: run interactively (for menu integration)
    void run();
};

#endif // RESISTOR_NETWORK_ANALYSER_H