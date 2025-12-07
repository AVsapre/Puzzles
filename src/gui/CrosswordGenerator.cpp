#include "CrosswordGenerator.h"

#include <algorithm>
#include <cctype>
#include <numeric>
#include <optional>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace {
struct Placement {
    std::string word;
    bool across = true;
    int row = 0;
    int col = 0;
};

static std::mt19937& rng() {
    static thread_local std::mt19937 gen{std::random_device{}()};
    return gen;
}

bool inBounds(int r, int c, int rows, int cols) {
    return r >= 0 && c >= 0 && r < rows && c < cols;
}

bool canPlace(const std::string& word,
              bool across,
              int row,
              int col,
              const std::vector<std::string>& grid) {
    const int rows = static_cast<int>(grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(grid.front().size());
    if (rows == 0 || cols == 0) return false;
    const int len = static_cast<int>(word.size());

    if (across) {
        if (col < 0 || col + len > cols) return false;
    } else {
        if (row < 0 || row + len > rows) return false;
    }

    auto hasLetter = [&](int r, int c) {
        return inBounds(r, c, rows, cols) && std::isalpha(static_cast<unsigned char>(grid[r][c]));
    };

    const int br = row - (across ? 0 : 1);
    const int bc = col - (across ? 1 : 0);
    if (hasLetter(br, bc)) return false;
    const int ar = row + (across ? 0 : len);
    const int ac = col + (across ? len : 0);
    if (hasLetter(ar, ac)) return false;

    for (int i = 0; i < len; ++i) {
        const int r = row + (across ? 0 : i);
        const int c = col + (across ? i : 0);
        const char existing = grid[r][c];
        const bool crossingCell = std::isalpha(static_cast<unsigned char>(existing));
        if (crossingCell && existing != word[i]) {
            return false;
        }

        if (across) {
            if (!crossingCell && (hasLetter(r - 1, c) || hasLetter(r + 1, c))) {
                return false;
            }
        } else {
            if (!crossingCell && (hasLetter(r, c - 1) || hasLetter(r, c + 1))) {
                return false;
            }
        }
    }
    return true;
}

bool placeWord(const std::string& word,
               bool across,
               int row,
               int col,
               std::vector<std::string>& grid,
               std::vector<Placement>& placed,
               int& crossings) {
    if (!canPlace(word, across, row, col, grid)) {
        return false;
    }
    bool crossed = false;
    for (int i = 0; i < static_cast<int>(word.size()); ++i) {
        const int r = row + (across ? 0 : i);
        const int c = col + (across ? i : 0);
        if (std::isalpha(static_cast<unsigned char>(grid[r][c]))) {
            crossed = true;
        }
        grid[r][c] = word[i];
    }
    if (crossed) {
        ++crossings;
    }
    placed.push_back(Placement{word, across, row, col});
    return true;
}

std::vector<std::pair<int, int>> findMatchingLetters(const std::vector<std::string>& grid, char letter) {
    std::vector<std::pair<int, int>> coords;
    for (int r = 0; r < static_cast<int>(grid.size()); ++r) {
        for (int c = 0; c < static_cast<int>(grid[r].size()); ++c) {
            if (grid[r][c] == letter) {
                coords.emplace_back(r, c);
            }
        }
    }
    return coords;
}

void trimGrid(std::vector<std::string>& grid, int& rowOffset, int& colOffset) {
    const int rows = static_cast<int>(grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(grid.front().size());
    int minR = rows, minC = cols, maxR = -1, maxC = -1;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (std::isalpha(static_cast<unsigned char>(grid[r][c]))) {
                minR = std::min(minR, r);
                minC = std::min(minC, c);
                maxR = std::max(maxR, r);
                maxC = std::max(maxC, c);
            }
        }
    }
    if (maxR == -1 || maxC == -1) {
        rowOffset = 0;
        colOffset = 0;
        grid.clear();
        return;
    }
    rowOffset = minR;
    colOffset = minC;
    std::vector<std::string> trimmed;
    for (int r = minR; r <= maxR; ++r) {
        trimmed.push_back(grid[r].substr(minC, maxC - minC + 1));
    }
    grid.swap(trimmed);
}

std::vector<std::string> centerGrid(const std::vector<std::string>& trimmed, int targetRows, int targetCols) {
    if (trimmed.empty()) return {};
    const int tr = static_cast<int>(trimmed.size());
    const int tc = static_cast<int>(trimmed.front().size());
    if (tr > targetRows || tc > targetCols) {
        return {};
    }
    std::vector<std::string> result(targetRows, std::string(targetCols, '#'));
    const int startR = (targetRows - tr) / 2;
    const int startC = (targetCols - tc) / 2;
    for (int r = 0; r < tr; ++r) {
        for (int c = 0; c < tc; ++c) {
            result[startR + r][startC + c] = trimmed[r][c];
        }
    }
    return result;
}

CrosswordPuzzle buildPuzzle(const std::vector<std::string>& rawGrid, int , int ) {
    CrosswordPuzzle puzzle;
    puzzle.grid = rawGrid;
    const int rows = static_cast<int>(puzzle.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle.grid.front().size());
    puzzle.numbers.assign(rows, std::vector<int>(cols, 0));

    int counter = 1;
    auto isBlock = [&](int r, int c) { return puzzle.grid[r][c] == '#'; };
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (isBlock(r, c)) continue;
            const bool startAcross = (c == 0 || isBlock(r, c - 1)) && c + 1 < cols && !isBlock(r, c + 1);
            const bool startDown = (r == 0 || isBlock(r - 1, c)) && r + 1 < rows && !isBlock(r + 1, c);
            if (startAcross || startDown) {
                puzzle.numbers[r][c] = counter;
                ++counter;
            }
        }
    }

    for (int r = 0; r < rows; ++r) {
        int c = 0;
        while (c < cols) {
            if (!isBlock(r, c) && (c == 0 || isBlock(r, c - 1)) && c + 1 < cols && !isBlock(r, c + 1)) {
                const int number = puzzle.numbers[r][c];
                std::string word;
                int cc = c;
                while (cc < cols && !isBlock(r, cc)) {
                    word.push_back(puzzle.grid[r][cc] == ' ' ? ' ' : puzzle.grid[r][cc]);
                    ++cc;
                }
                puzzle.across.push_back(CrosswordEntry{number, word});
                c = cc;
            } else {
                ++c;
            }
        }
    }

    for (int c = 0; c < cols; ++c) {
        int r = 0;
        while (r < rows) {
            if (!isBlock(r, c) && (r == 0 || isBlock(r - 1, c)) && r + 1 < rows && !isBlock(r + 1, c)) {
                const int number = puzzle.numbers[r][c];
                std::string word;
                int rr = r;
                while (rr < rows && !isBlock(rr, c)) {
                    word.push_back(puzzle.grid[rr][c] == ' ' ? ' ' : puzzle.grid[rr][c]);
                    ++rr;
                }
                puzzle.down.push_back(CrosswordEntry{number, word});
                r = rr;
            } else {
                ++r;
            }
        }
    }

    return puzzle;
}

std::string cleanWord(const std::string& raw) {
    std::string out;
    for (char ch : raw) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }
    return out;
}
}

std::optional<CrosswordPuzzle> generateCrossword(const std::vector<std::string>& rawWords, const int rows, const int cols) {
    if (rows < 3 || cols < 3) {
        return std::nullopt;
    }

    std::vector<std::string> words;
    words.reserve(rawWords.size());
    for (const auto& raw : rawWords) {
        auto cleaned = cleanWord(raw);
        if (cleaned.empty()) continue;
        words.push_back(std::move(cleaned));
    }

    std::sort(words.begin(), words.end(), [](const auto& a, const auto& b) {
        return a.size() > b.size();
    });
    if (words.size() > 1) {
        std::shuffle(words.begin() + 1, words.end(), rng());
    }

    std::vector<std::string> grid(rows, std::string(cols, '.'));
    std::vector<Placement> placed;
    int crossings = 0;

    const auto& first = words.front();
    if (!placeWord(first, true, rows / 2, std::max(0, (cols - static_cast<int>(first.size())) / 2), grid, placed, crossings)) {
        return std::nullopt;
    }

    for (size_t wi = 1; wi < words.size(); ++wi) {
        const std::string& word = words[wi];
        bool placedWord = false;

        std::vector<int> letterOrder(word.size());
        std::iota(letterOrder.begin(), letterOrder.end(), 0);
        std::shuffle(letterOrder.begin(), letterOrder.end(), rng());

        for (int li : letterOrder) {
            const char letter = word[li];
            auto matches = findMatchingLetters(grid, letter);
            std::shuffle(matches.begin(), matches.end(), rng());
            for (const auto& [mr, mc] : matches) {
                bool acrossFirst = std::uniform_int_distribution<int>(0, 1)(rng()) == 0;
                const int acrossCol = mc - li;
                const int downRow = mr - li;
                if (acrossFirst) {
                    if (placeWord(word, true, mr, acrossCol, grid, placed, crossings)) { placedWord = true; break; }
                    if (placeWord(word, false, downRow, mc, grid, placed, crossings)) { placedWord = true; break; }
                } else {
                    if (placeWord(word, false, downRow, mc, grid, placed, crossings)) { placedWord = true; break; }
                    if (placeWord(word, true, mr, acrossCol, grid, placed, crossings)) { placedWord = true; break; }
                }
            }
            if (placedWord) break;
        }

        if (!placedWord) {
            std::vector<int> rowOrder(rows);
            std::iota(rowOrder.begin(), rowOrder.end(), 0);
            std::shuffle(rowOrder.begin(), rowOrder.end(), rng());
            std::vector<int> colOrder(cols);
            std::iota(colOrder.begin(), colOrder.end(), 0);
            std::shuffle(colOrder.begin(), colOrder.end(), rng());
            for (int r : rowOrder) {
                for (int c : colOrder) {
                    bool acrossFirst = std::uniform_int_distribution<int>(0, 1)(rng()) == 0;
                    if (acrossFirst) {
                        if (placeWord(word, true, r, c, grid, placed, crossings) || placeWord(word, false, r, c, grid, placed, crossings)) {
                            placedWord = true;
                            break;
                        }
                    } else {
                        if (placeWord(word, false, r, c, grid, placed, crossings) || placeWord(word, true, r, c, grid, placed, crossings)) {
                            placedWord = true;
                            break;
                        }
                    }
                }
                if (placedWord) break;
            }
        }

        if (!placedWord) {
            return std::nullopt;
        }
    }

    for (auto& row : grid) {
        for (auto& ch : row) {
            if (ch == '.') ch = '#';
        }
    }

    int rowOffset = 0;
    int colOffset = 0;
    trimGrid(grid, rowOffset, colOffset);
    if (grid.empty()) {
        return std::nullopt;
    }

    auto centered = centerGrid(grid, rows, cols);
    if (centered.empty()) {
        return std::nullopt;
    }

    if (words.size() > 1 && crossings == 0) {
        return std::nullopt;
    }

    return buildPuzzle(centered, rowOffset, colOffset);
}
