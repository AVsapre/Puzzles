#include "CrosswordWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QSize>
#include <cctype>
#include <utility>

CrosswordWidget::CrosswordWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(420, 420);
    setFocusPolicy(Qt::StrongFocus);
}

void CrosswordWidget::setPuzzle(const CrosswordPuzzle& puzzle) {
    puzzle_ = puzzle;
    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
    userInput_.clear();
    userInput_.resize(rows, std::vector<char>(cols, ' '));
    resetSelectionState();
    
    if (rows > 0 && cols > 0) {
        const int width = cols * idealCellSize_;
        const int height = rows * idealCellSize_;
        setFixedSize(width, height);
    }
    
    updateGeometry();
    update();
}

void CrosswordWidget::clear() {
    puzzle_ = CrosswordPuzzle{};
    userInput_.clear();
    resetSelectionState();
    update();
}

void CrosswordWidget::setTestMode(bool testing) {
    testMode_ = testing;
    if (!testing) {
        resetSelectionState();
    }
    update();
}

QSize CrosswordWidget::sizeHint() const {
    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 15 : static_cast<int>(puzzle_.grid.front().size());
    return {cols * idealCellSize_, rows * idealCellSize_};
}

void CrosswordWidget::mousePressEvent(QMouseEvent* event) {
    if (!testMode_) {
        QWidget::mousePressEvent(event);
        return;
    }

    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
    if (rows == 0 || cols == 0) return;

    const double cell = static_cast<double>(idealCellSize_);
    const double totalWidth = cols * cell;
    const double totalHeight = rows * cell;
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (width() > totalWidth) {
        offsetX = (width() - totalWidth) * 0.5;
    }
    if (height() > totalHeight) {
        offsetY = (height() - totalHeight) * 0.5;
    }

    const int clickCol = static_cast<int>((event->pos().x() - offsetX) / cell);
    const int clickRow = static_cast<int>((event->pos().y() - offsetY) / cell);

    if (clickRow >= 0 && clickRow < rows && clickCol >= 0 && clickCol < cols) {
        if (puzzle_.grid[clickRow][clickCol] != '#') {
            selectedRow_ = clickRow;
            selectedCol_ = clickCol;
            entryDirection_ = EntryDir::Unset;
            setFocus();
            update();
        }
    }
}

void CrosswordWidget::keyPressEvent(QKeyEvent* event) {
    if (!testMode_ || selectedRow_ < 0 || selectedCol_ < 0) {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        const int dr = (event->key() == Qt::Key_Up) ? -1 : (event->key() == Qt::Key_Down ? 1 : 0);
        const int dc = (event->key() == Qt::Key_Left) ? -1 : (event->key() == Qt::Key_Right ? 1 : 0);
        if (dr != 0 || dc != 0) {
            entryDirection_ = (dr != 0) ? EntryDir::Down : EntryDir::Across;

            if (selectedRow_ < 0 || selectedCol_ < 0) {
                
                for (int r = 0; r < static_cast<int>(puzzle_.grid.size()); ++r) {
                    for (int c = 0; c < static_cast<int>(puzzle_.grid.front().size()); ++c) {
                        if (puzzle_.grid[r][c] != '#') {
                            selectedRow_ = r;
                            selectedCol_ = c;
                            break;
                        }
                    }
                    if (selectedRow_ >= 0) break;
                }
            } else {
                const int rows = static_cast<int>(puzzle_.grid.size());
                const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
                if (rows == 0 || cols == 0) {
                    update();
                    event->accept();
                    return;
                }
                int nr = selectedRow_ + dr;
                int nc = selectedCol_ + dc;
                while (nr >= 0 && nc >= 0 && nr < rows && nc < cols) {
                    if (canMoveTo(nr, nc)) {
                        selectedRow_ = nr;
                        selectedCol_ = nc;
                        break;
                    }
                    nr += dr;
                    nc += dc;
                }
            }
            update();
            event->accept();
            return;
        }
    }

    const QString text = event->text().toUpper();
    if (text.length() == 1) {
        const QChar ch = text.at(0);
        if (ch.isLetter() || ch == ' ') {
            userInput_[selectedRow_][selectedCol_] = ch.toLatin1();
            moveToNextCell();
            update();
            event->accept();
            return;
        }
    }

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        userInput_[selectedRow_][selectedCol_] = ' ';
        moveToPreviousCell();
        update();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

bool CrosswordWidget::canMoveRight(int row, int col) const {
    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
    if (col + 1 >= cols) return false;
    return puzzle_.grid[row][col + 1] != '#';
}

bool CrosswordWidget::canMoveDown(int row, int col) const {
    const int rows = static_cast<int>(puzzle_.grid.size());
    if (row + 1 >= rows) return false;
    return puzzle_.grid[row + 1][col] != '#';
}

bool CrosswordWidget::canMoveTo(int row, int col) const {
    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
    if (row < 0 || col < 0 || row >= rows || col >= cols) return false;
    return puzzle_.grid[row][col] != '#';
}

std::pair<int, int> CrosswordWidget::directionDelta() const {
    switch (entryDirection_) {
        case EntryDir::Across: return {0, 1};
        case EntryDir::Down: return {1, 0};
        case EntryDir::Unset: default: return {0, 0};
    }
}

void CrosswordWidget::moveToNextCell() {
    if (selectedRow_ < 0 || selectedCol_ < 0) return;
    
    setDirectionIfUnset();

    const auto [dr, dc] = directionDelta();
    if (dr == 0 && dc == 0) {
        return;
    }

    const int nextRow = selectedRow_ + dr;
    const int nextCol = selectedCol_ + dc;
    if (canMoveTo(nextRow, nextCol)) {
        selectedRow_ = nextRow;
        selectedCol_ = nextCol;
    }
}

void CrosswordWidget::moveToPreviousCell() {
    if (selectedRow_ < 0 || selectedCol_ < 0) return;

    setDirectionIfUnset();
    const auto [dr, dc] = directionDelta();

    int prevRow = selectedRow_ - dr;
    int prevCol = selectedCol_ - dc;

    
    if (dr == 0 && dc == 0) {
        if (canMoveTo(selectedRow_, selectedCol_ - 1)) {
            prevRow = selectedRow_;
            prevCol = selectedCol_ - 1;
        } else if (canMoveTo(selectedRow_ - 1, selectedCol_)) {
            prevRow = selectedRow_ - 1;
            prevCol = selectedCol_;
        } else {
            return;
        }
    }

    if (canMoveTo(prevRow, prevCol)) {
        selectedRow_ = prevRow;
        selectedCol_ = prevCol;
    }
}

void CrosswordWidget::setDirectionIfUnset() {
    if (entryDirection_ != EntryDir::Unset) {
        return;
    }
    if (canMoveRight(selectedRow_, selectedCol_)) {
        entryDirection_ = EntryDir::Across;
    } else if (canMoveDown(selectedRow_, selectedCol_)) {
        entryDirection_ = EntryDir::Down;
    }
}

void CrosswordWidget::resetSelectionState() {
    selectedRow_ = -1;
    selectedCol_ = -1;
    entryDirection_ = EntryDir::Unset;
}

std::vector<std::vector<bool>> CrosswordWidget::computeCorrectCells() const {
    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
    std::vector<std::vector<bool>> correct(rows, std::vector<bool>(cols, false));

    if (!testMode_ || userInput_.empty()) {
        return correct;
    }

    auto isBlock = [&](int r, int c) {
        return puzzle_.grid[r][c] == '#';
    };

    auto markRange = [&](int r, int c, int dr, int dc) {
        int rr = r;
        int cc = c;
        bool wordCorrect = true;
        std::vector<std::pair<int, int>> cells;
        while (rr >= 0 && rr < rows && cc >= 0 && cc < cols && !isBlock(rr, cc)) {
            cells.emplace_back(rr, cc);
            const char expected = static_cast<char>(std::toupper(static_cast<unsigned char>(puzzle_.grid[rr][cc])));
            const char input = static_cast<char>(std::toupper(static_cast<unsigned char>(userInput_[rr][cc])));
            if (input != expected) {
                wordCorrect = false;
            }
            rr += dr;
            cc += dc;
        }
        if (wordCorrect) {
            for (const auto& [rCell, cCell] : cells) {
                correct[rCell][cCell] = true;
            }
        }
    };

    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const bool startAcross = !isBlock(r, c) && (c == 0 || isBlock(r, c - 1)) && c + 1 < cols && !isBlock(r, c + 1);
            if (startAcross) {
                markRange(r, c, 0, 1);
            }
            const bool startDown = !isBlock(r, c) && (r == 0 || isBlock(r - 1, c)) && r + 1 < rows && !isBlock(r + 1, c);
            if (startDown) {
                markRange(r, c, 1, 0);
            }
        }
    }

    return correct;
}

bool CrosswordWidget::isCompleted() const {
    if (!testMode_ || userInput_.empty()) return false;

    const auto correct = computeCorrectCells();
    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (puzzle_.grid[r][c] != '#' && (r >= static_cast<int>(correct.size()) || c >= static_cast<int>(correct[r].size()) || !correct[r][c])) {
                return false;
            }
        }
    }
    return true;
}

void CrosswordWidget::paintEvent(QPaintEvent* ) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    const int rows = static_cast<int>(puzzle_.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle_.grid.front().size());
    if (rows == 0 || cols == 0) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Generate a crossword to begin");
        return;
    }

    const double cell = static_cast<double>(idealCellSize_);
    
    const double totalWidth = cols * cell;
    const double totalHeight = rows * cell;
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (width() > totalWidth) {
        offsetX = (width() - totalWidth) * 0.5;
    }
    if (height() > totalHeight) {
        offsetY = (height() - totalHeight) * 0.5;
    }

    QFont numFont = painter.font();
    numFont.setPointSizeF(std::max(6.0, cell * 0.28));
    QFont letterFont = painter.font();
    letterFont.setPointSizeF(std::max(10.0, cell * 0.55));

    const auto correctCells = testMode_ ? computeCorrectCells() : std::vector<std::vector<bool>>{};

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const char ch = puzzle_.grid[r][c];
            if (ch != '#') {
                const QRectF tile(offsetX + c * cell, offsetY + r * cell, cell, cell);
                
                const bool correct = r < static_cast<int>(correctCells.size()) &&
                                     c < static_cast<int>(correctCells[r].size()) &&
                                     correctCells[r][c];

                if (correct) {
                    painter.fillRect(tile, QColor(144, 238, 144));
                } else if (testMode_ && r == selectedRow_ && c == selectedCol_) {
                    painter.fillRect(tile, QColor(200, 220, 255));
                } else {
                    painter.fillRect(tile, Qt::white);
                }
                
                const int num = puzzle_.numbers.empty() ? 0 : puzzle_.numbers[r][c];
                if (num > 0) {
                    painter.setPen(Qt::black);
                    painter.setFont(numFont);
                    painter.drawText(tile.adjusted(cell * 0.08, cell * 0.05, -cell * 0.05, -cell * 0.6), Qt::AlignLeft | Qt::AlignTop, QString::number(num));
                }
                
                if (testMode_ && !userInput_.empty() && userInput_[r][c] != ' ') {
                    painter.setFont(letterFont);
                    painter.setPen(Qt::black);
                    painter.drawText(tile, Qt::AlignCenter, QString(userInput_[r][c]));
                }
                
                painter.setFont(letterFont);
                painter.setPen(Qt::black);
                painter.drawRect(tile);
            }
        }
    }
}
