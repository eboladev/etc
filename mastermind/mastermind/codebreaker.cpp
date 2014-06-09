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
void removeNonMatchingGuesses(const Code &guess, const Response &response) {
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
    setupGuesses();
    Code secret = Code::random();
    cout << "Code to break: " << secret.repr() << endl;
    
    Code guess("1122");
    while (true) {
        Response response = secret.respond(guess);
        cout << "Guessing " << guess.repr() << ": " << response.repr() << endl;
        if (response.isSolved()) {
            cout << "Solved!" << endl;
            return 0;
        }
        removeNonMatchingGuesses(guess, response);
        guess = guesses.front();
    }
}

