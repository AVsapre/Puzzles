#pragma once

#include <QMainWindow>

#include <string>
#include <tuple>

#include "MazeGame.h"
#include "MazeWidget.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QScrollArea;
class QSlider;

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
    void solveSelected();
    void saveSelectedImage();
    void changeLayer(int layer);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void updateStatus();
    void updateSizeControls();
    void loadMaze(int index);
    void clearActiveMaze();
    void refreshActions();
    QString algorithmLabel(GenerationAlgorithm algorithm) const;

    MazeGame game_;
    MazeWidget* mazeWidget_ = nullptr;
    QScrollArea* mazeScroll_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QListWidget* savedList_ = nullptr;
    QPushButton* deleteButton_ = nullptr;
    QPushButton* homeButton_ = nullptr;
    QStackedWidget* rightStack_ = nullptr;
    QWidget* playPage_ = nullptr;
    QWidget* generatePage_ = nullptr;
    QComboBox* algorithmCombo_ = nullptr;
    QLineEdit* nameEdit_ = nullptr;
    QSpinBox* widthSpin_ = nullptr;
    QSpinBox* heightSpin_ = nullptr;
    QSpinBox* depthSpin_ = nullptr;
    QSpinBox* layerSpin_ = nullptr;
    QComboBox* tessSizeCombo_ = nullptr;
    QLabel* widthLabel_ = nullptr;
    QLabel* heightLabel_ = nullptr;
    QLabel* depthLabel_ = nullptr;
    QLabel* tessSizeLabel_ = nullptr;
    QLabel* coordLabel_ = nullptr;
    int loadedIndex_ = -1;
    int currentLayer_ = 0;

    struct SavedMaze {
        std::string name;
        GenerationAlgorithm algorithm;
        int width;
        int height;
        int depth;
        std::vector<std::vector<std::vector<unsigned char>>> grid;
        std::tuple<int, int, int> start;
        std::tuple<int, int, int> goal;
    };
    std::vector<SavedMaze> savedMazes_;
};
