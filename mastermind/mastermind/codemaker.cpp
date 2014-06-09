#include <string>
#include <iostream>

#include "config.h"
#include "code.h"
#include "response.h"

using namespace std;

// Game loop
int main(int argc, const char * argv[]) {
    Code code = Code::random();
    cout << "Code to break: " << code.repr() << endl;
    
    string line;
    while (true) {
        cout << "> ";
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
