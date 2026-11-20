// rho2_simulator.cpp
// Full ρ² Circuit Simulator – Integrated CLI Menu
// Sections 2.4 + 3.2 + 3.3

#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

// ===================================================================================
// 1. FOURIER SIMULATOR (C2)
// ===================================================================================
class FourierSimulator {
private:
    static constexpr double PI = 3.14159265358979323846;
    static constexpr int PLOT_WIDTH = 180;
    static constexpr double TIME_SCALE = 75.0;

    double a0 = 0.0;
    double a[6] = {0};
    double b[6] = {0};

    void plotValue(float value) const {
        int spaces = static_cast<int>(value * PLOT_WIDTH);
        if (spaces < 0) spaces = 0;
        if (spaces > PLOT_WIDTH) spaces = PLOT_WIDTH;
        for (int i = 0; i < spaces; ++i) std::cout << ' ';
        std::cout << "*\n";
    }

    float computeWave(unsigned long t) const {
        double omega_t = (PI / TIME_SCALE) * static_cast<double>(t);
        double sum = a0;
        for (int n = 1; n <= 6; ++n) {
            sum += a[n-1] * std::cos(n * omega_t) + b[n-1] * std::sin(n * omega_t);
        }

        double maxAbs = std::abs(a0);
        for (int i = 0; i < 6; ++i) {
            maxAbs += std::abs(a[i]) + std::abs(b[i]);
        }
        if (maxAbs <= 0.0) return 0.5f;
        float y = static_cast<float>((sum + maxAbs) / (2.0 * maxAbs));
        if (y < 0.0f) y = 0.0f;
        if (y > 1.0f) y = 1.0f;
        return y;
    }

public:
    void setCoefficients(double _a0, const double _a[6], const double _b[6]) {
        a0 = _a0;
        for (int i = 0; i < 6; ++i) {
            a[i] = _a[i];
            b[i] = _b[i];
        }
    }

    void runSimulation() {
        int numSamples;
        std::cout << "\nEnter number of samples to plot (e.g., 250): ";
        if (!(std::cin >> numSamples) || numSamples <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Using default 250 samples.\n";
            numSamples = 250;
        }

        for (unsigned long x = 1; x <= static_cast<unsigned long>(numSamples); ++x) {
            float y = computeWave(x);
            std::cout << std::setw(8) << x;
            std::cout << std::fixed << std::setprecision(3);
            std::cout << std::setw(8) << y << " |";
            std::cout << (x % 10 == 0 ? "---" : "  ");
            plotValue(y);
        }
    }

    void run() {
        std::cout << "\n=== Fourier Wave Simulator ===\n";
        std::cout << "1. Pure Sine Wave (b1 = 1)\n";
        std::cout << "2. Square Wave (3 odd harmonics)\n";
        std::cout << "3. Cosine Wave (a1 = 1)\n";
        std::cout << "4. Sawtooth Wave (alternating signs)\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;

        double a[6] = {0}, b[6] = {0}, a0 = 0.0;
        switch (choice) {
            case 1: b[0] = 1.0; break;
            case 2: b[0] = 1.0; b[2] = 1.0/3.0; b[4] = 1.0/5.0; break;
            case 3: a[0] = 1.0; break;
            case 4:
                b[0] =  1.0;
                b[1] = -0.5;
                b[2] =  1.0/3.0;
                b[3] = -0.25;
                b[4] =  0.2;
                b[5] = -1.0/6.0;
                break;
            default: b[0] = 1.0;
        }
        setCoefficients(a0, a, b);
        runSimulation();
    }
};

// ===================================================================================
// 2. LOGIC GATE SIMULATOR (SOP)
// ===================================================================================
class LogicGateSimulator {
private:
    std::vector<std::string> inputNames;
    std::vector<std::vector<int>> andTerms;

    std::vector<int> parseTerm(const std::string& line) {
        std::vector<int> term;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            if (token == "-" || token == "X" || token == "x") term.push_back(-1);
            else if (token == "0") term.push_back(0);
            else if (token == "1") term.push_back(1);
            else term.push_back(-1);
        }
        return term;
    }

    bool evaluateAndGate(const std::vector<bool>& inputs, const std::vector<int>& term) {
        for (size_t i = 0; i < term.size(); ++i) {
            if (term[i] == -1) continue;
            bool expected = (term[i] == 1);
            if (inputs[i] != expected) return false;
        }
        return true;
    }

    bool evaluateOutput(const std::vector<bool>& inputs) {
        for (const auto& term : andTerms) {
            if (evaluateAndGate(inputs, term)) return true;
        }
        return false;
    }

public:
    bool loadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        inputNames.clear();
        andTerms.clear();

        std::string line;
        if (std::getline(file, line) && line.substr(0, 7) == "INPUTS:") {
            std::istringstream iss(line.substr(7));
            std::string name;
            while (iss >> name) inputNames.push_back(name);
        }

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#' || line[0] == '/') continue;
            size_t pos = line.find('#');
            if (pos != std::string::npos) line = line.substr(0, pos);
            auto term = parseTerm(line);
            if (!term.empty() && term.size() == inputNames.size()) {
                andTerms.push_back(term);
            }
        }
        file.close();
        return true;
    }

    void run() {
        std::string filename = "logic_config.txt";
        if (!loadConfig(filename)) {
            std::cout << "❌ Config file 'logic_config.txt' not found.\n";
            return;
        }

        char repeat = 'y';
        while (repeat == 'y' || repeat == 'Y') {
            std::vector<bool> vals(inputNames.size());
            std::cout << "\nEnter inputs (0/1):\n";
            for (size_t i = 0; i < inputNames.size(); ++i) {
                int v; std::cout << inputNames[i] << " = "; std::cin >> v;
                vals[i] = (v != 0);
            }
            bool out = evaluateOutput(vals);
            std::cout << "Output x = " << (out ? "1" : "0") << "\n";
            std::cout << "Test again? (y/n): ";
            std::cin >> repeat;
        }
    }
};

// ===================================================================================
// 3. RESISTOR NETWORK ANALYSER (C5)
// ===================================================================================
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

    std::vector<double> solveLinearSystem(std::vector<std::vector<double>>& A, std::vector<double>& b) {
        unsigned int n = A.size();
        std::vector<double> x(n, 0.0);
        for (unsigned int i = 0; i < n - 1; ++i) {
            for (unsigned int j = i + 1; j < n; ++j) {
                double f = A[j][i] / A[i][i];
                for (unsigned int k = i; k < n; ++k) A[j][k] -= f * A[i][k];
                b[j] -= f * b[i];
            }
        }
        for (int i = n - 1; i >= 0; --i) {
            double sum = 0.0;
            for (unsigned int j = i + 1; j < n; ++j) sum += A[i][j] * x[j];
            x[i] = (b[i] - sum) / A[i][i];
        }
        return x;
    }

public:
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        components.clear();
        nR = nV = nI = nN = 0;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream iss(line);
            std::string name;
            unsigned int n1, n2;
            double value;
            if (!(iss >> name >> n1 >> n2 >> value)) continue;

            CompType type;
            if (name[0] == 'R') { type = CompType::resistor; nR++; }
            else if (name[0] == 'V') { type = CompType::voltage; nV++; }
            else if (name[0] == 'I') { type = CompType::current; nI++; }
            else continue;

            components.push_back({type, name, n1, n2, value});
            nN = std::max({nN, n1, n2});
        }
        nN++;
        file.close();
        return true;
    }

    void analyse() {
        if (components.empty()) return;

        unsigned int N = nN + nV;
        std::vector<std::vector<double>> A(N, std::vector<double>(N, 0.0));
        std::vector<double> Z(N, 0.0);

        unsigned int vIdx = 0;
        for (const auto& c : components) {
            if (c.type == CompType::resistor) {
                double g = 1.0 / c.value;
                A[c.n1][c.n1] += g; A[c.n2][c.n2] += g;
                A[c.n1][c.n2] -= g; A[c.n2][c.n1] -= g;
            } else if (c.type == CompType::voltage) {
                A[c.n1][nN + vIdx] = 1.0; A[c.n2][nN + vIdx] = -1.0;
                A[nN + vIdx][c.n1] = 1.0; A[nN + vIdx][c.n2] = -1.0;
                Z[nN + vIdx] = -c.value;
                vIdx++;
            } else if (c.type == CompType::current) {
                Z[c.n1] -= c.value; Z[c.n2] += c.value;
            }
        }

        A[0][0] = 1.0; Z[0] = 0.0;
        for (unsigned int i = 1; i < N; ++i) A[0][i] = A[i][0] = 0.0;

        std::vector<double> X = solveLinearSystem(A, Z);

        std::cout << "\n" << std::string(30, '=') << "\n";
        std::cout << "   CIRCUIT ANALYSIS RESULTS\n";
        std::cout << std::string(30, '=') << "\n";
        std::cout << " Voltage sources: " << nV << "\n";
        std::cout << " Current sources: " << nI << "\n";
        std::cout << "       Resistors: " << nR << "\n";
        std::cout << "           Nodes: " << nN << "\n";
        std::cout << std::string(30, '-') << "\n";
        for (unsigned int i = 0; i < nN; ++i)
            std::cout << " Node " << std::setw(3) << i << " = " << std::fixed << std::setprecision(6) << X[i] << " V\n";
        if (nV > 0) {
            std::cout << std::string(30, '-') << "\n";
            vIdx = 0;
            for (const auto& c : components)
                if (c.type == CompType::voltage)
                    std::cout << " I(" << c.name << ")   = " << std::setw(10) << std::setprecision(6) << X[nN + vIdx++] << " A\n";
        }
        std::cout << std::string(30, '=') << "\n";
    }

    void run() {
        std::string file;
        std::cout << "\nEnter netlist filename (e.g., example2.cir): ";
        std::cin >> file;
        if (loadFromFile(file)) analyse();
        else std::cout << " Failed to load circuit.\n";
    }
};

// ===================================================================================
// MAIN CLI MENU – SECTION 2.4 + 3.2
// ===================================================================================
int main() {
    int choice;
    while (true) {
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "             CIRCUIT SIMULATOR \n";
        std::cout << std::string(50, '=') << "\n";
        std::cout << "1. Resistor Network Analyser (C5)\n";
        std::cout << "2. Fourier Wave Simulator (C2)\n";
        std::cout << "3. Logic Gate Array Simulator (SOP)\n";
        std::cout << "4. Exit\n";
        std::cout << "Choose an option (1-4): ";
        
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1;
        }

        switch (choice) {
            case 1: {
                ResistorNetworkAnalyser r;
                r.run();
                break;
            }
            case 2: {
                FourierSimulator f;
                f.run();
                break;
            }
            case 3: {
                LogicGateSimulator l;
                l.run();
                break;
            }
            case 4:
                std::cout << "Goodbye!\n";
                return 0;
            default:
                std::cout << " Invalid option. Please enter 1-4.\n";
        }
    }
}

