#include "CryptogramGenerator.h"

#include <algorithm>
#include <cctype>
#include <random>
#include <vector>

namespace {
std::string toUpperLetters(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (unsigned char ch : input) {
        if (std::isalpha(ch)) {
            out.push_back(static_cast<char>(std::toupper(ch)));
        } else {
            out.push_back(static_cast<char>(ch));
        }
    }
    return out;
}
}

CryptogramPuzzle CryptogramGenerator::generate(const std::string& plainText,
                                               const bool avoidSelfMapping,
                                               int hintCount) {
    CryptogramPuzzle puzzle;
    puzzle.plainText = toUpperLetters(plainText);
    
    std::vector<char> alphabet;
    alphabet.reserve(26);
    for (char c = 'A'; c <= 'Z'; ++c) {
        alphabet.push_back(c);
    }
    
    std::mt19937 rng(std::random_device{}());
    std::vector<char> mapping = alphabet;
    std::shuffle(mapping.begin(), mapping.end(), rng);
    
    if (avoidSelfMapping) {
        for (size_t i = 0; i < mapping.size(); ++i) {
            if (mapping[i] == alphabet[i]) {
                
                for (size_t j = i + 1; j < mapping.size(); ++j) {
                    if (mapping[j] != alphabet[i] && mapping[j] != alphabet[j]) {
                        std::swap(mapping[i], mapping[j]);
                        break;
                    }
                }
                if (mapping[i] == alphabet[i]) {
                    
                    const size_t swapIdx = (i == 0) ? 1 : i - 1;
                    std::swap(mapping[i], mapping[swapIdx]);
                }
            }
        }
    }
    
    for (size_t i = 0; i < alphabet.size(); ++i) {
        puzzle.cipherToPlain[mapping[i]] = alphabet[i];
    }
    
    puzzle.cipherText = puzzle.plainText;
    for (char& ch : puzzle.cipherText) {
        if (ch >= 'A' && ch <= 'Z') {
            const size_t idx = static_cast<size_t>(ch - 'A');
            ch = mapping[idx];
        }
    }
    
    if (hintCount < 0) hintCount = 0;
    if (hintCount > 26) hintCount = 26;
    std::vector<char> cipherLetters = alphabet;
    std::shuffle(cipherLetters.begin(), cipherLetters.end(), rng);
    for (int i = 0; i < hintCount && i < static_cast<int>(cipherLetters.size()); ++i) {
        puzzle.revealed.insert(cipherLetters[i]);
    }
    
    return puzzle;
}
