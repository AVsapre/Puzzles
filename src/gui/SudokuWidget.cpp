#include "SudokuWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QSize>

SudokuWidget::SudokuWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(360, 360);
    setFocusPolicy(Qt::StrongFocus);
}

void SudokuWidget::setPuzzle(const SudokuPuzzle& puzzle) {
    puzzle_ = puzzle;
    userInput_.clear();
    userInput_.resize(9, std::vector<int>(9, 0));
    resetSelection();
    
    if (!puzzle_.grid.empty()) {
        const int width = 9 * idealCellSize_;
        const int height = 9 * idealCellSize_;
        setFixedSize(width, height);
    }
    
    updateGeometry();
    update();
}

void SudokuWidget::clear() {
    puzzle_ = SudokuPuzzle{};
    userInput_.clear();
    resetSelection();
    update();
}

void SudokuWidget::setTestMode(bool testing) {
    testMode_ = testing;
    if (!testing) {
        resetSelection();
    }
    update();
}

void SudokuWidget::setColors(const QColor& line, const QColor& background) {
    lineColor_ = line;
    backgroundColor_ = background;
    update();
}

QSize SudokuWidget::sizeHint() const {
    return {9 * idealCellSize_, 9 * idealCellSize_};
}

void SudokuWidget::mousePressEvent(QMouseEvent* event) {
    if (!testMode_) {
        QWidget::mousePressEvent(event);
        return;
    }

    const double cell = static_cast<double>(idealCellSize_);
    const double totalWidth = 9 * cell;
    const double totalHeight = 9 * cell;
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

    if (clickRow >= 0 && clickRow < 9 && clickCol >= 0 && clickCol < 9) {
        if (puzzle_.grid[clickRow][clickCol] == 0) {
            selectedRow_ = clickRow;
            selectedCol_ = clickCol;
            setFocus();
            update();
        }
    }
}

void SudokuWidget::keyPressEvent(QKeyEvent* event) {
    if (!testMode_ || selectedRow_ < 0 || selectedCol_ < 0) {
        QWidget::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        if (selectedRow_ < 0 || selectedCol_ < 0) {
            for (int r = 0; r < 9; ++r) {
                bool found = false;
                for (int c = 0; c < 9; ++c) {
                    if (puzzle_.grid[r][c] == 0) {
                        selectedRow_ = r;
                        selectedCol_ = c;
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }
        const int dr = (event->key() == Qt::Key_Up) ? -1 : (event->key() == Qt::Key_Down ? 1 : 0);
        const int dc = (event->key() == Qt::Key_Left) ? -1 : (event->key() == Qt::Key_Right ? 1 : 0);
        if (moveSelectionDelta(dr, dc)) {
            update();
        }
        event->accept();
        return;
    }

    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9) {
        userInput_[selectedRow_][selectedCol_] = event->key() - Qt::Key_0;
        moveToNextCell();
        update();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        userInput_[selectedRow_][selectedCol_] = 0;
        update();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void SudokuWidget::moveToNextCell() {
    if (selectedRow_ < 0 || selectedCol_ < 0) return;

    int r = selectedRow_;
    int c = selectedCol_;
    do {
        c++;
        if (c >= 9) {
            c = 0;
            r++;
        }
        if (r >= 9) {
            r = 0;
        }
    } while (puzzle_.grid[r][c] != 0);
    
    selectedRow_ = r;
    selectedCol_ = c;
}

bool SudokuWidget::moveSelectionDelta(const int dr, const int dc) {
    if (selectedRow_ < 0 || selectedCol_ < 0) return false;
    int r = selectedRow_ + dr;
    int c = selectedCol_ + dc;
    for (int step = 0; step < 81; ++step) {
        if (r < 0 || c < 0 || r >= 9 || c >= 9) {
            return false;
        }
        if (puzzle_.grid[r][c] == 0) {
            selectedRow_ = r;
            selectedCol_ = c;
            return true;
        }
        r += dr;
        c += dc;
    }
    return false;
}

void SudokuWidget::resetSelection() {
    selectedRow_ = -1;
    selectedCol_ = -1;
}

bool SudokuWidget::isCompleted() const {
    if (!testMode_ || userInput_.empty()) return false;
    
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            int value = puzzle_.grid[r][c];
            if (value == 0) {
                value = userInput_[r][c];
            }
            if (value != puzzle_.solution[r][c]) {
                return false;
            }
        }
    }
    return true;
}

void SudokuWidget::paintEvent(QPaintEvent* ) {
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor_);

    if (puzzle_.grid.empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Generate a Sudoku to begin");
        return;
    }

    const double cell = static_cast<double>(idealCellSize_);
    
    const double totalWidth = 9 * cell;
    const double totalHeight = 9 * cell;
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (width() > totalWidth) {
        offsetX = (width() - totalWidth) * 0.5;
    }
    if (height() > totalHeight) {
        offsetY = (height() - totalHeight) * 0.5;
    }

    QFont font = painter.font();
    font.setPointSizeF(std::max(10.0, cell * 0.55));
    painter.setFont(font);

    const bool completed = isCompleted();

    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            const QRectF tile(offsetX + c * cell, offsetY + r * cell, cell, cell);
            
            if (completed) {
                painter.fillRect(tile, QColor(144, 238, 144));
            } else if (testMode_ && r == selectedRow_ && c == selectedCol_) {
                painter.fillRect(tile, QColor(200, 220, 255));
            } else {
                painter.fillRect(tile, backgroundColor_);
            }
            
            painter.setPen(lineColor_);
            
            int value = puzzle_.grid[r][c];
            if (value != 0) {
                painter.drawText(tile, Qt::AlignCenter, QString::number(value));
            } else if (testMode_ && userInput_[r][c] != 0) {
                painter.setPen(lineColor_);
                painter.drawText(tile, Qt::AlignCenter, QString::number(userInput_[r][c]));
            }
            
            painter.setPen(lineColor_);
            painter.drawRect(tile);
        }
    }
    
    painter.setPen(QPen(lineColor_, 3));
    for (int i = 0; i <= 3; ++i) {
        const double y = offsetY + i * 3 * cell;
        const double x = offsetX + i * 3 * cell;
        painter.drawLine(QLineF(offsetX, y, offsetX + 9 * cell, y));
        painter.drawLine(QLineF(x, offsetY, x, offsetY + 9 * cell));
    }
}
