#include "CryptogramWidget.h"

#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QString>
#include <algorithm>
#include <cctype>

namespace {
QString formatHints(const CryptogramPuzzle& puzzle) {
    if (puzzle.revealed.empty()) {
        return QStringLiteral("Hints: none");
    }
    QString hintText = "Hints: ";
    bool first = true;
    for (char cipher : puzzle.revealed) {
        const auto it = puzzle.cipherToPlain.find(cipher);
        if (it == puzzle.cipherToPlain.end()) continue;
        if (!first) hintText += ", ";
        hintText += QString("%1 = %2")
            .arg(QChar::fromLatin1(cipher))
            .arg(QChar::fromLatin1(it->second));
        first = false;
    }
    return hintText;
}
}

CryptogramWidget::CryptogramWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(200);
    setFocusPolicy(Qt::StrongFocus);
}

void CryptogramWidget::setPuzzle(const CryptogramPuzzle& puzzle) {
    puzzle_ = puzzle;
    userInput_.assign(puzzle_.cipherText.size(), ' ');
    rebuildWordIds();
    
    for (size_t i = 0; i < puzzle_.cipherText.size(); ++i) {
        const char ch = puzzle_.cipherText[i];
        if (ch < 'A' || ch > 'Z') continue;
        if (puzzle_.revealed.count(ch)) {
            const auto it = puzzle_.cipherToPlain.find(ch);
            if (it != puzzle_.cipherToPlain.end()) {
                userInput_[i] = it->second;
            }
        }
    }
    recomputeSolved();
    selectedIndex_ = nextLetterIndex(-1, 1);
    updateGeometry();
    update();
}

void CryptogramWidget::clear() {
    puzzle_ = {};
    userInput_.clear();
    wordIds_.clear();
    wordSolved_.clear();
    boxPositions_.clear();
    selectedIndex_ = -1;
    update();
}

void CryptogramWidget::setTestMode(const bool testing) {
    testMode_ = testing;
    update();
}

QSize CryptogramWidget::sizeHint() const {
    if (puzzle_.cipherText.empty()) {
        return {600, 240};
    }
    const int contentWidth = 800;
    const QSize sz = renderPuzzle(nullptr, puzzle_, nullptr, nullptr, nullptr, -1, false, contentWidth, padding_, cellSize_, nullptr);
    return {sz.width(), std::max(sz.height(), 240)};
}

QSize CryptogramWidget::renderPuzzle(QPainter* painter,
                                     const CryptogramPuzzle& puzzle,
                                     const std::string* userInput,
                                     const std::vector<bool>* wordSolved,
                                     const std::vector<int>* wordIds,
                                     const int selectedIndex,
                                     const bool revealAll,
                                     const int contentWidth,
                                     const int padding,
                                     const int cellSize,
                                     std::vector<QRect>* outBoxes) {
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(mono.pointSize() + 1);
    QFontMetrics metrics(mono);
    const int letterHeight = metrics.height();
    const int lineHeight = cellSize + letterHeight + 12;
    const int spaceAdvance = cellSize / 2;
    const int punctAdvance = cellSize / 2;
    
    int x = 0;
    int y = 0;
    int maxX = 0;
    
    const auto drawBox = [&](int bx, int by, char cipher, char plainChar, bool isSelected, bool solved, size_t charIndex) {
        if (!painter) return;
        const QRect boxRect(padding + bx, padding + by, cellSize, cellSize);
        if (outBoxes && charIndex < puzzle.cipherText.size()) {
            if (outBoxes->size() < puzzle.cipherText.size()) {
                outBoxes->resize(puzzle.cipherText.size());
            }
            (*outBoxes)[charIndex] = boxRect;
        }
        if (solved) {
            painter->fillRect(boxRect, QColor(200, 235, 200));
        } else {
            painter->fillRect(boxRect, QColor(255, 255, 255));
        }
        QPen pen(Qt::black);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(isSelected ? 2 : 1);
        if (isSelected) {
            pen.setColor(QColor(52, 122, 235));
        }
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boxRect);
        if (plainChar != 0) {
            painter->setPen(Qt::black);
            painter->setFont(mono);
            painter->drawText(boxRect, Qt::AlignCenter, QString(QChar::fromLatin1(plainChar)));
        }
        painter->setPen(Qt::black);
        painter->setFont(mono);
        const int baseY = padding + by + cellSize + metrics.ascent();
        painter->drawText(QPoint(padding + bx + (cellSize - metrics.horizontalAdvance(QChar::fromLatin1(cipher))) / 2, baseY),
                          QString(QChar::fromLatin1(cipher)));
    };
    
    const auto drawPunct = [&](int bx, int by, char ch) {
        if (!painter) return;
        painter->setPen(Qt::black);
        painter->setFont(mono);
        const int baseY = padding + by + cellSize + metrics.ascent();
        painter->drawText(QPoint(padding + bx, baseY), QString(QChar::fromLatin1(ch)));
    };
    
    const std::string& text = puzzle.cipherText;
    for (size_t i = 0; i < text.size(); ++i) {
        const char ch = text[i];
        
        if (ch == '\n') {
            x = 0;
            y += lineHeight;
            continue;
        }
        
        if (std::isspace(static_cast<unsigned char>(ch))) {
            x += spaceAdvance;
            continue;
        }
        
        if (!std::isalpha(static_cast<unsigned char>(ch))) {
            if (x + punctAdvance > contentWidth) {
                x = 0;
                y += lineHeight;
            }
            drawPunct(x, y, ch);
            x += punctAdvance;
            maxX = std::max(maxX, x);
            continue;
        }
        
        
        if (x + cellSize > contentWidth) {
            x = 0;
            y += lineHeight;
        }
        
        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        char plainChar = 0;
        const auto it = puzzle.cipherToPlain.find(upper);
        if (it != puzzle.cipherToPlain.end()) {
            if (revealAll || (userInput && (*userInput)[i] != ' ')) {
                plainChar = userInput ? (*userInput)[i] : it->second;
            } else if (puzzle.revealed.count(upper) > 0) {
                plainChar = it->second;
            }
        }
        const bool solved = wordSolved && wordIds && i < wordIds->size() && (*wordIds)[i] >= 0
            ? (*wordSolved)[static_cast<size_t>((*wordIds)[i])]
            : false;
        const bool isSelected = static_cast<int>(i) == selectedIndex;
        drawBox(x, y, upper, plainChar, isSelected, solved, i);
        x += cellSize;
        maxX = std::max(maxX, x);
    }
    
    
    const QString hints = formatHints(puzzle);
    const int hintsHeight = hints.isEmpty() ? 0 : (metrics.height() + 8);
    if (painter && !hints.isEmpty()) {
        painter->setPen(Qt::black);
        painter->setFont(mono);
        const QRect hintRect(padding, padding + y + lineHeight + 4, std::max(contentWidth, maxX), hintsHeight);
        painter->drawText(hintRect, Qt::AlignLeft | Qt::AlignTop, hints);
    }
    
    const int totalHeight = padding + y + lineHeight + hintsHeight + padding;
    const int totalWidth = std::max(maxX, contentWidth) + padding * 2;
    return {totalWidth, totalHeight};
}

void CryptogramWidget::paintEvent(QPaintEvent* ) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    
    if (puzzle_.cipherText.empty()) {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Create or load a cryptogram to begin.");
        return;
    }
    
    const int contentWidth = std::max(10, width() - padding_ * 2);
    renderPuzzle(&painter, puzzle_, &userInput_, &wordSolved_, &wordIds_, selectedIndex_, false, contentWidth, padding_, cellSize_, &boxPositions_);
}

void CryptogramWidget::keyPressEvent(QKeyEvent* event) {
    if (puzzle_.cipherText.empty()) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    auto moveSelection = [&](int delta) {
        const int next = nextLetterIndex(selectedIndex_, delta);
        if (next >= 0) {
            selectedIndex_ = next;
            update();
        }
    };
    
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up) { moveSelection(-1); event->accept(); return; }
    if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Down) { moveSelection(1); event->accept(); return; }
    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(userInput_.size())) {
            userInput_[static_cast<size_t>(selectedIndex_)] = ' ';
            recomputeSolved();
            moveSelection(-1);
            update();
            event->accept();
            return;
        }
    }
    
    const QString txt = event->text().toUpper();
    if (txt.size() == 1 && txt.at(0).isLetter()) {
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(userInput_.size())) {
            userInput_[static_cast<size_t>(selectedIndex_)] = txt.at(0).toLatin1();
            recomputeSolved();
            moveSelection(1);
            update();
            event->accept();
            return;
        }
    }
    
    QWidget::keyPressEvent(event);
}

void CryptogramWidget::mousePressEvent(QMouseEvent* event) {
    if (boxPositions_.empty()) {
        QWidget::mousePressEvent(event);
        return;
    }
    for (size_t i = 0; i < boxPositions_.size(); ++i) {
        if (boxPositions_[i].contains(event->pos())) {
            selectedIndex_ = static_cast<int>(i);
            setFocus();
            update();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void CryptogramWidget::rebuildWordIds() {
    wordIds_.assign(puzzle_.cipherText.size(), -1);
    int wordId = -1;
    bool inWord = false;
    for (size_t i = 0; i < puzzle_.cipherText.size(); ++i) {
        const char ch = puzzle_.cipherText[i];
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            if (!inWord) {
                ++wordId;
                inWord = true;
            }
            wordIds_[i] = wordId;
        } else {
            inWord = false;
        }
    }
    if (wordId >= 0) {
        wordSolved_.assign(static_cast<size_t>(wordId + 1), false);
    } else {
        wordSolved_.clear();
    }
}

void CryptogramWidget::recomputeSolved() {
    if (wordSolved_.empty()) return;
    std::fill(wordSolved_.begin(), wordSolved_.end(), true);
    for (size_t i = 0; i < puzzle_.cipherText.size(); ++i) {
        const int w = wordIds_.empty() ? -1 : wordIds_[i];
        if (w < 0) continue;
        const char cipher = puzzle_.cipherText[i];
        const auto it = puzzle_.cipherToPlain.find(cipher);
        if (it == puzzle_.cipherToPlain.end()) {
            wordSolved_[static_cast<size_t>(w)] = false;
            continue;
        }
        const char target = it->second;
        const char input = userInput_.empty() ? ' ' : userInput_[i];
        if (std::toupper(static_cast<unsigned char>(input)) != std::toupper(static_cast<unsigned char>(target))) {
            wordSolved_[static_cast<size_t>(w)] = false;
        }
    }
}

int CryptogramWidget::nextLetterIndex(const int from, const int delta) const {
    if (puzzle_.cipherText.empty()) return -1;
    int idx = from;
    for (size_t step = 0; step < puzzle_.cipherText.size(); ++step) {
        idx += delta;
        if (idx < 0) idx = static_cast<int>(puzzle_.cipherText.size()) - 1;
        if (idx >= static_cast<int>(puzzle_.cipherText.size())) idx = 0;
        const char ch = puzzle_.cipherText[static_cast<size_t>(idx)];
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            return idx;
        }
    }
    return -1;
}
