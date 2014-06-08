#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>

#include "config.h"
#include "response.h"
#include "code.h"

using namespace std;

// Game loop
int main(int argc, const char * argv[]) {
    srand((unsigned)time(0));
    Code code = Code::random();
    cout << "Code to break: " << code.repr() << endl;
    
    string line;
    for (int iter = 1;; iter++) {
        cout << iter << "> ";
        getline(cin, line);
        Code guess = Code(line);
        Response response = code.respond(guess);
        cout << "\t\t" << guess.repr() << ": " << response.repr() << endl;
        if (response.isSolved()) {
            cout << "You won!" << endl;
            return 0;
        }
    }
}

