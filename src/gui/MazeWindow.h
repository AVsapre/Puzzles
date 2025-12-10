#pragma once

#include <QMainWindow>
#include <QString>
#include <QColor>
#include <QPrinter>
#include <memory>

#include <string>
#include <utility>
#include <unordered_map>
#include <optional>
#include <vector>

#include "MazeGame.h"
#include "MazeWidget.h"
#include "CrosswordWidget.h"
#include "CrosswordGenerator.h"
#include "WordSearchWidget.h"
#include "WordSearchGenerator.h"
#include "SudokuWidget.h"
#include "SudokuGenerator.h"
#include "CryptogramWidget.h"
#include "CryptogramGenerator.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QTextEdit;
class QCheckBox;
class QSpinBox;
class QToolBar;
class QAction;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QScrollArea;
class QSlider;
class QVBoxLayout;
class QDialog;

class MazeWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MazeWindow(QWidget* parent = nullptr);

private slots:
    void generateMaze();
    void handleMove(Direction direction);
    void showGenerator();
    void cancelGenerator();
    void handleSelectionChanged(int row);
    void deleteSelected();
    void goHome();
    void showContextMenu(const QPoint& pos);
    void testSelected();
    void saveSelectedImage();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    enum class ActiveMode { None, Maze, Crossword, WordSearch, Sudoku, Cryptogram };

    void updateStatus();
    void updateSizeControls();
    void updateGeneratorView();
    void centerMazeOnPosition(double row, double col);
    void centerMazeOnCell(int row, int col);
    void addWordRow(const QString& word = {}, const QString& hint = {});
    void removeWordRow(QWidget* rowWidget);
    void loadMaze(int index);
    void loadCrossword(int index);
    void loadWordSearch(int index);
    void loadSudoku(int index);
    void clearActivePuzzle();
    void refreshActions();
    QString algorithmLabel(GenerationAlgorithm algorithm) const;
    void generateCrossword();
    void generateWordSearch();
    void generateSudoku();
    void showCrossword(const CrosswordPuzzle& puzzle);
    void showWordSearch(const WordSearchPuzzle& puzzle);
    void showSudoku(const SudokuPuzzle& puzzle);
    void printActivePuzzle();
    void printPreviewActivePuzzle();
    void renderActivePuzzle(QPrinter& printer);
    bool renderActivePuzzleIfAvailable();
    std::unique_ptr<QWidget> makeRenderWidgetForSelection(int row) const;
    std::vector<std::pair<std::string, std::string>> collectWords() const;
    static std::string cleanWord(const QString& raw);
    void saveCrosswordImage();
    void saveWordSearchImage();
    void saveSudokuImage();
    QString sudokuDifficultyLabel(int difficulty) const;
    void persistState() const;
    void restoreState();
    QString stateFilePath() const;
    void createMenusAndToolbars();
    void updateActionStates();
    void updateStatusBarText(const QString& message);
    void applyMazeColors();
    void pickMazeWallColor();
    void pickMazeBackgroundColor();
    void pickMazeColorsHex();
    void saveActiveProgress();
    bool eventFilter(QObject* watched, QEvent* event) override;

    bool inTestMode_ = false;
    bool isRestoring_ = false;

    MazeGame game_;
    ActiveMode activeMode_ = ActiveMode::None;
    MazeWidget* mazeWidget_ = nullptr;
    CrosswordWidget* crosswordWidget_ = nullptr;
    WordSearchWidget* wordSearchWidget_ = nullptr;
    SudokuWidget* sudokuWidget_ = nullptr;
    CryptogramWidget* cryptogramWidget_ = nullptr;
    WordSearchGenerator wordSearchGen_;
    CryptogramGenerator cryptogramGen_;
    QStackedWidget* puzzleViewStack_ = nullptr;
    QWidget* mazeView_ = nullptr;
    QWidget* crosswordView_ = nullptr;
    QWidget* wordSearchView_ = nullptr;
    QWidget* sudokuView_ = nullptr;
    QWidget* cryptogramView_ = nullptr;
    QScrollArea* mazeScroll_ = nullptr;
    QScrollArea* crosswordScroll_ = nullptr;
    QScrollArea* wordSearchScroll_ = nullptr;
    QScrollArea* sudokuScroll_ = nullptr;
    QScrollArea* cryptogramScroll_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* coordStatusLabel_ = nullptr;
    QListWidget* savedList_ = nullptr;
    QStackedWidget* rightStack_ = nullptr;
    QWidget* playPage_ = nullptr;
    QWidget* generatePage_ = nullptr;
    QStackedWidget* generatorStack_ = nullptr;
    QWidget* mazeGeneratorPage_ = nullptr;
    QWidget* crosswordGeneratorPage_ = nullptr;
    QWidget* wordSearchGeneratorPage_ = nullptr;
    QWidget* sudokuGeneratorPage_ = nullptr;
    QWidget* cryptogramGeneratorPage_ = nullptr;
    QComboBox* puzzleTypeCombo_ = nullptr;
    QComboBox* algorithmCombo_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QSpinBox* widthSpin_ = nullptr;
    QSpinBox* heightSpin_ = nullptr;
    QComboBox* startModeCombo_ = nullptr;
    QSpinBox* startColSpin_ = nullptr;
    QSpinBox* startRowSpin_ = nullptr;
    QComboBox* exitModeCombo_ = nullptr;
    QSpinBox* exitColSpin_ = nullptr;
    QSpinBox* exitRowSpin_ = nullptr;
    QWidget* wordListContainer_ = nullptr;
    QVBoxLayout* wordListLayout_ = nullptr;
    QPushButton* addWordButton_ = nullptr;
    struct WordRow {
        QWidget* widget = nullptr;
        QLineEdit* word = nullptr;
        QLineEdit* hint = nullptr;
        QPushButton* remove = nullptr;
    };
    std::vector<WordRow> wordRows_;
    QComboBox* tessSizeCombo_ = nullptr;
    QLabel* widthLabel_ = nullptr;
    QLabel* heightLabel_ = nullptr;
    QLabel* startColLabel_ = nullptr;
    QLabel* startRowLabel_ = nullptr;
    QLabel* exitModeLabel_ = nullptr;
    QLabel* exitColLabel_ = nullptr;
    QLabel* exitRowLabel_ = nullptr;
    QLabel* tessSizeLabel_ = nullptr;
    QLabel* generatorErrorLabel_ = nullptr;
    QLabel* coordLabel_ = nullptr;
    QLabel* acrossLabel_ = nullptr;
    QLabel* downLabel_ = nullptr;
    QLabel* wordsLabel_ = nullptr;
    QSpinBox* wordSearchSizeSpin_ = nullptr;
    QSpinBox* wordCountSpin_ = nullptr;
    QLineEdit* sudokuNameEdit_ = nullptr;
    QComboBox* sudokuDifficultyCombo_ = nullptr;
    QLineEdit* cryptogramNameEdit_ = nullptr;
    QTextEdit* cryptogramPlaintextEdit_ = nullptr;
    QSpinBox* cryptogramHintSpin_ = nullptr;
    QCheckBox* cryptogramNoSelfMapCheck_ = nullptr;
    std::optional<CrosswordPuzzle> currentCrossword_;
    std::optional<WordSearchPuzzle> currentWordSearch_;
    std::optional<SudokuPuzzle> currentSudoku_;
    std::optional<CryptogramPuzzle> currentCryptogram_;
    std::unordered_map<std::string, std::string> lastCrosswordHints_;
    int loadedIndex_ = -1;

    struct SavedMaze {
        std::string name;
        GenerationAlgorithm algorithm;
        int width = 0;
        int height = 0;
        MazeGraph graph;
        int entranceNode = -1;
        int exitNode = -1;
        int playerNode = -1;
    };
    
    struct SavedCrossword {
        std::string name;
        CrosswordPuzzle puzzle;
        std::unordered_map<std::string, std::string> hints;
    };
    
    struct SavedWordSearch {
        std::string name;
        WordSearchPuzzle puzzle;
    };

    struct SavedSudoku {
        std::string name;
        SudokuPuzzle puzzle;
        int difficulty = 0;
    };
    
    struct SavedCryptogram {
        std::string name;
        CryptogramPuzzle puzzle;
        bool avoidSelfMapping = true;
        int hintCount = 0;
    };
    
    struct SavedPuzzle {
        enum class Type { Maze, Crossword, WordSearch, Sudoku, Cryptogram };
        Type type;
        std::optional<SavedMaze> maze;
        std::optional<SavedCrossword> crossword;
        std::optional<SavedWordSearch> wordSearch;
        std::optional<SavedSudoku> sudoku;
        std::optional<SavedCryptogram> cryptogram;
    };
    
    std::vector<SavedPuzzle> savedPuzzles_;
    std::vector<SavedMaze> savedMazes_;
    std::vector<SavedCryptogram> savedCryptograms_;
    QAction* newAction_ = nullptr;
    QAction* playAction_ = nullptr;
    QAction* endTestAction_ = nullptr;
    QAction* saveImageAction_ = nullptr;
    QAction* exitAction_ = nullptr;
    QAction* zoomInAction_ = nullptr;
    QAction* zoomOutAction_ = nullptr;
    QAction* zoomResetAction_ = nullptr;
    QDialog* generatorDialog_ = nullptr;
    QColor mazeWallColor_{30, 30, 30};
    QColor mazeBackgroundColor_{Qt::white};
    
    void generateCryptogram();
    void showCryptogram(const CryptogramPuzzle& puzzle);
    void loadCryptogram(int index);
    void saveCryptogramImage();
};
