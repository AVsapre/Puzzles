#pragma once

#include <QWidget>
#include <vector>

#include "maze.h"

class MazeWidget : public QWidget {
    Q_OBJECT
public:
    explicit MazeWidget(QWidget* parent = nullptr);

    void setMaze(std::vector<std::vector<unsigned char>> grid, bool solved);
    [[nodiscard]] QSize sizeHint() const override;

signals:
    void moveRequested(Direction direction);
    void layerChangeRequested(int delta);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    std::vector<std::vector<unsigned char>> grid_;
    bool solved_ = false;
    int idealCellSize_ = 28;
    int minCellSize_ = 10;
};
