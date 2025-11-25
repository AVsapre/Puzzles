#include "MazeWindow.h"

#include <QAbstractItemView>
#include <algorithm>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QPalette>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <tuple>

namespace {
constexpr int kMinUnits = 2;
constexpr int kMaxUnits = 1000;
} // namespace

MazeWindow::MazeWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("MazeUtil");
    resize(1160, 780);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    // Left column: saved mazes list and actions
    savedList_ = new QListWidget(this);
    savedList_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(savedList_, &QListWidget::currentRowChanged, this, &MazeWindow::handleSelectionChanged);
    savedList_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(savedList_, &QListWidget::customContextMenuRequested, this, &MazeWindow::showContextMenu);

    auto* newMazeButton = new QPushButton("New Maze", this);
    connect(newMazeButton, &QPushButton::clicked, this, &MazeWindow::showGenerator);

    deleteButton_ = new QPushButton("Delete", this);
    connect(deleteButton_, &QPushButton::clicked, this, &MazeWindow::deleteSelected);

    auto* leftButtons = new QHBoxLayout;
    leftButtons->addWidget(newMazeButton);
    leftButtons->addWidget(deleteButton_);

    auto* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("Saved Mazes", this));
    leftLayout->addWidget(savedList_, 1);
    leftLayout->addLayout(leftButtons);

    // Right stack: play view + generator view
    rightStack_ = new QStackedWidget(this);

    // Play page (view + controls)
    playPage_ = new QWidget(this);
    mazeWidget_ = new MazeWidget(this);
    mazeScroll_ = new QScrollArea(this);
    mazeScroll_->setWidgetResizable(true);
    mazeScroll_->setBackgroundRole(QPalette::Base);
    mazeScroll_->setWidget(mazeWidget_);

    statusLabel_ = new QLabel("No saved mazes yet. Click New Maze to create one.", this);
    statusLabel_->setWordWrap(true);
    coordLabel_ = new QLabel("Coordinates: N/A", this);
    layerSpin_ = new QSpinBox(this);
    layerSpin_->setRange(1, 1);
    layerSpin_->setPrefix("Layer ");
    layerSpin_->setSuffix("");
    connect(layerSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &MazeWindow::changeLayer);
    homeButton_ = new QPushButton("Back to Home", this);
    connect(homeButton_, &QPushButton::clicked, this, &MazeWindow::goHome);
    homeButton_->setVisible(false);

    auto* statusRow = new QHBoxLayout;
    statusRow->addWidget(statusLabel_, 1);
    statusRow->addWidget(homeButton_);

    auto* layerRow = new QHBoxLayout;
    layerRow->addWidget(new QLabel("Layer:", this));
    layerRow->addWidget(layerSpin_, 0);
    layerRow->addStretch(1);

    auto* playLayout = new QVBoxLayout;
    playLayout->addWidget(mazeScroll_, 1);
    playLayout->addLayout(layerRow);
    playLayout->addWidget(coordLabel_);
    playLayout->addLayout(statusRow);
    playPage_->setLayout(playLayout);

    // Generator page
    generatePage_ = new QWidget(this);
    algorithmCombo_ = new QComboBox(this);
    algorithmCombo_->addItem("Depth-first search", static_cast<int>(GenerationAlgorithm::DFS));
    algorithmCombo_->addItem("Breadth-first search", static_cast<int>(GenerationAlgorithm::BFS));
    algorithmCombo_->addItem("Wilson's algorithm", static_cast<int>(GenerationAlgorithm::Wilson));
    algorithmCombo_->addItem("Kruskal's algorithm", static_cast<int>(GenerationAlgorithm::Kruskal));
    algorithmCombo_->addItem("Prim's algorithm", static_cast<int>(GenerationAlgorithm::Prim));
    algorithmCombo_->addItem("Tessellation", static_cast<int>(GenerationAlgorithm::Tessellation));

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Maze name (optional)");

    widthSpin_ = new QSpinBox(this);
    widthSpin_->setRange(kMinUnits, kMaxUnits);
    widthSpin_->setValue(12);

    heightSpin_ = new QSpinBox(this);
    heightSpin_->setRange(kMinUnits, kMaxUnits);
    heightSpin_->setValue(12);

    depthSpin_ = new QSpinBox(this);
    depthSpin_->setRange(1, kMaxUnits);
    depthSpin_->setValue(1);

    tessSizeCombo_ = new QComboBox(this);
    for (int size : {2, 4, 8, 16, 32, 64}) {
        tessSizeCombo_->addItem(QString::number(size), size);
    }
    tessSizeCombo_->setCurrentIndex(2); // default to 8

    auto* createButton = new QPushButton("Create Maze", this);
    connect(createButton, &QPushButton::clicked, this, &MazeWindow::generateMaze);
    auto* cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &MazeWindow::cancelGenerator);

    auto* formLayout = new QFormLayout;
    formLayout->addRow("Name", nameEdit_);
    formLayout->addRow("Algorithm", algorithmCombo_);
    widthLabel_ = new QLabel("Width (units)", this);
    heightLabel_ = new QLabel("Height (units)", this);
    depthLabel_ = new QLabel("Depth (layers)", this);
    tessSizeLabel_ = new QLabel("Size (units)", this);
    formLayout->addRow(widthLabel_, widthSpin_);
    formLayout->addRow(heightLabel_, heightSpin_);
    formLayout->addRow(depthLabel_, depthSpin_);
    formLayout->addRow(tessSizeLabel_, tessSizeCombo_);

    auto* formButtons = new QHBoxLayout;
    formButtons->addWidget(createButton);
    formButtons->addWidget(cancelButton);

    auto* generateLayout = new QVBoxLayout;
    generateLayout->addLayout(formLayout);
    generateLayout->addLayout(formButtons);
    generateLayout->addStretch(1);
    generatePage_->setLayout(generateLayout);

    rightStack_->addWidget(playPage_);
    rightStack_->addWidget(generatePage_);
    rightStack_->setCurrentWidget(playPage_);

    auto* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(rightStack_, 1);
    central->setLayout(mainLayout);

    connect(mazeWidget_, &MazeWidget::moveRequested, this, &MazeWindow::handleMove);
    connect(mazeWidget_, &MazeWidget::layerChangeRequested, this, [this](int delta) {
        if (game_.moveLayer(delta)) {
            currentLayer_ = game_.currentLayer();
            layerSpin_->blockSignals(true);
            layerSpin_->setValue(currentLayer_ + 1);
            layerSpin_->blockSignals(false);
            mazeWidget_->setMaze(game_.layerGrid(currentLayer_), game_.solved());
            updateStatus();
        }
    });
    connect(algorithmCombo_, &QComboBox::currentIndexChanged, this, [this](int) { updateSizeControls(); });

    updateSizeControls();
    updateStatus();
    refreshActions();
    mazeWidget_->setFocus();
}

void MazeWindow::generateMaze() {
    if (algorithmCombo_->currentIndex() < 0) {
        return;
    }

    const auto algorithm = static_cast<GenerationAlgorithm>(algorithmCombo_->currentData().toInt());
    int widthUnits = widthSpin_->value();
    int heightUnits = heightSpin_->value();
    int depthUnits = depthSpin_->value();

    QString mazeName = nameEdit_->text().trimmed();
    if (mazeName.isEmpty()) {
        mazeName = QString("Maze %1").arg(savedMazes_.size() + 1);
    }

    if (algorithm == GenerationAlgorithm::Tessellation) {
        const int size = tessSizeCombo_->currentData().toInt();
        widthUnits = size;
        heightUnits = size;
        depthUnits = 1;
    }

    game_.generate(algorithm, widthUnits, heightUnits, depthUnits);
    auto gridCopy = game_.grid();
    for (auto& layer : gridCopy) {
        for (auto& row : layer) {
            for (auto& cell : row) {
                if (cell == 2 || cell == 3) {
                    cell = 0;
                }
            }
        }
    }

    savedMazes_.push_back(SavedMaze{
        mazeName.toStdString(),
        algorithm,
        widthUnits,
        heightUnits,
        depthUnits,
        std::move(gridCopy),
        game_.start(),
        game_.goal()
    });

    const int row = static_cast<int>(savedMazes_.size()) - 1;
    savedList_->addItem(QString("%1 — %2 (%3x%4x%5)")
        .arg(mazeName.isEmpty() ? QString("Maze %1").arg(row + 1) : mazeName)
        .arg(algorithmLabel(algorithm))
        .arg(widthUnits)
        .arg(heightUnits)
        .arg(depthUnits));
    savedList_->setCurrentRow(row);

    rightStack_->setCurrentWidget(playPage_);
    refreshActions();
    updateStatus();
    mazeWidget_->clearFocus();
    nameEdit_->clear();
}

void MazeWindow::handleMove(const Direction direction) {
    if (!game_.hasMaze() || savedList_->currentRow() < 0) {
        return;
    }

    if (game_.move(direction)) {
        mazeWidget_->setMaze(game_.layerGrid(currentLayer_), game_.solved());
        updateStatus();
    }
}

void MazeWindow::updateStatus() {
    QString mazeName = "—";
    if (!game_.hasMaze()) {
        if (savedList_->count() == 0) {
            statusLabel_->setText("No saved mazes yet. Click New Maze to create one.");
        } else {
            statusLabel_->setText("Select a maze and right-click Solve to play.");
        }
        homeButton_->setVisible(false);
        coordLabel_->setText("Coordinates: N/A");
        return;
    }

    const int idx = loadedIndex_;
    if (idx >= 0 && idx < static_cast<int>(savedMazes_.size())) {
        mazeName = QString::fromStdString(savedMazes_[idx].name);
    }
    if (game_.solved()) {
        statusLabel_->setText("Exit reached! Pick another maze or return to home.");
        homeButton_->setVisible(true);
    } else {
        statusLabel_->setText("Move the blue marker to the yellow exit.");
        homeButton_->setVisible(false);
    }

    const auto [pl, pr, pc] = game_.player();
    const auto [gl, gr, gc] = game_.goal();
    coordLabel_->setText(QString("Layer %1 | Player: (%2, %3)    Exit: (%4, %5)")
                         .arg(pl + 1)
                         .arg(-pc)
                         .arg(pr)
                         .arg(-gc)
                         .arg(gr));
}

void MazeWindow::updateSizeControls() {
    const auto algorithm = static_cast<GenerationAlgorithm>(algorithmCombo_->currentData().toInt());
    const bool tessellation = algorithm == GenerationAlgorithm::Tessellation;

    widthSpin_->setEnabled(!tessellation);
    heightSpin_->setEnabled(!tessellation);
    depthSpin_->setEnabled(!tessellation);
    widthSpin_->setVisible(!tessellation);
    widthLabel_->setVisible(!tessellation);
    heightSpin_->setVisible(!tessellation);
    heightLabel_->setVisible(!tessellation);
    depthSpin_->setVisible(!tessellation);
    depthLabel_->setVisible(!tessellation);

    tessSizeCombo_->setVisible(tessellation);
    tessSizeLabel_->setVisible(tessellation);
}

void MazeWindow::showGenerator() {
    rightStack_->setCurrentWidget(generatePage_);
    mazeWidget_->clearFocus();
}

void MazeWindow::cancelGenerator() {
    rightStack_->setCurrentWidget(playPage_);
    mazeWidget_->setFocus();
}

void MazeWindow::handleSelectionChanged(const int /*row*/) {
    const int row = savedList_->currentRow();
    if (row >= 0) {
        loadMaze(row);
    } else {
        clearActiveMaze();
    }
    refreshActions();
}

void MazeWindow::deleteSelected() {
    const int row = savedList_->currentRow();
    if (row < 0 || row >= static_cast<int>(savedMazes_.size())) {
        return;
    }

    savedMazes_.erase(savedMazes_.begin() + row);
    delete savedList_->takeItem(row);

    // Update list labels to keep numbering tidy
    for (int i = 0; i < savedList_->count(); ++i) {
        auto* item = savedList_->item(i);
        if (!item) continue;
        item->setText(item->text().replace(QRegularExpression("^Maze \\d+"), QString("Maze %1").arg(i + 1)));
    }

    if (savedList_->count() > 0) {
        const int nextRow = std::min(row, savedList_->count() - 1);
        savedList_->setCurrentRow(nextRow);
    } else {
        clearActiveMaze();
    }

    refreshActions();
}

void MazeWindow::loadMaze(const int index) {
    if (index < 0 || index >= static_cast<int>(savedMazes_.size())) {
        clearActiveMaze();
        return;
    }

    const auto& saved = savedMazes_[index];
    game_.load(saved.grid, saved.start, saved.goal);
    currentLayer_ = std::get<0>(game_.player());
    layerSpin_->blockSignals(true);
    layerSpin_->setRange(1, static_cast<int>(saved.depth));
    layerSpin_->setValue(currentLayer_ + 1);
    layerSpin_->blockSignals(false);
    mazeWidget_->setMaze(game_.layerGrid(currentLayer_), game_.solved());
    loadedIndex_ = index;
    updateStatus();
    rightStack_->setCurrentWidget(playPage_);
    mazeWidget_->setFocus();
}

void MazeWindow::clearActiveMaze() {
    game_ = MazeGame{};
    mazeWidget_->setMaze({}, false);
    loadedIndex_ = -1;
    currentLayer_ = 0;
    layerSpin_->blockSignals(true);
    layerSpin_->setRange(1, 1);
    layerSpin_->setValue(1);
    layerSpin_->blockSignals(false);
    updateStatus();
    rightStack_->setCurrentWidget(playPage_);
}

void MazeWindow::refreshActions() {
    const bool hasSelection = savedList_->currentRow() >= 0;
    deleteButton_->setEnabled(hasSelection);
}

QString MazeWindow::algorithmLabel(const GenerationAlgorithm algorithm) const {
    switch (algorithm) {
        case GenerationAlgorithm::DFS: return "DFS";
        case GenerationAlgorithm::BFS: return "BFS";
        case GenerationAlgorithm::Wilson: return "Wilson";
        case GenerationAlgorithm::Kruskal: return "Kruskal";
        case GenerationAlgorithm::Prim: return "Prim";
        case GenerationAlgorithm::Tessellation: return "Tessellation";
    }
    return "Unknown";
}

void MazeWindow::closeEvent(QCloseEvent* event) {
    QMainWindow::closeEvent(event);
    if (event->isAccepted()) {
        QCoreApplication::quit();
    }
}

void MazeWindow::goHome() {
    savedList_->clearSelection();
    savedList_->setCurrentRow(-1);
    clearActiveMaze();
}

void MazeWindow::showContextMenu(const QPoint& pos) {
    const auto item = savedList_->itemAt(pos);
    if (!item) {
        return;
    }
    const int row = savedList_->row(item);
    savedList_->setCurrentRow(row);

    QMenu menu(this);
    QAction* solveAction = menu.addAction("Solve");
    connect(solveAction, &QAction::triggered, this, &MazeWindow::solveSelected);
    QAction* saveImageAction = menu.addAction("Save as Image...");
    connect(saveImageAction, &QAction::triggered, this, &MazeWindow::saveSelectedImage);
    menu.exec(savedList_->viewport()->mapToGlobal(pos));
}

void MazeWindow::solveSelected() {
    const int row = savedList_->currentRow();
    if (row < 0 || row >= static_cast<int>(savedMazes_.size())) {
        return;
    }
    loadMaze(row);
}

void MazeWindow::saveSelectedImage() {
    const int row = savedList_->currentRow();
    if (row < 0 || row >= static_cast<int>(savedMazes_.size())) {
        return;
    }
    const auto& saved = savedMazes_[row];
    const int layer = std::clamp(layerSpin_->value() - 1, 0, saved.depth - 1);

    auto grid = saved.grid.at(layer);
    const auto [sl, sr, sc] = saved.start;
    const auto [gl, gr, gc] = saved.goal;
    if (layer == sl && sr >= 0 && sc >= 0 && sr < static_cast<int>(grid.size()) && sc < static_cast<int>(grid.front().size())) {
        grid[sr][sc] = 2;
    }
    if (layer == gl && gr >= 0 && gc >= 0 && gr < static_cast<int>(grid.size()) && gc < static_cast<int>(grid.front().size())) {
        grid[gr][gc] = 3;
    }

    const int rows = static_cast<int>(grid.size());
    const int cols = rows > 0 ? static_cast<int>(grid.front().size()) : 0;
    if (rows == 0 || cols == 0) {
        return;
    }

    const int cell = 10;
    QImage image(cols * cell, rows * cell, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    auto colorFor = [](unsigned char v) -> QColor {
        switch (v) {
            case 1: return QColor(30, 30, 30);
            case 2: return QColor(245, 245, 245);
            case 3: return QColor(245, 245, 245);
            case 4: return QColor(180, 55, 55);
            case 5: return QColor(245, 245, 245);
            case 6: return QColor(245, 245, 245);
            default: return QColor(245, 245, 245);
        }
    };

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            painter.fillRect(c * cell, r * cell, cell, cell, colorFor(grid[r][c]));
            if (grid[r][c] == 5) { // down
                painter.setPen(Qt::black);
                painter.setBrush(Qt::black);
                painter.drawEllipse(QPointF(c * cell + cell * 0.5, r * cell + cell * 0.5), cell * 0.2, cell * 0.2);
            } else if (grid[r][c] == 6) { // up
                painter.setPen(QPen(Qt::black, std::max(1.0, cell * 0.08)));
                const double pad = cell * 0.2;
                painter.drawLine(QPointF(c * cell + pad, r * cell + pad), QPointF((c + 1) * cell - pad, (r + 1) * cell - pad));
                painter.drawLine(QPointF(c * cell + pad, (r + 1) * cell - pad), QPointF((c + 1) * cell - pad, r * cell + pad));
            }
        }
    }

    const QString suggested = QString::fromStdString(saved.name).isEmpty()
        ? QString("maze.png")
        : QString::fromStdString(saved.name) + ".png";
    const QString path = QFileDialog::getSaveFileName(this, "Save Maze Image", suggested, "PNG Images (*.png)");
    if (path.isEmpty()) {
        return;
    }

    if (!image.save(path, "PNG")) {
        QMessageBox::warning(this, "Save Failed", "Could not save the maze image.");
    }
}

void MazeWindow::changeLayer(const int layerVal) {
    if (!game_.hasMaze()) {
        return;
    }
    const int target = layerVal - 1;
    if (target < 0 || target >= game_.layers()) {
        return;
    }
    currentLayer_ = target;
    mazeWidget_->setMaze(game_.layerGrid(currentLayer_), game_.solved());
    updateStatus();
}
