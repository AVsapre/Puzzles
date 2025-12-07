#include "WordSearchGenerator.h"

#include <fstream>
#include <random>
#include <algorithm>
#include <cctype>

WordSearchGenerator::WordSearchGenerator() = default;

bool WordSearchGenerator::loadDictionary(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    dictionary_.clear();
    wordList_.clear();
    std::string word;
    
    while (std::getline(file, word)) {
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::toupper);
        
        if (word.length() >= 3 && word.length() <= 12) {
            dictionary_.insert(word);
            wordList_.push_back(word);
        }
    }
    
    return !dictionary_.empty();
}

WordSearchPuzzle WordSearchGenerator::generate(int size, int wordCount) {
    WordSearchPuzzle puzzle;
    puzzle.size = size;
    puzzle.grid.resize(size, std::vector<char>(size, ' '));
    
    if (wordList_.empty()) {
        fillRandomLetters(puzzle.grid);
        return puzzle;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::vector<std::string> validWords;
    for (const auto& word : wordList_) {
        if (static_cast<int>(word.length()) <= size) {
            validWords.push_back(word);
        }
    }
    
    if (validWords.empty()) {
        fillRandomLetters(puzzle.grid);
        return puzzle;
    }
    
    std::shuffle(validWords.begin(), validWords.end(), gen);
    
    const std::vector<std::pair<int, int>> directions = {
        {0, 1}, {1, 0}, {1, 1}, {1, -1},
        {0, -1}, {-1, 0}, {-1, 1}, {-1, -1}
    };
    
    int placed = 0;
    for (const auto& word : validWords) {
        if (placed >= wordCount) break;
        
        bool wordPlaced = false;
        int attempts = 0;
        const int maxAttempts = 100;
        
        while (!wordPlaced && attempts < maxAttempts) {
            attempts++;
            
            std::uniform_int_distribution<> rowDist(0, size - 1);
            std::uniform_int_distribution<> colDist(0, size - 1);
            std::uniform_int_distribution<> dirDist(0, directions.size() - 1);
            
            int row = rowDist(gen);
            int col = colDist(gen);
            auto [dx, dy] = directions[dirDist(gen)];
            
            if (canPlaceWord(puzzle.grid, word, row, col, dx, dy)) {
                placeWord(puzzle.grid, word, row, col, dx, dy);
                puzzle.words.push_back(word);
                placed++;
                wordPlaced = true;
            }
        }
    }
    
    fillRandomLetters(puzzle.grid);
    return puzzle;
}

bool WordSearchGenerator::canPlaceWord(const std::vector<std::vector<char>>& grid,
                                        const std::string& word, int row, int col,
                                        int dx, int dy) {
    const int size = static_cast<int>(grid.size());
    const int wordLen = static_cast<int>(word.length());
    
    for (int i = 0; i < wordLen; ++i) {
        int r = row + i * dx;
        int c = col + i * dy;
        
        if (r < 0 || r >= size || c < 0 || c >= size) {
            return false;
        }
        
        if (grid[r][c] != ' ' && grid[r][c] != word[i]) {
            return false;
        }
    }
    
    return true;
}

bool WordSearchGenerator::placeWord(std::vector<std::vector<char>>& grid,
                                     const std::string& word, int row, int col,
                                     int dx, int dy) {
    const int wordLen = static_cast<int>(word.length());
    
    for (int i = 0; i < wordLen; ++i) {
        int r = row + i * dx;
        int c = col + i * dy;
        grid[r][c] = word[i];
    }
    
    return true;
}

void WordSearchGenerator::fillRandomLetters(std::vector<std::vector<char>>& grid) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist('A', 'Z');
    
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell == ' ') {
                cell = static_cast<char>(dist(gen));
            }
        }
    }
}
