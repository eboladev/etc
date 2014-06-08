#ifndef mastermind_code_h
#define mastermind_code_h

#include "response.h"

using namespace std;

// Code: List of coloured pegs representing a code or a guess
class Code {
public:
    string r;
    
    // Construct from a string representation
    Code(const string &r);
    
    // Construct from an integer
    Code(size_t n);
    
    // Construct a random code
    static Code random();
    
    // Respond to a guess
    Response respond(const Code &guess) const;
    
    // Get string representation
    string repr() const;

    // Compare
    bool operator==(const Code &other) const;
};

#endif
