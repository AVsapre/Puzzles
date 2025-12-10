#include "MazeWidget.h"
#include <QColor>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QWheelEvent>
#include <QSizePolicy>
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace {
std::pair<int, int> nodeRowCol(const MazeGraph& g, const int nodeId) {
    if (nodeId < 0 || nodeId >= static_cast<int>(g.nodes.size())) return {-1, -1};
    const auto& n = g.nodes[nodeId];
    return {n.row, n.col};
}

} 

MazeWidget::MazeWidget(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(420, 420);

    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(16);
    animationTimer_->setTimerType(Qt::PreciseTimer);
    connect(animationTimer_, &QTimer::timeout, this, &MazeWidget::updateAnimation);
}

void MazeWidget::setGraph(const MazeGraph& graph,
                          const bool tested,
                          const bool showMarkers,
                          const int entranceNode,
                          const int exitNode,
                          const int playerNode) {
    graph_ = graph;
    tested_ = tested;
    showMarkers_ = showMarkers;
    entranceNode_ = entranceNode;
    exitNode_ = exitNode;
    playerNode_ = playerNode >= 0 ? playerNode : entranceNode_;

    const auto [pr, pc] = nodeRowCol(graph_, playerNode_);
    playerRow_ = targetRow_ = pr >= 0 ? pr : 0;
    playerCol_ = targetCol_ = pc >= 0 ? pc : 0;

    applySizeFromGraph();
    update();
}

void MazeWidget::setColors(const QColor& walls, const QColor& background) {
    wallColor_ = walls;
    backgroundColor_ = background;
    update();
}

QRect MazeWidget::cellRect(const int row, const int col) const {
    if (graph_.rows <= 0 || graph_.cols <= 0) return {};
    if (row < 0 || col < 0 || row >= graph_.rows || col >= graph_.cols) return {};

    const int cell = cellSizePx();
    const int mazeWidth = graph_.cols * cell;
    const int mazeHeight = graph_.rows * cell;
    const int offsetX = std::max(0, (width() - mazeWidth) / 2);
    const int offsetY = std::max(0, (height() - mazeHeight) / 2);
    return QRect(offsetX + col * cell, offsetY + row * cell, cell, cell);
}

QPointF MazeWidget::cellCenter(const double row, const double col) const {
    if (graph_.rows <= 0 || graph_.cols <= 0) return {};
    if (row < 0.0 || col < 0.0 || row >= static_cast<double>(graph_.rows) || col >= static_cast<double>(graph_.cols)) return {};

    const int cell = cellSizePx();
    const int mazeWidth = graph_.cols * cell;
    const int mazeHeight = graph_.rows * cell;
    const int offsetX = std::max(0, (width() - mazeWidth) / 2);
    const int offsetY = std::max(0, (height() - mazeHeight) / 2);
    return QPointF(offsetX + (col + 0.5) * cell, offsetY + (row + 0.5) * cell);
}

QSize MazeWidget::sizeHint() const {
    if (graph_.rows > 0 && graph_.cols > 0) {
        const int cell = cellSizePx();
        return {graph_.cols * cell, graph_.rows * cell};
    }
    constexpr int defaultCells = 20;
    const int cell = cellSizePx();
    return {defaultCells * cell, defaultCells * cell};
}

void MazeWidget::paintEvent(QPaintEvent* ) {
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor_);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (graph_.nodes.empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Generate a maze to begin");
        return;
    }

    const int cell = cellSizePx();
    const int rows = graph_.rows;
    const int cols = graph_.cols;
    const int mazeWidth = cols * cell;
    const int mazeHeight = rows * cell;

    const int offsetX = std::max(0, (width() - mazeWidth) / 2);
    const int offsetY = std::max(0, (height() - mazeHeight) / 2);

    const int lineWidth = std::max(2, cell / 8);
    const int outerLineWidth = lineWidth * 2;  
    
    struct Segment {
        int x1; int y1; int x2; int y2;
        bool operator==(const Segment& other) const {
            return x1 == other.x1 && y1 == other.y1 && x2 == other.x2 && y2 == other.y2;
        }
    };
    struct SegmentHash {
        size_t operator()(const Segment& s) const noexcept {
            size_t h1 = std::hash<int>()(s.x1);
            size_t h2 = std::hash<int>()(s.y1);
            size_t h3 = std::hash<int>()(s.x2);
            size_t h4 = std::hash<int>()(s.y2);
            return (((h1 * 1315423911u) ^ h2) * 1315423911u ^ h3) * 1315423911u ^ h4;
        }
    };
    
    std::unordered_set<Segment, SegmentHash> innerSegments;
    std::unordered_set<Segment, SegmentHash> outerSegments;
    
    auto addInnerSegment = [&](int x1, int y1, int x2, int y2) {
        if (x1 == x2 && y1 == y2) return;
        if (x2 < x1 || (x1 == x2 && y2 < y1)) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        innerSegments.insert(Segment{x1, y1, x2, y2});
    };
    
    auto addOuterSegment = [&](int x1, int y1, int x2, int y2) {
        if (x1 == x2 && y1 == y2) return;
        if (x2 < x1 || (x1 == x2 && y2 < y1)) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        outerSegments.insert(Segment{x1, y1, x2, y2});
    };

    
    for (const auto& e : graph_.edges) {
        if (e.open) continue;
        const auto& from = graph_.nodes[e.from];
        const auto& to = graph_.nodes[e.to];
        if (from.row == to.row) {
            
            const int cMin = std::min(from.col, to.col);
            const int x = offsetX + (cMin + 1) * cell;
            const int y1 = offsetY + from.row * cell;
            const int y2 = offsetY + (from.row + 1) * cell;
            addInnerSegment(x, y1, x, y2);
        } else if (from.col == to.col) {
            
            const int rMin = std::min(from.row, to.row);
            const int y = offsetY + (rMin + 1) * cell;
            const int x1 = offsetX + from.col * cell;
            const int x2 = offsetX + (from.col + 1) * cell;
            addInnerSegment(x1, y, x2, y);
        }
    }

    
    
    for (int col = 0; col < cols; ++col) {
        int x1 = offsetX + col * cell;
        int x2 = offsetX + (col + 1) * cell;
        if (col == 0) x1 -= outerLineWidth / 2;
        if (col == cols - 1) x2 += outerLineWidth / 2;
        addOuterSegment(x1, offsetY, x2, offsetY);
    }
    
    for (int col = 0; col < cols; ++col) {
        int x1 = offsetX + col * cell;
        int x2 = offsetX + (col + 1) * cell;
        if (col == 0) x1 -= outerLineWidth / 2;
        if (col == cols - 1) x2 += outerLineWidth / 2;
        addOuterSegment(x1, offsetY + mazeHeight, x2, offsetY + mazeHeight);
    }
    
    for (int row = 0; row < rows; ++row) {
        int y1 = offsetY + row * cell;
        int y2 = offsetY + (row + 1) * cell;
        if (row == 0) y1 -= outerLineWidth / 2;
        if (row == rows - 1) y2 += outerLineWidth / 2;
        addOuterSegment(offsetX, y1, offsetX, y2);
    }
    
    for (int row = 0; row < rows; ++row) {
        int y1 = offsetY + row * cell;
        int y2 = offsetY + (row + 1) * cell;
        if (row == 0) y1 -= outerLineWidth / 2;
        if (row == rows - 1) y2 += outerLineWidth / 2;
        addOuterSegment(offsetX + mazeWidth, y1, offsetX + mazeWidth, y2);
    }

    
    painter.setPen(QPen(wallColor_, lineWidth, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    for (const auto& seg : innerSegments) {
        painter.drawLine(seg.x1, seg.y1, seg.x2, seg.y2);
    }

    
    painter.setPen(QPen(wallColor_, outerLineWidth, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    for (const auto& seg : outerSegments) {
        painter.drawLine(seg.x1, seg.y1, seg.x2, seg.y2);
    }

    auto carveGap = [&](int nodeId) {
        if (nodeId < 0 || nodeId >= static_cast<int>(graph_.nodes.size())) return;
        const auto& n = graph_.nodes[nodeId];
        const int x = offsetX + n.col * cell;
        const int y = offsetY + n.row * cell;
        painter.setPen(Qt::NoPen);
        painter.setBrush(backgroundColor_);
        const int gapW = std::max(lineWidth, cell - lineWidth * 2);
        if (n.row == 0) {
            painter.drawRect(QRect(x + (cell - gapW) / 2, y - outerLineWidth, gapW, outerLineWidth * 2));
        } else if (n.row == rows - 1) {
            painter.drawRect(QRect(x + (cell - gapW) / 2, y + cell - outerLineWidth, gapW, outerLineWidth * 2));
        } else if (n.col == 0) {
            painter.drawRect(QRect(x - outerLineWidth, y + (cell - gapW) / 2, outerLineWidth * 2, gapW));
        } else if (n.col == cols - 1) {
            painter.drawRect(QRect(x + cell - outerLineWidth, y + (cell - gapW) / 2, outerLineWidth * 2, gapW));
        }
    };

    carveGap(entranceNode_);
    carveGap(exitNode_);

    if (showMarkers_) {
        const double displayRow = isAnimating_
            ? playerRow_ + (targetRow_ - playerRow_) * animationProgress_
            : playerRow_;
        const double displayCol = isAnimating_
            ? playerCol_ + (targetCol_ - playerCol_) * animationProgress_
            : playerCol_;

        const double cx = offsetX + (displayCol + 0.5) * cell;
        const double cy = offsetY + (displayRow + 0.5) * cell;
        const int size = std::max(2, cell / 2);
        const QRectF markerRect(cx - size / 2.0, cy - size / 2.0, size, size);
        painter.setBrush(QColor(52, 122, 235));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(markerRect);
    }
}

void MazeWidget::keyPressEvent(QKeyEvent* event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        switch (event->key()) {
            case Qt::Key_Plus:
            case Qt::Key_Equal:
                zoomIn();
                event->accept();
                return;
            case Qt::Key_Minus:
                zoomOut();
                event->accept();
                return;
            case Qt::Key_0:
                resetZoom();
                event->accept();
                return;
            default:
                break;
        }
    }

    if (event->isAutoRepeat()) {
        event->accept();
        return;
    }

    Direction direction;
    int keyIndex = -1;
    bool handled = true;

    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up:
            direction = Direction::UP;
            keyIndex = 0;
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            direction = Direction::DOWN;
            keyIndex = 1;
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            direction = Direction::LEFT;
            keyIndex = 2;
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            direction = Direction::RIGHT;
            keyIndex = 3;
            break;
        default: handled = false; break;
    }

    if (handled) {
        if (keyIndex >= 0) {
            keyPressed_[keyIndex] = true;
            currentKeyIndex_ = keyIndex;
        }
        if (!isAnimating_) {
            emit moveRequested(direction);
        }
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void MazeWidget::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) {
        event->accept();
        return;
    }

    int keyIndex = -1;
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up: keyIndex = 0; break;
        case Qt::Key_S:
        case Qt::Key_Down: keyIndex = 1; break;
        case Qt::Key_A:
        case Qt::Key_Left: keyIndex = 2; break;
        case Qt::Key_D:
        case Qt::Key_Right: keyIndex = 3; break;
        default: break;
    }

    if (keyIndex >= 0) {
        keyPressed_[keyIndex] = false;
        if (keyIndex == currentKeyIndex_) {
            currentKeyIndex_ = -1;
        }
        event->accept();
        return;
    }

    QWidget::keyReleaseEvent(event);
}

void MazeWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        lastDragPos_ = event->globalPosition().toPoint();
        if (auto* scrollArea = qobject_cast<QScrollArea*>(parentWidget() ? parentWidget()->parentWidget() : nullptr)) {
            scrollStartOffset_ = QPoint(scrollArea->horizontalScrollBar()->value(), scrollArea->verticalScrollBar()->value());
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void MazeWidget::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_) {
        if (auto* scrollArea = qobject_cast<QScrollArea*>(parentWidget() ? parentWidget()->parentWidget() : nullptr)) {
            const QPoint delta = event->globalPosition().toPoint() - lastDragPos_;
            scrollArea->horizontalScrollBar()->setValue(scrollStartOffset_.x() - delta.x());
            scrollArea->verticalScrollBar()->setValue(scrollStartOffset_.y() - delta.y());
        }
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void MazeWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (dragging_ && event->button() == Qt::LeftButton) {
        dragging_ = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void MazeWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        const int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else if (delta < 0) {
            zoomOut();
        }
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

int MazeWidget::cellSizePx() const {
    return cellPixelSize_;
}

void MazeWidget::applySizeFromGraph() {
    const double scaled = static_cast<double>(baseCellSize_) * zoomFactor_;
    const int cell = std::max(minCellSize_, static_cast<int>(std::round(scaled)));
    cellPixelSize_ = cell;

    if (graph_.rows > 0 && graph_.cols > 0) {
        setFixedSize(graph_.cols * cell, graph_.rows * cell);
    } else {
        setMinimumSize(0, 0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    updateGeometry();
}

void MazeWidget::setZoomFactor(const double factor) {
    const double clamped = std::clamp(factor, 0.4, 3.0);
    int targetCell = static_cast<int>(std::round(static_cast<double>(baseCellSize_) * clamped));
    targetCell = std::max(minCellSize_, targetCell);
    zoomFactor_ = static_cast<double>(targetCell) / static_cast<double>(baseCellSize_);
    applySizeFromGraph();
    update();
}

void MazeWidget::zoomIn() {
    const double next = static_cast<double>(cellPixelSize_ + 1) / static_cast<double>(baseCellSize_);
    setZoomFactor(next);
}

void MazeWidget::zoomOut() {
    const double next = static_cast<double>(std::max(minCellSize_, cellPixelSize_ - 1)) / static_cast<double>(baseCellSize_);
    setZoomFactor(next);
}

void MazeWidget::resetZoom() {
    setZoomFactor(1.0);
}

void MazeWidget::startMove(const int fromRow, const int fromCol, const int toRow, const int toCol) {
    if (isAnimating_) {
        playerRow_ = targetRow_;
        playerCol_ = targetCol_;
        animationTimer_->stop();
    }

    playerRow_ = fromRow;
    playerCol_ = fromCol;
    targetRow_ = toRow;
    targetCol_ = toCol;
    animationProgress_ = 0.0;
    isAnimating_ = true;
    animationClock_.restart();
    animationTimer_->start();
    emit playerDisplayPositionChanged(static_cast<double>(playerRow_), static_cast<double>(playerCol_));
    update();
}

void MazeWidget::tryNextKey() {
    auto emitForIndex = [this](int idx) {
        Direction dir;
        switch (idx) {
            case 0: dir = Direction::UP; break;
            case 1: dir = Direction::DOWN; break;
            case 2: dir = Direction::LEFT; break;
            case 3: dir = Direction::RIGHT; break;
            default: return;
        }
        currentKeyIndex_ = idx;
        emit moveRequested(dir);
    };

    if (currentKeyIndex_ >= 0 && keyPressed_[currentKeyIndex_]) {
        emitForIndex(currentKeyIndex_);
        return;
    }

    for (int i = 0; i < 4; ++i) {
        if (keyPressed_[i]) {
            emitForIndex(i);
            return;
        }
    }

    currentKeyIndex_ = -1;
}

void MazeWidget::updateAnimation() {
    if (!isAnimating_) {
        return;
    }

    const double elapsedMs = static_cast<double>(animationClock_.elapsed());
    const double durationMs = static_cast<double>(animationDurationMs_);
    animationProgress_ = std::clamp(elapsedMs / durationMs, 0.0, 1.0);
    const double displayRow = playerRow_ + (targetRow_ - playerRow_) * animationProgress_;
    const double displayCol = playerCol_ + (targetCol_ - playerCol_) * animationProgress_;
    emit playerDisplayPositionChanged(displayRow, displayCol);

    if (animationProgress_ >= 1.0) {
        animationProgress_ = 1.0;
        isAnimating_ = false;
        animationTimer_->stop();
        playerRow_ = targetRow_;
        playerCol_ = targetCol_;
        tryNextKey();
    }

    update();
}
