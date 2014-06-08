#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <list>
#include <cmath>

#include "config.h"
#include "code.h"
#include "response.h"

using namespace std;

// An unused code
struct UnusedCode {
    size_t score;
    Code code;
    UnusedCode(Code c): score(0), code(c) {}
};

// All unused codes
list<UnusedCode> unusedCodes;

// Possible guesses
list<Code> s;

// Set up the array of unused and possible codes
void setupCodes() {
    for (size_t i = 0; i < pow(Config::NumColours, Config::NumPegs); i++) {
        Code code(i);
        s.push_back(code);
        UnusedCode unusedCode(code);
        unusedCodes.push_back(unusedCode);
    }
}

// Mark a given code as "used"
void removeUsedCode(const Code &code) {
    for (list<UnusedCode>::iterator i = unusedCodes.begin(); i != unusedCodes.end();) {
        if ((*i).code == code) {
            i = unusedCodes.erase(i);
        } else {
            i++;
        }
    }
}

// Remove all codes from s that would not give the same response
void removeNonMatchingCodes(const Code &guess, const Response &response) {
    for (list<Code>::iterator i = s.begin(); i != s.end();) {
        Response r = guess.respond(*i);
        if (r == response) {
            i++;
        } else {
            i = s.erase(i);
        }
    }
}

Code findBestGuess() {
    // Minimax should go here instead
    if (s.empty()) {
        return (*unusedCodes.begin()).code;
    } else {
        return *s.begin();
    }
}

// Game loop
int main(int argc, const char * argv[]) {
    srand((unsigned)time(0));
    setupCodes();
    Code code = Code::random();
    cout << "Code to break: " << code.repr() << endl;
    
    Code guess("1122"); // Initial guess
    for (size_t i = 1;; i++) {
        removeUsedCode(guess);
        Response response = code.respond(guess);
        cout << i << " Guessing " << guess.repr() << ": " << response.repr() << endl;
        if (response.isSolved()) {
            cout << "Solved!" << endl;
            return 0;
        }
        removeNonMatchingCodes(guess, response);
        guess = findBestGuess();
    }
    
    return 0;
}

