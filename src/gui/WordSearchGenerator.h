#pragma once

#include <string>
#include <vector>
#include <unordered_set>

struct WordSearchPuzzle {
    std::vector<std::vector<char>> grid;
    std::vector<std::string> words;
    int size = 0;
};

class WordSearchGenerator {
public:
    WordSearchGenerator();
    bool loadDictionary(const std::string& filepath);
    WordSearchPuzzle generate(int size, int wordCount);

private:
    std::unordered_set<std::string> dictionary_;
    std::vector<std::string> wordList_;
    
    bool canPlaceWord(const std::vector<std::vector<char>>& grid, 
                      const std::string& word, int row, int col, 
                      int dx, int dy);
    bool placeWord(std::vector<std::vector<char>>& grid, 
                   const std::string& word, int row, int col, 
                   int dx, int dy);
    void fillRandomLetters(std::vector<std::vector<char>>& grid);
};
