#include <iostream>

using namespace std;

int main() {
    int a; 
    int b; 
    int c; 
    a = 16;
    while (a > 10) {
        a = a - 1;
        b = 50 - a;
        c = b + a;
    }
    cout << "a : " << a << endl;
    cout << "b : " << b << endl;
    cout << "c : " << c;
    return 0;
}
