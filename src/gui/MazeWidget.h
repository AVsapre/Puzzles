#pragma once

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QWheelEvent>
#include <QColor>
#include <vector>

#include "maze.h"

class MazeWidget : public QWidget {
    Q_OBJECT
public:
    explicit MazeWidget(QWidget* parent = nullptr);

    void setGraph(const MazeGraph& graph, bool tested, bool showMarkers, int entranceNode, int exitNode, int playerNode);
    void startMove(int fromRow, int fromCol, int toRow, int toCol);
    void tryNextKey();
    [[nodiscard]] QSize sizeHint() const override;
    void setColors(const QColor& walls, const QColor& background);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void setZoomFactor(double factor);
    [[nodiscard]] double zoomFactor() const { return zoomFactor_; }

signals:
    void moveRequested(Direction direction);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    MazeGraph graph_;
    bool tested_ = false;
    bool showMarkers_ = false;
    int baseCellSize_ = 34;
    int minCellSize_ = 10;
    double zoomFactor_ = 1.0;
    int cellPixelSize_ = 28;

    
    QTimer* animationTimer_ = nullptr;
    Direction pendingDirection_ = Direction::UP;
    bool isAnimating_ = false;
    double animationProgress_ = 0.0;
    QElapsedTimer animationClock_;
    int animationDurationMs_ = 100; 
    int playerRow_ = 0;
    int playerCol_ = 0;
    int targetRow_ = 0;
    int targetCol_ = 0;
    bool keyPressed_[4] = {false, false, false, false}; 
    int currentKeyIndex_ = -1; 
    QColor wallColor_{30, 30, 30};
    QColor backgroundColor_{Qt::white};
    bool dragging_ = false;
    QPoint lastDragPos_;
    QPoint scrollStartOffset_;
    int entranceNode_ = -1;
    int exitNode_ = -1;
    int playerNode_ = -1;

    void updateAnimation();
    [[nodiscard]] int cellSizePx() const;
    void applySizeFromGraph();
};
