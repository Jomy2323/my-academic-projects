#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>

using namespace std;

size_t _N, _M, _L;
vector<vector<int>> grafo_linhas;
int max_linhas = 0;

int readInput() {
    cin >> _N >> _M >> _L;
    vector<unordered_set<int>> estacoes(_L); //O(L)
    vector<int> estacoes_visitadas(_N, 0); //O(N)
    vector<int> linhas_visitadas(_L,0); //O(L)
    
    for (size_t i = 0; i < _M; i++) { //O(M)
        int e1, e2, l;
        cin >> e1 >> e2 >> l;
        e1--, e2--, l--;
        estacoes[l].insert(e1);
        estacoes[l].insert(e2);
        estacoes_visitadas[e1] = 1;
        estacoes_visitadas[e2] = 1;
    }
    
    for (int visitada : estacoes_visitadas) { //O(N)
        if (!visitada) {
            return 1;
        }
    }

    grafo_linhas = vector<vector<int>>(_L, vector<int>(_L, 0)); //O(L^2)
    for (size_t i = 0; i < _L-1; i++) { //O(L)
        if(estacoes[i].size() == _N) {
            return 2;
        }
        for (size_t j = i + 1; j < _L; j++) { //O(L)
            for (int estacao : estacoes[i]) { //O(N)
                if (estacoes[j].count(estacao)) {
                    grafo_linhas[i][j] = grafo_linhas[j][i] = 1;
                    break;
                }
            }
        }
    }    
    return 0;
}

int bfs(int start) {
    vector<int> visitados(_L, 0), peso(_L, 0); //O(L)
    queue<int> fila;
    
    fila.push(start);
    visitados[start] = 1;

    while (!fila.empty()) { //O(L)
        int v = fila.front();
        fila.pop();
        for (size_t i = 0; i < _L; i++) { //O(L)
            if (grafo_linhas[v][i] && !visitados[i]) {
                fila.push(i);
                visitados[i] = 1;
                peso[i] = peso[v] + 1;
                max_linhas = max(max_linhas, peso[i]);
            }
        }
    }
    for (int visitada : visitados) {
        if (!visitada) {
            return 1;
        }
    }
    return 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int a = readInput();
    if (a == 1) {
        cout << "-1\n";
        return 0;
    } else if (a == 2) {
        cout << "0\n";
        return 0;
    }
    for (size_t i = 0; i < _L; i++) { //O(L^3)
        if(bfs(i)){
            cout << "-1\n";
            return 0;
        }
    }
    cout << max_linhas << "\n";
    return 0;
}
