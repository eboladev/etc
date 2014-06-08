#include <string>
#include "stdlib.h"

#include "code.h"
#include "response.h"
#include "config.h"

using namespace std;

// Construct from a string representation
Code::Code(const string &s) {
    r = s;
}

// Construct from an integer
Code::Code(size_t n) {
    static char table[] = "123456789";
    for (size_t i = 0; i < Config::NumPegs; i++) {
        r += '1';
    }
    size_t index = 0;
    while (n >= Config::NumColours) {
        r[index] = table[n % Config::NumColours];
        n /= Config::NumColours;
        index++;
    }
    r[index] = table[n];
}

// Construct a random code
Code Code::random() {
    string pegs;
    for (size_t i = 0; i < Config::NumPegs; i++) {
        pegs += '1' + (unsigned)rand() % Config::NumColours;
    }
    return Code(pegs);
}

// Respond to a guess
Response Code::respond(const Code &other) const {
    int blacks = 0;
    int whites = 0;
    enum Match {
        MatchNone,
        MatchColor,
        MatchFull,
    };
    Match match[Config::NumPegs];
    for (size_t i = 0; i < Config::NumPegs; i++) {
        match[i] = MatchNone;
    }
    
    for (size_t i = 0; i < other.r.length(); i++) {
        if (i == r.length()) {
            break;
        }
        if (r[i] == other.r[i]) {
            match[i] = MatchFull;
        } else {
            size_t index = r.find(other.r[i]);
            if (index != string::npos && match[index] == MatchNone) {
                match[index] = MatchColor;
            }
        }
    }
    for (size_t i = 0; i < Config::NumPegs; i++) {
        if (match[i] == MatchFull) {
            blacks++;
        } else if (match[i] == MatchColor) {
            whites++;
        }
    }
    return Response(blacks, whites);
}

// Get string representation
string Code::repr() const {
    return r;
}

// Compare
bool Code::operator==(const Code &other) const {
    return r == other.r;
}