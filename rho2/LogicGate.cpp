#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>

class LogicGateSimulator {
private:
    std::vector<std::string> inputNames;
    std::vector<std::vector<int>> andTerms; // -1 = don't care, 0 = negated, 1 = asserted

    // Helper: parse config line like "1 0 -"
    std::vector<int> parseTerm(const std::string& line) {
        std::vector<int> term;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            if (token == "-" || token == "X" || token == "x") {
                term.push_back(-1);
            } else if (token == "0") {
                term.push_back(0);
            } else if (token == "1") {
                term.push_back(1);
            } else {
                term.push_back(-1); // treat unknown as don't care
            }
        }
        return term;
    }

    bool evaluateAndGate(const std::vector<bool>& inputs, const std::vector<int>& term) {
        for (size_t i = 0; i < term.size(); ++i) {
            if (term[i] == -1) continue; // don't care
            bool expected = (term[i] == 1);
            if (inputs[i] != expected) {
                return false;
            }
        }
        return true;
    }

    bool evaluateOutput(const std::vector<bool>& inputs) {
        for (const auto& term : andTerms) {
            if (evaluateAndGate(inputs, term)) {
                return true; // OR of all ANDs
            }
        }
        return false;
    }

public:
    bool loadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open config file '" << filename << "'\n";
            return false;
        }

        std::string line;
        // First line: INPUTS: A B C
        if (std::getline(file, line)) {
            if (line.substr(0, 7) == "INPUTS:") {
                std::istringstream iss(line.substr(7));
                std::string name;
                while (iss >> name) {
                    inputNames.push_back(name);
                }
            }
        }

        // Read AND terms
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == '/') continue;
            auto term = parseTerm(line);
            if (!term.empty() && term.size() == inputNames.size()) {
                andTerms.push_back(term);
            }
        }

        std::cout << "Loaded " << andTerms.size() << " AND term(s) for inputs: ";
        for (const auto& name : inputNames) std::cout << name << " ";
        std::cout << "\n";
        return true;
    }

    void runInteractive() {
        if (inputNames.empty()) {
            std::cout << "No inputs loaded. Run loadConfig() first.\n";
            return;
        }

        char repeat = 'y';
        while (repeat == 'y' || repeat == 'Y') {
            std::vector<bool> inputValues(inputNames.size());
            std::cout << "\nEnter input values (0 or 1):\n";
            for (size_t i = 0; i < inputNames.size(); ++i) {
                int val;
                std::cout << inputNames[i] << " = ";
                std::cin >> val;
                inputValues[i] = (val != 0);
            }

            bool output = evaluateOutput(inputValues);
            std::cout << "Output x = " << (output ? "1" : "0") << "\n";

            std::cout << "Test another input? (y/n): ";
            std::cin >> repeat;
        }
    }
};



// ===================================================================================
// MAIN – ONLY LOGIC MODULE
// ===================================================================================
int main() {
    LogicGateSimulator sim;

    // You can change the filename here
    if (!sim.loadConfig("logic_config.txt")) {
        std::cout << "Creating default config file 'logic_config.txt'...\n";
        std::ofstream def("logic_config.txt");
        def << "INPUTS: A B C\n";
        def << "1 1 0\n";  // ABC'
        def << "1 1 -\n";  // AB
        def << "- 0 1\n";  // B'C
        def.close();
        std::cout << "Default config created. Please re-run the program.\n";
        return 1;
    }

    std::cout << "\n=== Logic Gate Array Simulator (SOP) ===\n";
    std::cout << "Expression: x = ABC' + AB + B'C\n";
    sim.runInteractive();

    return 0;
}