#pragma once

#include <QWidget>
#include <vector>
#include "WordSearchGenerator.h"

struct FoundWord {
    int startRow, startCol;
    int endRow, endCol;
};

class WordSearchWidget : public QWidget {
    Q_OBJECT
public:
    explicit WordSearchWidget(QWidget* parent = nullptr);

    void setPuzzle(const WordSearchPuzzle& puzzle);
    void clear();
    void setTestMode(bool testing);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    QSize sizeHint() const override;

private:
    WordSearchPuzzle puzzle_;
    int idealCellSize_ = 32;
    bool testMode_ = false;
    bool dragging_ = false;
    int dragStartRow_ = -1;
    int dragStartCol_ = -1;
    int dragEndRow_ = -1;
    int dragEndCol_ = -1;
    std::vector<FoundWord> foundWords_;
    
    void getCellFromPos(const QPoint& pos, int& row, int& col);
    bool checkWord(int startRow, int startCol, int endRow, int endCol);
    std::string extractWord(int startRow, int startCol, int endRow, int endCol);
};

