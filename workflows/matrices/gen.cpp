#include <ctime>
#include <iostream>
#include <random>

using namespace std;

int main(int argc, char **argv) {
    uniform_real_distribution<double> unif(0, 1);
    mt19937_64 rnd(clock());
    int n = atoi(argv[1]);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << unif(rnd) << ' ';
        }
        cout << endl;
    }
}
