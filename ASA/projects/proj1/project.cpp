#include <iostream>
#include <vector>
#include <list>
using namespace std;

size_t _N, _M;
int target;
vector<int> sequence;
vector<vector<int>> matrix;
vector<vector<list<vector<int>>>> resultMatrix;

void readInput() {
    cin >> _N >> _M;

    matrix = vector<vector<int>>(_N, vector<int>(_N, 0));
    
    for (size_t i = 0; i < _N; i++) {
        for (size_t j = 0; j < _N; j++) {
            cin >> matrix[i][j];
        }
    }

    sequence.resize(_M);
    for (size_t i = 0; i < _M; i++) {
        cin >> sequence[i];
    }

    cin >> target;
}

void calculateResulteMatriz() {
    for (size_t i = 0; i < _M; i++) {
        vector<int> vec = {sequence[i]-1};
        resultMatrix[i][i].push_back(vec);
    }
    for (size_t d = 1; d < _M; d++) {
        for (size_t i = 0; i < _M-d; i++) {
            size_t j = i + d;
            vector<bool> seen(_N, false);
            size_t c = 0;
            for (size_t k = j; k > i; k--) {
                for (const auto& vecl : resultMatrix[i][k-1]) {
                    for (const auto& vecr : resultMatrix[k][j]) {
                        if (c == _N) {
                            break;
                        }
                        int result = matrix[vecl[0]][vecr[0]] - 1;
                        if (!seen[result]) {
                            seen[result] = true;
                            c++;
                            resultMatrix[i][j].push_back({result, static_cast<int>(k) - 1, vecl[0], vecr[0]});
                        }
                    }
                    if (c == _N) {
                        break;
                    }
                }
            }
        }
    }
}

string findPath(int target, int i, int j) {
    if (i == j) {
        return to_string(sequence[i]);
    }

    for (auto& vec : resultMatrix[i][j]) {
        if (vec[0] == target) {
            int targetLeft = vec[2];
            int targetRight = vec[3];
            int k = vec[1];
            string leftPath = findPath(targetLeft, i, k);
            string rightPath = findPath(targetRight, k+1, j);
            return "(" + leftPath + " " + rightPath + ")";
        }
    }
    return "";
}

int main() {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);
    readInput();
    resultMatrix = vector<vector<list<vector<int>>>>(_M , vector<list<vector<int>>>(_M , list<vector<int>>()));
    calculateResulteMatriz();
    for (auto& vec : resultMatrix[0][_M-1]) {
        if (vec[0] == target-1) {
            cout << "1" << endl;
            cout << findPath(target-1, 0, _M-1) << endl;
            return 0;
        }
    }
    cout << "0" << endl;
    return 0;
}
