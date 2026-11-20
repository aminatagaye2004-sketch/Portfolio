#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int board[9][9] = {0};

void LoadBoard(string filename, int board[9][9]) {
    ifstream File(filename);
    string line;
    for (int i = 0; i < 9; i++) {
        getline(File, line);
        for (int j = 0; j < 9; j++) {
            if (j < line.length()) {
                char c = line[j];
                if (c == ',' || c == '0' || c == '.') {
                    board[i][j] = 0;
                } else if (c >= '1' && c <= '9') {
                    board[i][j] = c - '0';
                } else {
                    board[i][j] = 0;
                }
            } else {
                board[i][j] = 0;
            }
        }
    }
    File.close();
}

void DisplayBoard(int board[9][9]) {
    cout << "\n    0 1 2    3 4 5    6 7 8\n";
    cout << "  +-------+-------+-------+\n";
    for (int i = 0; i < 9; i++) {
        cout << i << " | ";
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == 0)
                cout << ". ";
            else
                cout << board[i][j] << " ";
            
            if ((j + 1) % 3 == 0 && j != 8)
                cout << "| ";
        }
        cout << "|\n";
        if ((i + 1) % 3 == 0 && i != 8)
            cout << "  +-------+-------+-------+\n";
    }
    cout << "  +-------+-------+-------+\n";
}

int main() {
    string filename;
    cout << "Enter Sudoku file name: ";
    cin >> filename;
    
    LoadBoard(filename, board);
    DisplayBoard(board);
    
    return 0;
}