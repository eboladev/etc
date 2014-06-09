#include <string>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

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
    static bool isSeeded = false;
    if (!isSeeded) {
        srand((unsigned)time(0));
        isSeeded = true;
    }
    string pegs;
    for (size_t i = 0; i < Config::NumPegs; i++) {
        pegs += '1' + (unsigned)rand() % Config::NumColours;
    }
    return Code(pegs);
}

// Respond to a guess
Response Code::respond(const Code &guess) const {
    // Count full matches
    int fullMatches = 0;
    for (size_t i = 0; i < Config::NumPegs; i++) {
        if (r[i] == guess.r[i]) {
            fullMatches++;
        }
    }

    // Count colour matches
    int colourMatches = 0;
    vector<size_t> colourMap = mapColours();
    vector<size_t> guessColourMap = guess.mapColours();
    for (size_t i = 0; i < Config::NumColours; i++) {
        colourMatches += min(colourMap[i], guessColourMap[i]);
    }
    
    return Response(fullMatches, colourMatches - fullMatches);
}

// Get string representation
string Code::repr() const {
    return r;
}

// Compare
bool Code::operator==(const Code &other) const {
    return r == other.r;
}

// Get the colour distribution
vector<size_t> Code::mapColours() const {
    vector<size_t> colourMap;
    for (size_t i = 0; i < Config::NumColours; i++) {
        char colour = '1' + i;
        size_t colourCount = count(r.begin(), r.end(), colour);
        colourMap.push_back(colourCount);
    }
    return colourMap;
}
