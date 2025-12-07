#include "WordSearchWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QSize>
#include <cmath>
#include <algorithm>

WordSearchWidget::WordSearchWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(320, 320);
    setMouseTracking(false);
}

void WordSearchWidget::setPuzzle(const WordSearchPuzzle& puzzle) {
    puzzle_ = puzzle;
    foundWords_.clear();
    dragging_ = false;
    dragStartRow_ = dragStartCol_ = -1;
    dragEndRow_ = dragEndCol_ = -1;
    
    if (puzzle_.size > 0) {
        const int width = puzzle_.size * idealCellSize_;
        const int height = puzzle_.size * idealCellSize_;
        setFixedSize(width, height);
    }
    
    updateGeometry();
    update();
}

void WordSearchWidget::clear() {
    puzzle_ = WordSearchPuzzle{};
    foundWords_.clear();
    dragging_ = false;
    testMode_ = false;
    update();
}

void WordSearchWidget::setTestMode(bool testing) {
    testMode_ = testing;
    if (!testing) {
        foundWords_.clear();
        dragging_ = false;
    }
    update();
}

QSize WordSearchWidget::sizeHint() const {
    const int size = puzzle_.size > 0 ? puzzle_.size : 10;
    return {size * idealCellSize_, size * idealCellSize_};
}

void WordSearchWidget::getCellFromPos(const QPoint& pos, int& row, int& col) {
    const int size = puzzle_.size;
    const double cell = static_cast<double>(idealCellSize_);
    const double totalWidth = size * cell;
    const double totalHeight = size * cell;
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (width() > totalWidth) {
        offsetX = (width() - totalWidth) * 0.5;
    }
    if (height() > totalHeight) {
        offsetY = (height() - totalHeight) * 0.5;
    }

    col = static_cast<int>((pos.x() - offsetX) / cell);
    row = static_cast<int>((pos.y() - offsetY) / cell);
}

void WordSearchWidget::mousePressEvent(QMouseEvent* event) {
    if (!testMode_ || puzzle_.grid.empty()) {
        QWidget::mousePressEvent(event);
        return;
    }

    int row, col;
    getCellFromPos(event->pos(), row, col);
    
    if (row >= 0 && row < puzzle_.size && col >= 0 && col < puzzle_.size) {
        dragging_ = true;
        dragStartRow_ = row;
        dragStartCol_ = col;
        dragEndRow_ = row;
        dragEndCol_ = col;
        update();
    }
}

void WordSearchWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!dragging_) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    int row, col;
    getCellFromPos(event->pos(), row, col);
    
    if (row >= 0 && row < puzzle_.size && col >= 0 && col < puzzle_.size) {
        dragEndRow_ = row;
        dragEndCol_ = col;
        update();
    }
}

void WordSearchWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!dragging_) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    dragging_ = false;
    
    if (checkWord(dragStartRow_, dragStartCol_, dragEndRow_, dragEndCol_)) {
        foundWords_.push_back({dragStartRow_, dragStartCol_, dragEndRow_, dragEndCol_});
    }
    
    dragStartRow_ = dragStartCol_ = -1;
    dragEndRow_ = dragEndCol_ = -1;
    update();
}

std::string WordSearchWidget::extractWord(int startRow, int startCol, int endRow, int endCol) {
    std::string word;
    
    int dx = (endCol > startCol) ? 1 : (endCol < startCol) ? -1 : 0;
    int dy = (endRow > startRow) ? 1 : (endRow < startRow) ? -1 : 0;
    
    int r = startRow, c = startCol;
    while (true) {
        word += puzzle_.grid[r][c];
        if (r == endRow && c == endCol) break;
        r += dy;
        c += dx;
    }
    
    return word;
}

bool WordSearchWidget::checkWord(int startRow, int startCol, int endRow, int endCol) {
    if (startRow == endRow && startCol == endCol) return false;
    
    int dx = endCol - startCol;
    int dy = endRow - startRow;
    
    if (dx != 0 && dy != 0 && std::abs(dx) != std::abs(dy)) {
        return false;
    }
    
    std::string word = extractWord(startRow, startCol, endRow, endCol);
    
    for (const auto& puzzleWord : puzzle_.words) {
        if (word == puzzleWord) {
            for (const auto& found : foundWords_) {
                if ((found.startRow == startRow && found.startCol == startCol &&
                     found.endRow == endRow && found.endCol == endCol) ||
                    (found.startRow == endRow && found.startCol == endCol &&
                     found.endRow == startRow && found.endCol == startCol)) {
                    return false;
                }
            }
            return true;
        }
    }
    
    return false;
}

void WordSearchWidget::paintEvent(QPaintEvent* ) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    if (puzzle_.grid.empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Generate a word search to begin");
        return;
    }

    const int size = puzzle_.size;
    const double cell = static_cast<double>(idealCellSize_);
    
    const double totalWidth = size * cell;
    const double totalHeight = size * cell;
    double offsetX = 0.0;
    double offsetY = 0.0;
    if (width() > totalWidth) {
        offsetX = (width() - totalWidth) * 0.5;
    }
    if (height() > totalHeight) {
        offsetY = (height() - totalHeight) * 0.5;
    }

    QFont letterFont = painter.font();
    letterFont.setPointSizeF(std::max(10.0, cell * 0.5));
    letterFont.setBold(true);

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            const QRectF tile(offsetX + c * cell, offsetY + r * cell, cell, cell);
            
            painter.fillRect(tile, Qt::white);
            painter.setPen(Qt::lightGray);
            painter.drawRect(tile);
            
            painter.setFont(letterFont);
            painter.setPen(Qt::black);
            painter.drawText(tile, Qt::AlignCenter, QString(puzzle_.grid[r][c]));
        }
    }
    
    if (testMode_) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(50, 200, 50, 180));
        
        for (const auto& found : foundWords_) {
            QPointF start(offsetX + found.startCol * cell + cell * 0.5,
                         offsetY + found.startRow * cell + cell * 0.5);
            QPointF end(offsetX + found.endCol * cell + cell * 0.5,
                       offsetY + found.endRow * cell + cell * 0.5);
            
            const double radius = cell * 0.375;
            painter.setPen(QPen(QColor(50, 200, 50), cell * 0.525, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(start, end);
            
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(start, radius, radius);
            painter.drawEllipse(end, radius, radius);
        }
        
        if (dragging_ && dragStartRow_ >= 0 && dragEndRow_ >= 0) {
            QPointF start(offsetX + dragStartCol_ * cell + cell * 0.5,
                         offsetY + dragStartRow_ * cell + cell * 0.5);
            QPointF end(offsetX + dragEndCol_ * cell + cell * 0.5,
                       offsetY + dragEndRow_ * cell + cell * 0.5);
            
            const double radius = cell * 0.375;
            painter.setPen(QPen(QColor(100, 100, 255, 150), cell * 0.525, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(start, end);
            
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(100, 100, 255, 150));
            painter.drawEllipse(start, radius, radius);
            painter.drawEllipse(end, radius, radius);
        }
    }
}
