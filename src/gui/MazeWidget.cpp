#include "MazeWidget.h"

#include <QColor>
#include <QKeyEvent>
#include <algorithm>
#include <QPainter>
#include <QPaintEvent>
#include <QSize>

namespace {
QColor cellColor(const unsigned char value, const bool /*solved*/) {
    switch (value) {
        case 1: return QColor(30, 30, 30);
        case 2: return QColor(245, 245, 245); // player rendered as floor for preview
        case 3: return QColor(245, 245, 245); // goal rendered as floor for preview
        case 5: return QColor(245, 245, 245); // down connector background
        case 6: return QColor(245, 245, 245); // up connector background
        case 4: return QColor(180, 55, 55);
        default: return QColor(245, 245, 245);
    }
}
} // namespace

MazeWidget::MazeWidget(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(420, 420);
}

void MazeWidget::setMaze(std::vector<std::vector<unsigned char>> grid, const bool solved) {
    grid_ = std::move(grid);
    solved_ = solved;
    updateGeometry();
    update();
}

QSize MazeWidget::sizeHint() const {
    constexpr int defaultCells = 20;
    return {defaultCells * idealCellSize_, defaultCells * idealCellSize_};
}

void MazeWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(245, 245, 245));

    if (grid_.empty() || grid_.front().empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Generate a maze to begin");
        return;
    }

    const int rows = static_cast<int>(grid_.size());
    const int cols = static_cast<int>(grid_.front().size());

    int playerRow = 0;
    int playerCol = 0;
    bool found = false;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (grid_[r][c] == 2) {
                playerRow = r;
                playerCol = c;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    const int maxViewRows = std::max(1, height() / idealCellSize_);
    const int maxViewCols = std::max(1, width() / idealCellSize_);
    const int viewRows = std::min(rows, maxViewRows);
    const int viewCols = std::min(cols, maxViewCols);

    int top = std::clamp(playerRow - viewRows / 2, 0, std::max(0, rows - viewRows));
    int left = std::clamp(playerCol - viewCols / 2, 0, std::max(0, cols - viewCols));

    const double cellSize = std::max(
        static_cast<double>(minCellSize_),
        std::min(width() / static_cast<double>(viewCols), height() / static_cast<double>(viewRows))
    );

    const double offsetX = (width() - cellSize * viewCols) * 0.5;
    const double offsetY = (height() - cellSize * viewRows) * 0.5;

    for (int vr = 0; vr < viewRows; ++vr) {
        for (int vc = 0; vc < viewCols; ++vc) {
            const int r = top + vr;
            const int c = left + vc;
            const QRectF tile(offsetX + vc * cellSize, offsetY + vr * cellSize, cellSize, cellSize);
            const unsigned char cell = grid_[r][c];
            painter.fillRect(tile, cellColor(cell, solved_));
            if (cell == 5) { // down connector
                painter.setPen(Qt::black);
                painter.setBrush(Qt::black);
                painter.drawEllipse(tile.center(), cellSize * 0.2, cellSize * 0.2);
            } else if (cell == 6) { // up connector
                painter.setPen(QPen(Qt::black, std::max(1.0, cellSize * 0.08)));
                const double pad = cellSize * 0.2;
                painter.drawLine(QPointF(tile.left() + pad, tile.top() + pad), QPointF(tile.right() - pad, tile.bottom() - pad));
                painter.drawLine(QPointF(tile.left() + pad, tile.bottom() - pad), QPointF(tile.right() - pad, tile.top() + pad));
            }
            if (cellSize >= 6.0) {
                painter.setPen(QPen(QColor(0, 0, 0, 20), 0));
                painter.drawRect(tile);
            }
        }
    }

    painter.setPen(QPen(QColor(160, 160, 160), 1));
    painter.drawRect(QRectF(offsetX, offsetY, cellSize * viewCols, cellSize * viewRows));
}

void MazeWidget::keyPressEvent(QKeyEvent* event) {
    Direction direction;
    bool handled = true;

    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up: direction = Direction::UP; break;
        case Qt::Key_S:
        case Qt::Key_Down: direction = Direction::DOWN; break;
        case Qt::Key_A:
        case Qt::Key_Left: direction = Direction::LEFT; break;
        case Qt::Key_D:
        case Qt::Key_Right: direction = Direction::RIGHT; break;
        case Qt::Key_Q: emit layerChangeRequested(-1); event->accept(); return;
        case Qt::Key_E: emit layerChangeRequested(1); event->accept(); return;
        default: handled = false; break;
    }

    if (handled) {
        emit moveRequested(direction);
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}
