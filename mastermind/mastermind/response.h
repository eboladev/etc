#ifndef mastermind_response_h
#define mastermind_response_h

#include <string>

using namespace std;

// Response: The result of a guess
class Response {
public:
    int blacks;
    int whites;
    
    // Construct from a number of black and white pegs
    Response(int blacks, int whites);
    
    // Does the response indicate a solution?
    bool isSolved() const;
    
    // Get string representation
    string repr() const;
    
    // Compare
    bool operator==(const Response &other) const;
};

#endif
