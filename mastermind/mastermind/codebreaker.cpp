#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <list>
#include <cmath>

#include "config.h"
#include "code.h"
#include "response.h"

using namespace std;

// Possible guesses
list<Code> guesses;

// Set up the list of potential guesses
void setupGuesses() {
    for (size_t i = 0; i < pow(Config::NumColours, Config::NumPegs); i++) {
        guesses.push_back(Code(i));
    }
}

// Remove all guesses that would not give the same response
void removeNonMatchingCodes(const Code &guess, const Response &response) {
    for (list<Code>::iterator i = guesses.begin(); i != guesses.end();) {
        Response r = guess.respond(*i);
        if (r == response) {
            i++;
        } else {
            i = guesses.erase(i);
        }
    }
}

// Game loop
int main(int argc, const char * argv[]) {
    srand((unsigned)time(0));
    setupGuesses();
    Code code = Code::random();
    cout << "Code to break: " << code.repr() << endl;
    
    Code guess("1122"); // Initial guess
    while (true) {
        Response response = code.respond(guess);
        cout << "Guessing " << guess.repr() << ": " << response.repr() << endl;
        if (response.isSolved()) {
            cout << "Solved!" << endl;
            return 0;
        }
        removeNonMatchingCodes(guess, response);
        guess = guesses.front();
    }
    
    return 1;
}

