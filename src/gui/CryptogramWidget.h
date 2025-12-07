#pragma once

#pragma once

#include <QWidget>
#include <vector>
#include <string>

#include "CryptogramGenerator.h"

class QPainter;

class CryptogramWidget : public QWidget {
    Q_OBJECT
public:
    explicit CryptogramWidget(QWidget* parent = nullptr);

    void setPuzzle(const CryptogramPuzzle& puzzle);
    void clear();
    void setTestMode(bool testing);
    [[nodiscard]] QSize sizeHint() const override;

    
    static QSize renderPuzzle(QPainter* painter,
                              const CryptogramPuzzle& puzzle,
                              const std::string* userInput,
                              const std::vector<bool>* wordSolved,
                              const std::vector<int>* wordIds,
                              int selectedIndex,
                              bool revealAll,
                              int contentWidth,
                              int padding,
                              int cellSize = 32,
                              std::vector<QRect>* outBoxes = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    CryptogramPuzzle puzzle_;
    bool testMode_ = false;
    int padding_ = 12;
    int cellSize_ = 32;
    std::string userInput_;
    int selectedIndex_ = -1;
    std::vector<int> wordIds_;          
    std::vector<bool> wordSolved_;
    std::vector<QRect> boxPositions_;   
    
    void rebuildWordIds();
    void recomputeSolved();
    int nextLetterIndex(int from, int delta) const;
};
