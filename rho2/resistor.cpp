// resistor_network.cpp
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

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

    // Gaussian elimination for A*x = b
    std::vector<double> solveLinearSystem(std::vector<std::vector<double>>& A, std::vector<double>& b) {
        unsigned int n = A.size();
        std::vector<double> x(n, 0.0);

        // Forward elimination
        for (unsigned int i = 0; i < n - 1; ++i) {
            for (unsigned int j = i + 1; j < n; ++j) {
                double factor = A[j][i] / A[i][i];
                for (unsigned int k = i; k < n; ++k) {
                    A[j][k] -= factor * A[i][k];
                }
                b[j] -= factor * b[i];
            }
        }

        // Back substitution
        for (int i = n - 1; i >= 0; --i) {
            double sum = 0.0;
            for (unsigned int j = i + 1; j < n; ++j) {
                sum += A[i][j] * x[j];
            }
            x[i] = (b[i] - sum) / A[i][i];
        }

        return x;
    }

public:
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "❌ Error: Could not open file '" << filename << "'\n";
            return false;
        }

        components.clear();
        nR = nV = nI = nN = 0;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string name;
            unsigned int n1, n2;
            double value;

            if (!(iss >> name >> n1 >> n2 >> value)) {
                continue; // skip malformed lines
            }

            CompType type;
            if (name[0] == 'R') { type = CompType::resistor; nR++; }
            else if (name[0] == 'V') { type = CompType::voltage; nV++; }
            else if (name[0] == 'I') { type = CompType::current; nI++; }
            else {
                continue;
            }

            components.push_back({type, name, n1, n2, value});
            nN = std::max({nN, n1, n2});
        }
        nN++; // Node 0 is ground → total nodes = max_index + 1
        file.close();
        return true;
    }

    void analyse() {
        if (components.empty()) {
            std::cout << "⚠ No components to analyse.\n";
            return;
        }

        unsigned int totalSize = nN + nV;
        std::vector<std::vector<double>> A(totalSize, std::vector<double>(totalSize, 0.0));
        std::vector<double> Z(totalSize, 0.0);

        unsigned int voltageIndex = 0;

        // Build MNA matrix and RHS vector
        for (const auto& comp : components) {
            unsigned int n1 = comp.n1;
            unsigned int n2 = comp.n2;
            double val = comp.value;

            switch (comp.type) {
                case CompType::resistor: {
                    double g = 1.0 / val;
                    A[n1][n1] += g;
                    A[n2][n2] += g;
                    A[n1][n2] -= g;
                    A[n2][n1] -= g;
                    break;
                }
                case CompType::voltage: {
                    A[n1][nN + voltageIndex] = 1.0;
                    A[n2][nN + voltageIndex] = -1.0;
                    A[nN + voltageIndex][n1] = 1.0;
                    A[nN + voltageIndex][n2] = -1.0;
                    Z[nN + voltageIndex] = -val;
                    voltageIndex++;
                    break;
                }
                case CompType::current: {
                    Z[n1] -= val;
                    Z[n2] += val;
                    break;
                }
            }
        }

        // Ground node 0
        A[0][0] = 1.0;
        Z[0] = 0.0;
        for (unsigned int i = 1; i < totalSize; ++i) {
            A[0][i] = A[i][0] = 0.0;
        }

        // Solve
        std::vector<double> X = solveLinearSystem(A, Z);

        // Output results
        std::cout << "\n" << std::string(30, '=') << "\n";
        std::cout << "   CIRCUIT ANALYSIS RESULTS\n";
        std::cout << std::string(30, '=') << "\n";
        std::cout << " Voltage sources: " << nV << "\n";
        std::cout << " Current sources: " << nI << "\n";
        std::cout << "       Resistors: " << nR << "\n";
        std::cout << "           Nodes: " << nN << "\n";
        std::cout << std::string(30, '-') << "\n";

        for (unsigned int i = 0; i < nN; ++i) {
            std::cout << " Node " << std::setw(3) << i
                      << " = " << std::fixed << std::setprecision(6)
                      << X[i] << " V\n";
        }

        if (nV > 0) {
            std::cout << std::string(30, '-') << "\n";
            unsigned int vIdx = 0;
            for (const auto& comp : components) {
                if (comp.type == CompType::voltage) {
                    std::cout << " I(" << comp.name << ")   = "
                              << std::setw(10) << std::setprecision(6)
                              << X[nN + vIdx++] << " A\n";
                }
            }
        }
        std::cout << std::string(30, '=') << "\n\n";
    }

    // Public interface for menu integration
    void run() {
        std::string filename;
        std::cout << "\n=== Resistor Network Analyser ===\n";
        std::cout << "Enter netlist filename (e.g., example2.cir): ";
        std::cin >> filename;

        if (loadFromFile(filename)) {
            analyse();
        } else {
            std::cout << "Failed to load circuit.\n";
        }
    }
};

// Example usage (for standalone testing)

int main() {
    ResistorNetworkAnalyser sim;
    sim.run(); // Prompts for filename
    return 0;
}
