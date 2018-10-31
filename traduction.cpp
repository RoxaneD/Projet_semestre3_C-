int main() {
    int i; 
    int j; 
    i = 16;
    j = 1;
    for (j = 1; j < 5; j = j + 1;) {
        i = i - 1;
    }
    for (j = 0; j < 10; j = j + 1;) {
        i = i - 1;
    }
    return 0;
}
