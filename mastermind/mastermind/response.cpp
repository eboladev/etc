#include <string>

#include "response.h"
#include "config.h"

using namespace std;

// Construct from a string representation
Response::Response(int b, int w) {
    blacks = b;
    whites = w;
}

// Does the response indicate a solution?
bool Response::isSolved() const {
    return blacks == Config::NumPegs && whites == 0;
}

// Get string representation
string Response::repr() const {
    string r;
    for (int i = 0; i < blacks; i++) {
        r += '*';
    }
    for (int i = 0; i < whites; i++) {
        r += 'o';
    }
    return r;
}

// Compare
bool Response::operator==(const Response &other) const {
    return blacks == other.blacks && whites == other.whites;
}