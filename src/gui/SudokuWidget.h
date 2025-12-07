#pragma once

#include <QWidget>

#include "SudokuGenerator.h"

class SudokuWidget : public QWidget {
    Q_OBJECT
public:
    explicit SudokuWidget(QWidget* parent = nullptr);

    void setPuzzle(const SudokuPuzzle& puzzle);
    void clear();
    void setTestMode(bool testing);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QSize sizeHint() const override;

private:
    SudokuPuzzle puzzle_;
    int idealCellSize_ = 40;
    bool testMode_ = false;
    int selectedRow_ = -1;
    int selectedCol_ = -1;
    std::vector<std::vector<int>> userInput_;
    
    void moveToNextCell();
    bool moveSelectionDelta(int dr, int dc);
    void resetSelection();
    bool isCompleted() const;
};
