#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

struct CryptogramPuzzle {
    std::string plainText;
    std::string cipherText;
    std::unordered_map<char, char> cipherToPlain;
    std::unordered_set<char> revealed; 
};

class CryptogramGenerator {
public:
    CryptogramPuzzle generate(const std::string& plainText, bool avoidSelfMapping, int hintCount);
};
