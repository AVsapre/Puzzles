#pragma once

#include <QWidget>
#include <utility>

#include "CrosswordGenerator.h"

class CrosswordWidget : public QWidget {
    Q_OBJECT
public:
    explicit CrosswordWidget(QWidget* parent = nullptr);

    void setPuzzle(const CrosswordPuzzle& puzzle);
    void clear();
    void setTestMode(bool testing);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QSize sizeHint() const override;

private:
    CrosswordPuzzle puzzle_;
    int idealCellSize_ = 36;
    bool testMode_ = false;
    int selectedRow_ = -1;
    int selectedCol_ = -1;
    std::vector<std::vector<char>> userInput_;
    enum class EntryDir { Unset, Across, Down };
    EntryDir entryDirection_ = EntryDir::Unset;
    
    bool canMoveRight(int row, int col) const;
    bool canMoveDown(int row, int col) const;
    bool canMoveTo(int row, int col) const;
    std::pair<int, int> directionDelta() const;
    void moveToNextCell();
    void moveToPreviousCell();
    void setDirectionIfUnset();
    void resetSelectionState();
    std::vector<std::vector<bool>> computeCorrectCells() const;
    bool isCompleted() const;
};
