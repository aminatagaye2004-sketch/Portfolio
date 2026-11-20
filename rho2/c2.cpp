#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

// ========================
// FourierSimulator Class
// ========================
class FourierSimulator { 
private:
    static constexpr double PI = 3.14159265358979323846;
    static constexpr int PLOT_WIDTH = 180;
    static constexpr double TIME_SCALE = 75.0; // matches original x*(PI/75)

    double a0 = 0.0;          // DC offset
    double a[6] = {0};        // cosine coefficients a1 to a6
    double b[6] = {0};        // sine coefficients b1 to b6


    void plotValue(float value) const {
        int spaces = static_cast<int>(value * PLOT_WIDTH);
        if (spaces < 0) spaces = 0;
        if (spaces > PLOT_WIDTH) spaces = PLOT_WIDTH;

        for (int i = 0; i < spaces; ++i) {
            std::cout << ' ';
        }
        std::cout << "*\n";
    }

    float computeWave(unsigned long t) const {
        double omega_t = (PI / TIME_SCALE) * static_cast<double>(t);
        double sum = a0;

        for (int n = 1; n <= 6; ++n) {
            sum += a[n-1] * std::cos(n * omega_t) + b[n-1] * std::sin(n * omega_t);
        }

        // Compute a safe normalization factor from coefficients (max possible abs sum)
        double maxAbs = std::abs(a0);
        for (int i = 0; i < 6; ++i) {
            maxAbs += std::abs(a[i]) + std::abs(b[i]);
        }

        // If coefficients are all zero, avoid division by zero
        if (maxAbs <= 0.0) {
            return 0.5f;
        }

        // Map sum from [-maxAbs, maxAbs] -> [0,1]
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

   // ...existing code...
public:
    void runSimulation() {
        int numSamples;
        std::cout << "Enter number of samples to plot (e.g., 250): ";
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

            if (x % 10 == 0) {
                std::cout << "---";
            } else {
                std::cout << "  ";
            }

            plotValue(y);
        }
    }
};



// ========================
// Main Function
// ========================
int main() {
    FourierSimulator sim;

    std::cout << "=== Fourier Wave Simulator (ρ² Lab) ===\n";
    std::cout << "Choose a test waveform:\n";
    std::cout << "1. Pure Sine Wave (b1 = 1)\n";
    std::cout << "2. Square Wave Approximation (3 odd harmonics)\n";
    std::cout << "3. Cosine Wave (a1 = 1)\n";
    std::cout << "4. Custom (hardcoded in code)\n";
    std::cout << "Enter choice (1-4): ";

    int choice;
    std::cin >> choice;

    double a[6] = {0};
    double b[6] = {0};
    double a0 = 0.0;

    switch (choice) {
        case 1:
            b[0] = 1.0; // sin(ωt)
            break;
        case 2:
            b[0] = 1.0;         // sin(ωt)
            b[2] = 1.0 / 3.0;   // sin(3ωt)
            b[4] = 1.0 / 5.0;   // sin(5ωt)
            break;
        case 3:
            a[0] = 1.0; // cos(ωt)
            break;
        case 4:
            // Example: sawtooth-like (you can change these!)
            b[0] = 1.0;
            b[1] = 0.5;
            b[2] = 1.0/3.0;
            b[3] = 0.25;
            b[4] = 0.2;
            b[5] = 1.0/6.0;
            break;
        default:
            std::cout << "Invalid choice. Using sine wave.\n";
            b[0] = 1.0;
    }

    sim.setCoefficients(a0, a, b);
    sim.runSimulation();

    return 0;
}