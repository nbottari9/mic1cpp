#include "Control.h"

int main(int argc, char* argv[]) {
    ControlStore test("./prom.dat");
    std::cout << test;
    
    return 0;
}