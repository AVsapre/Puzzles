#include "MazeWindow.h"

#include <algorithm>
#include <cctype>
#include <QAbstractItemView>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QSize>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextEdit>
#include <QScrollArea>
#include <QPalette>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStackedWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QPrintDialog>
#include <QPrinter>
#include <QFontDatabase>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QDialog>
#include <QStatusBar>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include "puzzles/maze_graph.h"

namespace {
constexpr int kMinUnits = 2;
constexpr int kMaxUnits = 1000;

QJsonObject graphToJson(const MazeGraph& graph) {
    QJsonObject obj;
    obj["rows"] = graph.rows;
    obj["cols"] = graph.cols;
    obj["entranceNode"] = graph.entranceNode;
    obj["exitNode"] = graph.exitNode;

    QJsonArray nodes;
    for (const auto& n : graph.nodes) {
        QJsonObject nodeObj;
        nodeObj["id"] = n.id;
        nodeObj["row"] = n.row;
        nodeObj["col"] = n.col;
        nodes.append(nodeObj);
    }
    obj["nodes"] = nodes;

    QJsonArray edges;
    for (const auto& e : graph.edges) {
        QJsonObject edgeObj;
        edgeObj["from"] = e.from;
        edgeObj["to"] = e.to;
        edgeObj["open"] = e.open;
        edges.append(edgeObj);
    }
    obj["edges"] = edges;
    return obj;
}

std::optional<MazeGraph> graphFromJson(const QJsonObject& obj) {
    MazeGraph graph;
    graph.rows = obj.value("rows").toInt();
    graph.cols = obj.value("cols").toInt();
    graph.entranceNode = obj.value("entranceNode").toInt(-1);
    graph.exitNode = obj.value("exitNode").toInt(-1);

    const auto nodesArr = obj.value("nodes").toArray();
    const auto edgesArr = obj.value("edges").toArray();
    if (graph.rows <= 0 || graph.cols <= 0 || nodesArr.isEmpty() || edgesArr.isEmpty()) {
        return std::nullopt;
    }

    graph.nodes.reserve(nodesArr.size());
    for (const auto& val : nodesArr) {
        const auto nodeObj = val.toObject();
        MazeNode node;
        node.id = nodeObj.value("id").toInt();
        node.row = nodeObj.value("row").toInt();
        node.col = nodeObj.value("col").toInt();
        graph.nodes.push_back(node);
    }

    graph.edges.reserve(edgesArr.size());
    for (const auto& val : edgesArr) {
        const auto edgeObj = val.toObject();
        MazeEdge edge;
        edge.from = edgeObj.value("from").toInt();
        edge.to = edgeObj.value("to").toInt();
        edge.open = edgeObj.value("open").toBool();
        graph.edges.push_back(edge);
    }

    return graph;
}

QJsonArray intGridToJson(const std::vector<std::vector<int>>& grid) {
    QJsonArray rows;
    for (const auto& row : grid) {
        QJsonArray jsonRow;
        for (int cell : row) {
            jsonRow.append(cell);
        }
        rows.append(jsonRow);
    }
    return rows;
}

std::vector<std::vector<int>> intGridFromJson(const QJsonArray& arr) {
    std::vector<std::vector<int>> grid;
    grid.reserve(arr.size());
    for (const auto& rowVal : arr) {
        if (!rowVal.isArray()) {
            grid.clear();
            return {};
        }
        const auto rowArray = rowVal.toArray();
        std::vector<int> row;
        row.reserve(rowArray.size());
        for (const auto& cellVal : rowArray) {
            row.push_back(cellVal.toInt());
        }
        grid.push_back(std::move(row));
    }
    return grid;
}

int algorithmToInt(const GenerationAlgorithm algorithm) {
    return static_cast<int>(algorithm);
}

GenerationAlgorithm algorithmFromInt(const int value) {
    switch (value) {
        case static_cast<int>(GenerationAlgorithm::DFS): return GenerationAlgorithm::DFS;
        case static_cast<int>(GenerationAlgorithm::BFS): return GenerationAlgorithm::BFS;
        case static_cast<int>(GenerationAlgorithm::Wilson): return GenerationAlgorithm::Wilson;
        case static_cast<int>(GenerationAlgorithm::Kruskal): return GenerationAlgorithm::Kruskal;
        case static_cast<int>(GenerationAlgorithm::Prim): return GenerationAlgorithm::Prim;
        case static_cast<int>(GenerationAlgorithm::Tessellation): return GenerationAlgorithm::Tessellation;
        default: return GenerationAlgorithm::DFS;
    }
}

void showSizedMessage(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text) {
    QMessageBox box(icon, title, text, QMessageBox::Ok, parent);
    box.setModal(true);
    box.setSizeGripEnabled(false);
    const QSize hint = box.sizeHint();
    const int width = std::max(360, hint.width());
    const int height = std::max(200, hint.height());
    box.setFixedSize(width, height);
    box.exec();
}

QColor pickColorWithHex(QWidget* parent, const QColor& current, const QString& title) {
    QDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setModal(true);
    dlg.setSizeGripEnabled(false);
    dlg.setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    const QList<QColor> palette = {
        QColor("#000000"), QColor("#434343"), QColor("#666666"), QColor("#999999"), QColor("#b7b7b7"), QColor("#cccccc"), QColor("#efefef"), QColor("#ffffff"),
        QColor("#fe0000"), QColor("#ff9900"), QColor("#ffff00"), QColor("#00ff00"), QColor("#00ffff"), QColor("#4a86e8"), QColor("#0000ff"), QColor("#9900ff"),
        QColor("#e6007e"), QColor("#ff5b31"), QColor("#ffcb6b"), QColor("#b6d7a8"), QColor("#a2c4c9"), QColor("#9fc5e8"), QColor("#6fa8dc"), QColor("#8e7cc3")
    };

    auto* paletteLabel = new QLabel("Basic colors", &dlg);
    layout->addWidget(paletteLabel);
    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(4);
    grid->setVerticalSpacing(4);
    int rowIdx = 0;
    int colIdx = 0;
    QColor currentColor = current.isValid() ? current : palette.front();
    std::vector<QPushButton*> paletteButtons;
    paletteButtons.reserve(palette.size());
    int selectedIdx = -1;
    bool customSelected = false;

    const auto updateSelectionStyle = [&](QPushButton* btn, const QColor& color, bool selected) {
        btn->setStyleSheet(QString("background:%1; border: %2px solid %3;")
            .arg(color.name(QColor::HexRgb))
            .arg(selected ? 2 : 1)
            .arg(selected ? "#2a82da" : "#444"));
    };

    QPushButton* customSwatch = nullptr;
    QColor customColor = currentColor;

    for (int i = 0; i < palette.size(); ++i) {
        auto* swatchBtn = new QPushButton(&dlg);
        swatchBtn->setFixedSize(28, 20);
        swatchBtn->setCursor(Qt::PointingHandCursor);
        paletteButtons.push_back(swatchBtn);
        if (palette[i].name(QColor::HexRgb) == currentColor.name(QColor::HexRgb)) {
            selectedIdx = i;
        }
        grid->addWidget(swatchBtn, rowIdx, colIdx);
        QObject::connect(swatchBtn, &QPushButton::clicked, &dlg, [&, i]() {
            currentColor = palette[i];
            selectedIdx = i;
            customSelected = false;
            for (int j = 0; j < static_cast<int>(paletteButtons.size()); ++j) {
                updateSelectionStyle(paletteButtons[j], palette[j], j == selectedIdx);
            }
            if (customSwatch) updateSelectionStyle(customSwatch, customColor, customSelected);
        });
        if (++colIdx >= 8) {
            colIdx = 0;
            ++rowIdx;
        }
    }
    layout->addLayout(grid);

    auto* customRow = new QHBoxLayout();
    customRow->setSpacing(6);
    auto* customLabel = new QLabel("Custom:", &dlg);
    customRow->addWidget(customLabel, 0);
    customSwatch = new QPushButton(&dlg);
    customSwatch->setFixedSize(32, 22);
    customSwatch->setCursor(Qt::PointingHandCursor);
    customRow->addWidget(customSwatch, 0);
    customRow->addStretch(1);
    layout->addLayout(customRow);

    const auto refreshSwatches = [&]() {
        for (int j = 0; j < static_cast<int>(paletteButtons.size()); ++j) {
            updateSelectionStyle(paletteButtons[j], palette[j], j == selectedIdx && !customSelected);
        }
        updateSelectionStyle(customSwatch, customColor, customSelected);
    };

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    layout->addWidget(buttons);

    QObject::connect(customSwatch, &QPushButton::clicked, &dlg, [&]() {
        const QColor picked = QColorDialog::getColor(currentColor, &dlg, "Custom Color", QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
        if (picked.isValid()) {
            customColor = picked;
            currentColor = picked;
            customSelected = true;
            selectedIdx = -1;
            refreshSwatches();
        }
    });

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
        dlg.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    dlg.adjustSize();
    dlg.setFixedSize(dlg.sizeHint());
    if (dlg.exec() == QDialog::Accepted) {
        return currentColor;
    }
    return QColor();
}
}

MazeWindow::MazeWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Puzzles");
    resize(1160, 780);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    savedList_ = new QListWidget(this);
    savedList_->setSelectionMode(QAbstractItemView::SingleSelection);
    savedList_->setMaximumWidth(200);
    connect(savedList_, &QListWidget::currentRowChanged, this, &MazeWindow::handleSelectionChanged);
    savedList_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(savedList_, &QListWidget::customContextMenuRequested, this, &MazeWindow::showContextMenu);

    auto* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("Saved Puzzles", this));
    leftLayout->addWidget(savedList_, 1);
    leftLayout->setContentsMargins(8, 8, 8, 8);
    leftLayout->setSpacing(8);

    rightStack_ = new QStackedWidget(this);

    playPage_ = new QWidget(this);
    mazeWidget_ = new MazeWidget(this);
    mazeWidget_->setColors(mazeWallColor_, mazeBackgroundColor_);
    mazeWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mazeScroll_ = new QScrollArea(this);
    mazeScroll_->setWidgetResizable(true);
    mazeScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mazeScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mazeScroll_->setStyleSheet("QScrollArea { background-color: white; }");
    mazeScroll_->setWidget(mazeWidget_);
    
    mazeScroll_->viewport()->installEventFilter(this);
    mazeScroll_->installEventFilter(this);

    crosswordWidget_ = new CrosswordWidget(this);
    crosswordWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    crosswordScroll_ = new QScrollArea(this);
    crosswordScroll_->setWidgetResizable(true);
    crosswordScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    crosswordScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    crosswordScroll_->setStyleSheet("QScrollArea { background-color: white; }");
    crosswordScroll_->setWidget(crosswordWidget_);
    crosswordWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(crosswordWidget_, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        if (!currentCrossword_) return;
        QMenu menu(this);
        QAction* exportAction = menu.addAction("Export Crossword Image...");
        connect(exportAction, &QAction::triggered, this, &MazeWindow::saveCrosswordImage);
        menu.exec(crosswordWidget_->mapToGlobal(pos));
    });

    wordSearchWidget_ = new WordSearchWidget(this);
    wordSearchWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    wordSearchScroll_ = new QScrollArea(this);
    wordSearchScroll_->setWidgetResizable(true);
    wordSearchScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wordSearchScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    wordSearchScroll_->setStyleSheet("QScrollArea { background-color: white; }");
    wordSearchScroll_->setWidget(wordSearchWidget_);

    sudokuWidget_ = new SudokuWidget(this);
    sudokuWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sudokuScroll_ = new QScrollArea(this);
    sudokuScroll_->setWidgetResizable(true);
    sudokuScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sudokuScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sudokuScroll_->setStyleSheet("QScrollArea { background-color: white; }");
    sudokuScroll_->setWidget(sudokuWidget_);
    
    cryptogramWidget_ = new CryptogramWidget(this);
    cryptogramWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cryptogramScroll_ = new QScrollArea(this);
    cryptogramScroll_->setWidgetResizable(true);
    cryptogramScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    cryptogramScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    cryptogramScroll_->setStyleSheet("QScrollArea { background-color: white; }");
    cryptogramScroll_->setWidget(cryptogramWidget_);

    statusLabel_ = new QLabel("No saved puzzles yet. Click New Puzzle to create one.", this);
    statusLabel_->setWordWrap(true);
    statusLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    coordStatusLabel_ = new QLabel(this);
    coordStatusLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    coordLabel_ = new QLabel("Coordinates: N/A", this);
    coordLabel_->setVisible(false);
    coordLabel_->setAlignment(Qt::AlignCenter);
    coordLabel_->setVisible(false);

    mazeView_ = new QWidget(this);
    auto* mazeLayout = new QVBoxLayout;
    mazeLayout->addWidget(mazeScroll_, 1);
    mazeLayout->setContentsMargins(0, 0, 0, 0);
    mazeView_->setLayout(mazeLayout);

    crosswordView_ = new QWidget(this);
    acrossLabel_ = new QLabel(this);
    downLabel_ = new QLabel(this);
    acrossLabel_->setTextFormat(Qt::RichText);
    downLabel_->setTextFormat(Qt::RichText);
    acrossLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    downLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    auto* cluesLayout = new QHBoxLayout;
    auto* acrossBox = new QVBoxLayout;
    auto* acrossHeader = new QLabel("ACROSS", this);
    acrossHeader->setAlignment(Qt::AlignLeft);
    acrossBox->addWidget(acrossHeader);
    acrossBox->addWidget(acrossLabel_);
    auto* downBox = new QVBoxLayout;
    auto* downHeader = new QLabel("DOWN", this);
    downHeader->setAlignment(Qt::AlignLeft);
    downBox->addWidget(downHeader);
    downBox->addWidget(downLabel_);
    cluesLayout->addLayout(acrossBox, 1);
    cluesLayout->addLayout(downBox, 1);

    auto* crosswordLayout = new QVBoxLayout;
    crosswordLayout->addWidget(crosswordScroll_, 1);
    crosswordLayout->addLayout(cluesLayout);
    crosswordView_->setLayout(crosswordLayout);

    wordSearchView_ = new QWidget(this);
    wordsLabel_ = new QLabel(this);
    wordsLabel_->setTextFormat(Qt::RichText);
    wordsLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    wordsLabel_->setWordWrap(true);
    auto* wordSearchLayout = new QVBoxLayout;
    wordSearchLayout->addWidget(wordSearchScroll_, 1);
    wordSearchLayout->addWidget(new QLabel("Find these words:", this));
    wordSearchLayout->addWidget(wordsLabel_);
    wordSearchView_->setLayout(wordSearchLayout);

    sudokuView_ = new QWidget(this);
    auto* sudokuLayout = new QVBoxLayout;
    sudokuLayout->addWidget(sudokuScroll_, 1);
    sudokuView_->setLayout(sudokuLayout);
    
    cryptogramView_ = new QWidget(this);
    auto* cryptogramLayout = new QVBoxLayout;
    cryptogramLayout->addWidget(cryptogramScroll_, 1);
    cryptogramView_->setLayout(cryptogramLayout);

    puzzleViewStack_ = new QStackedWidget(this);
    puzzleViewStack_->addWidget(mazeView_);
    puzzleViewStack_->addWidget(crosswordView_);
    puzzleViewStack_->addWidget(wordSearchView_);
    puzzleViewStack_->addWidget(sudokuView_);
    puzzleViewStack_->addWidget(cryptogramView_);
    puzzleViewStack_->setCurrentWidget(mazeView_);

    auto* playLayout = new QVBoxLayout;
    playLayout->addWidget(puzzleViewStack_, 1);
    playLayout->setContentsMargins(8, 8, 8, 8);
    playLayout->setSpacing(8);
    playPage_->setLayout(playLayout);

    generatePage_ = new QWidget(this);
    puzzleTypeCombo_ = new QComboBox(this);
    puzzleTypeCombo_->addItem("Maze", QStringLiteral("maze"));
    puzzleTypeCombo_->addItem("Crossword", QStringLiteral("crossword"));
    puzzleTypeCombo_->addItem("Word Search", QStringLiteral("wordsearch"));
    puzzleTypeCombo_->addItem("Sudoku", QStringLiteral("sudoku"));
    puzzleTypeCombo_->addItem("Cryptogram", QStringLiteral("cryptogram"));
    connect(puzzleTypeCombo_, &QComboBox::currentIndexChanged, this, [this](int) { updateGeneratorView(); });

    mazeGeneratorPage_ = new QWidget(this);
    algorithmCombo_ = new QComboBox(this);
    algorithmCombo_->addItem("Depth-first search", static_cast<int>(GenerationAlgorithm::DFS));
    algorithmCombo_->addItem("Breadth-first search", static_cast<int>(GenerationAlgorithm::BFS));
    algorithmCombo_->addItem("Wilson's algorithm", static_cast<int>(GenerationAlgorithm::Wilson));
    algorithmCombo_->addItem("Kruskal's algorithm", static_cast<int>(GenerationAlgorithm::Kruskal));
    algorithmCombo_->addItem("Prim's algorithm", static_cast<int>(GenerationAlgorithm::Prim));
    algorithmCombo_->addItem("Tessellation", static_cast<int>(GenerationAlgorithm::Tessellation));

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText("Puzzle name (optional)");
    nameEdit_->setToolTip("Optional name to identify the puzzle.");

    widthSpin_ = new QSpinBox(this);
    widthSpin_->setRange(kMinUnits, kMaxUnits);
    widthSpin_->setValue(12);
    widthSpin_->setToolTip("Maze width in units (cells are 2x+1).");

    heightSpin_ = new QSpinBox(this);
    heightSpin_->setRange(kMinUnits, kMaxUnits);
    heightSpin_->setValue(12);
    heightSpin_->setToolTip("Maze height in units (cells are 2y+1).");

    startModeCombo_ = new QComboBox(this);
    startModeCombo_->addItem("Random", QStringLiteral("random"));
    startModeCombo_->addItem("Center", QStringLiteral("center"));
    startModeCombo_->addItem("Edge", QStringLiteral("edge"));
    startModeCombo_->addItem("Custom", QStringLiteral("custom"));
    startModeCombo_->setToolTip("Choose where the maze should start.");

    startColSpin_ = new QSpinBox(this);
    startColSpin_->setRange(1, kMaxUnits);
    startColSpin_->setValue(1);
    startColSpin_->setToolTip("Optional: starting column (1-based unit).");

    startRowSpin_ = new QSpinBox(this);
    startRowSpin_->setRange(1, kMaxUnits);
    startRowSpin_->setValue(1);
    startRowSpin_->setToolTip("Optional: starting row (1-based unit).");

    exitModeCombo_ = new QComboBox(this);
    exitModeCombo_->addItem("Default", QStringLiteral("default"));
    exitModeCombo_->addItem("Custom", QStringLiteral("custom"));
    exitModeCombo_->setToolTip("Choose where the maze should exit.");

    exitColSpin_ = new QSpinBox(this);
    exitColSpin_->setRange(1, kMaxUnits);
    exitColSpin_->setValue(1);
    exitColSpin_->setToolTip("Exit column (must be on an edge).");

    exitRowSpin_ = new QSpinBox(this);
    exitRowSpin_->setRange(1, kMaxUnits);
    exitRowSpin_->setValue(1);
    exitRowSpin_->setToolTip("Exit row (must be on an edge).");

    const auto toggleStart = [this](const QString& mode) {
        const bool custom = mode == "custom";
        if (startColSpin_) startColSpin_->setEnabled(custom);
        if (startRowSpin_) startRowSpin_->setEnabled(custom);
        if (startColLabel_) startColLabel_->setEnabled(custom);
        if (startRowLabel_) startRowLabel_->setEnabled(custom);
    };
    const auto warnUnsupportedStart = [this]() {
        if (!startModeCombo_ || !algorithmCombo_) {
            return;
        }
        const auto algorithm = static_cast<GenerationAlgorithm>(algorithmCombo_->currentData().toInt());
        const QString mode = startModeCombo_->currentData().toString();
        const bool ignoresCustom = algorithm == GenerationAlgorithm::Kruskal || algorithm == GenerationAlgorithm::Tessellation;
        if (mode == "custom" && ignoresCustom) {
            showSizedMessage(this, QMessageBox::Information,
                             "Start position may be ignored",
                             "This algorithm uses a randomized start position; the custom start will be ignored.");
        }
    };
    toggleStart(startModeCombo_->currentData().toString());
    connect(startModeCombo_, &QComboBox::currentTextChanged, this, [this, toggleStart, warnUnsupportedStart](const QString&){
        toggleStart(startModeCombo_->currentData().toString());
        warnUnsupportedStart();
    });

    const auto toggleExit = [this](const QString& mode) {
        const bool custom = mode == "custom";
        if (exitColSpin_) exitColSpin_->setEnabled(custom);
        if (exitRowSpin_) exitRowSpin_->setEnabled(custom);
        if (exitColLabel_) exitColLabel_->setEnabled(custom);
        if (exitRowLabel_) exitRowLabel_->setEnabled(custom);
    };
    toggleExit(exitModeCombo_->currentData().toString());
    connect(exitModeCombo_, &QComboBox::currentTextChanged, this, [this, toggleExit](const QString&) {
        toggleExit(exitModeCombo_->currentData().toString());
    });

    tessSizeCombo_ = new QComboBox(this);
    for (int size : {2, 4, 8, 16, 32, 64}) {
        tessSizeCombo_->addItem(QString::number(size), size);
    }
    tessSizeCombo_->setCurrentIndex(2);

    auto* mazeFormLayout = new QFormLayout;
    mazeFormLayout->addRow("Name", nameEdit_);
    mazeFormLayout->addRow("Algorithm", algorithmCombo_);
    widthLabel_ = new QLabel("Width (units)", this);
    heightLabel_ = new QLabel("Height (units)", this);
    startColLabel_ = new QLabel("Start Column (1-based)", this);
    startRowLabel_ = new QLabel("Start Row (1-based)", this);
    exitModeLabel_ = new QLabel("Exit Position", this);
    exitColLabel_ = new QLabel("Exit Column (edge only)", this);
    exitRowLabel_ = new QLabel("Exit Row (edge only)", this);
    tessSizeLabel_ = new QLabel("Size (units)", this);
    mazeFormLayout->addRow(widthLabel_, widthSpin_);
    mazeFormLayout->addRow(heightLabel_, heightSpin_);
    mazeFormLayout->addRow("Start Position", startModeCombo_);
    mazeFormLayout->addRow(startColLabel_, startColSpin_);
    mazeFormLayout->addRow(startRowLabel_, startRowSpin_);
    mazeFormLayout->addRow(exitModeLabel_, exitModeCombo_);
    mazeFormLayout->addRow(exitColLabel_, exitColSpin_);
    mazeFormLayout->addRow(exitRowLabel_, exitRowSpin_);
    mazeFormLayout->addRow(tessSizeLabel_, tessSizeCombo_);
    mazeGeneratorPage_->setLayout(mazeFormLayout);

    crosswordGeneratorPage_ = new QWidget(this);
    wordListContainer_ = new QWidget(this);
    wordListLayout_ = new QVBoxLayout;
    wordListLayout_->setContentsMargins(0, 0, 0, 0);
    wordListLayout_->setSpacing(6);
    wordListContainer_->setLayout(wordListLayout_);
    addWordRow();

    addWordButton_ = new QPushButton("Add Word", this);
    connect(addWordButton_, &QPushButton::clicked, this, [this]() { addWordRow(); });

    auto* crosswordForm = new QFormLayout;
    crosswordForm->addRow("Words", wordListContainer_);
    crosswordForm->addRow("", addWordButton_);
    crosswordGeneratorPage_->setLayout(crosswordForm);

    wordSearchGeneratorPage_ = new QWidget(this);
    wordSearchSizeSpin_ = new QSpinBox(this);
    wordSearchSizeSpin_->setRange(8, 30);
    wordSearchSizeSpin_->setValue(15);
    
    wordCountSpin_ = new QSpinBox(this);
    wordCountSpin_->setRange(5, 50);
    wordCountSpin_->setValue(10);
    
    auto* wordSearchForm = new QFormLayout;
    wordSearchForm->addRow("Grid Size", wordSearchSizeSpin_);
    wordSearchForm->addRow("Number of Words", wordCountSpin_);
    wordSearchGeneratorPage_->setLayout(wordSearchForm);

    sudokuGeneratorPage_ = new QWidget(this);
    auto* sudokuForm = new QFormLayout;
    sudokuNameEdit_ = new QLineEdit(this);
    sudokuNameEdit_->setPlaceholderText("Puzzle name (optional)");
    sudokuDifficultyCombo_ = new QComboBox(this);
    sudokuDifficultyCombo_->addItem("Easy", 0);
    sudokuDifficultyCombo_->addItem("Medium", 1);
    sudokuDifficultyCombo_->addItem("Hard", 2);
    sudokuForm->addRow("Name", sudokuNameEdit_);
    sudokuForm->addRow("Difficulty", sudokuDifficultyCombo_);
    sudokuGeneratorPage_->setLayout(sudokuForm);
    
    cryptogramGeneratorPage_ = new QWidget(this);
    auto* cryptogramForm = new QFormLayout;
    cryptogramNameEdit_ = new QLineEdit(this);
    cryptogramNameEdit_->setPlaceholderText("Puzzle name (optional)");
    cryptogramPlaintextEdit_ = new QTextEdit(this);
    cryptogramPlaintextEdit_->setPlaceholderText("Enter plaintext for the cryptogram.\nSpaces and punctuation are preserved.");
    cryptogramPlaintextEdit_->setMinimumHeight(120);
    cryptogramNoSelfMapCheck_ = new QCheckBox("Avoid mapping a letter to itself", this);
    cryptogramNoSelfMapCheck_->setChecked(true);
    cryptogramHintSpin_ = new QSpinBox(this);
    cryptogramHintSpin_->setRange(0, 10);
    cryptogramHintSpin_->setValue(2);
    cryptogramForm->addRow("Name", cryptogramNameEdit_);
    cryptogramForm->addRow("Plaintext", cryptogramPlaintextEdit_);
    cryptogramForm->addRow("Hints (letters revealed)", cryptogramHintSpin_);
    cryptogramForm->addRow("", cryptogramNoSelfMapCheck_);
    cryptogramGeneratorPage_->setLayout(cryptogramForm);

    generatorStack_ = new QStackedWidget(this);
    generatorStack_->addWidget(mazeGeneratorPage_);
    generatorStack_->addWidget(crosswordGeneratorPage_);
    generatorStack_->addWidget(wordSearchGeneratorPage_);
    generatorStack_->addWidget(sudokuGeneratorPage_);
    generatorStack_->addWidget(cryptogramGeneratorPage_);
    generatorStack_->setCurrentWidget(mazeGeneratorPage_);

    auto* createButton = new QPushButton("Create Puzzle", this);
    connect(createButton, &QPushButton::clicked, this, &MazeWindow::generateMaze);
    auto* cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &MazeWindow::cancelGenerator);

    auto* topRow = new QHBoxLayout;
    topRow->addWidget(new QLabel("Type", this));
    topRow->addWidget(puzzleTypeCombo_, 1);

    auto* generateLayout = new QVBoxLayout;
    generateLayout->addLayout(topRow);
    generateLayout->addWidget(generatorStack_);
    generatorErrorLabel_ = new QLabel(this);
    generatorErrorLabel_->setStyleSheet("color: #c0392b;"); 
    generatorErrorLabel_->setWordWrap(true);
    generatorErrorLabel_->setVisible(false);
    generateLayout->addWidget(generatorErrorLabel_);
    generateLayout->addWidget(createButton);
    generateLayout->addWidget(cancelButton);
    generateLayout->addStretch(1);
    generatePage_->setLayout(generateLayout);

    rightStack_->addWidget(playPage_);
    rightStack_->setCurrentWidget(playPage_);

    auto* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(rightStack_, 1);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    central->setLayout(mainLayout);

    connect(mazeWidget_, &MazeWidget::moveRequested, this, &MazeWindow::handleMove);
    connect(algorithmCombo_, &QComboBox::currentIndexChanged, this, [this, warnUnsupportedStart](int) {
        updateSizeControls();
        warnUnsupportedStart();
    });

    if (!wordSearchGen_.loadDictionary("words.txt")) {
        updateStatusBarText("Warning: Could not load word dictionary. Word search may not work properly.");
    }

    createMenusAndToolbars();
    statusBar()->addWidget(statusLabel_, 1);
    statusBar()->addPermanentWidget(coordStatusLabel_);

    updateGeneratorView();
    updateSizeControls();
    restoreState();
    updateStatus();
    refreshActions();
    mazeWidget_->setFocus();
    
    setTabOrder(puzzleTypeCombo_, algorithmCombo_);
    setTabOrder(algorithmCombo_, nameEdit_);
    setTabOrder(nameEdit_, widthSpin_);
    setTabOrder(widthSpin_, heightSpin_);
    setTabOrder(heightSpin_, startModeCombo_);
    setTabOrder(startModeCombo_, startColSpin_);
    setTabOrder(startColSpin_, startRowSpin_);
    setTabOrder(startRowSpin_, exitModeCombo_);
    setTabOrder(exitModeCombo_, exitColSpin_);
    setTabOrder(exitColSpin_, exitRowSpin_);
    setTabOrder(exitRowSpin_, tessSizeCombo_);
}

void MazeWindow::generateMaze() {
    const QString type = puzzleTypeCombo_->currentData().toString();
    if (type == "crossword") {
        generateCrossword();
        return;
    }
    if (type == "wordsearch") {
        generateWordSearch();
        return;
    }
    if (type == "sudoku") {
        generateSudoku();
        return;
    }
    if (type == "cryptogram") {
        generateCryptogram();
        return;
    }

    auto showError = [&](const QString& msg) {
        if (generatorErrorLabel_) {
            generatorErrorLabel_->clear();
            generatorErrorLabel_->setVisible(false);
        }
        showSizedMessage(this, QMessageBox::Warning, "Invalid Input", msg);
    };

    if (algorithmCombo_->currentIndex() < 0) {
        return;
    }

    activeMode_ = ActiveMode::Maze;
    puzzleViewStack_->setCurrentWidget(mazeView_);
    coordLabel_->setVisible(true);
    crosswordWidget_->clear();
    acrossLabel_->clear();
    downLabel_->clear();

    const auto algorithm = static_cast<GenerationAlgorithm>(algorithmCombo_->currentData().toInt());
    int widthUnits = widthSpin_->value();
    int heightUnits = heightSpin_->value();
    std::pair<int, int> startUnits{-1, -1};
    std::pair<int, int> exitUnits{-1, -1};
    const QString startMode = startModeCombo_ ? startModeCombo_->currentData().toString() : QStringLiteral("center");
    const QString exitMode = exitModeCombo_ ? exitModeCombo_->currentData().toString() : QStringLiteral("default");
    if (startMode == "custom") {
        startUnits = {startColSpin_->value(), startRowSpin_->value()};
        if (startUnits.first < 1 || startUnits.second < 1 || startUnits.first > widthUnits || startUnits.second > heightUnits) {
            showError(QString("Start must be within 1..%1 (cols) and 1..%2 (rows).").arg(widthUnits).arg(heightUnits));
            return;
        }
    } else if (startMode == "center") {
        startUnits = { (widthUnits + 1) / 2, (heightUnits + 1) / 2 };
    } else { 
        startUnits = {-1, -1};
    }

    const bool useCustomExit = exitMode == "custom";
    if (useCustomExit) {
        exitUnits = {exitColSpin_->value(), exitRowSpin_->value()};
        const bool onEdge = exitUnits.first == 1 || exitUnits.first == widthUnits ||
                            exitUnits.second == 1 || exitUnits.second == heightUnits;
        if (!onEdge) {
            showError("Exit must be on an edge (row=1/height or col=1/width).");
            return;
        }
    } else {
        exitUnits = {-1, -1};
    }

    QString mazeName = nameEdit_->text().trimmed();
    if (mazeName.isEmpty()) {
        mazeName = QString("Maze %1").arg(savedPuzzles_.size() + 1);
    }

    if (algorithm == GenerationAlgorithm::Tessellation) {
        const int size = tessSizeCombo_->currentData().toInt();
        widthUnits = size;
        heightUnits = size;
        startUnits = {-1, -1};
        if (startModeCombo_) startModeCombo_->setCurrentIndex(0);
        if (exitModeCombo_) exitModeCombo_->setCurrentIndex(0);
    }

    if (!game_.generate(algorithm, widthUnits, heightUnits, startUnits, exitUnits, useCustomExit)) {
        showError("Invalid exit position; ensure it is on the edge and connected to a path.");
        return;
    }
    const auto& graph = game_.graph();
    SavedMaze savedMaze{
        mazeName.toStdString(),
        algorithm,
        graph.cols,
        graph.rows,
        graph,
        game_.entranceNode(),
        game_.exitNode(),
        game_.playerNode()
    };
    savedMazes_.push_back(savedMaze);
    
    SavedPuzzle puzzle;
    puzzle.type = SavedPuzzle::Type::Maze;
    puzzle.maze = savedMazes_.back();
    savedPuzzles_.push_back(puzzle);

    const int row = static_cast<int>(savedPuzzles_.size()) - 1;
    savedList_->addItem(QString("%1 — %2 (%3x%4)")
        .arg(mazeName.isEmpty() ? QString("Maze %1").arg(row + 1) : mazeName)
        .arg(algorithmLabel(algorithm))
        .arg(savedMaze.width)
        .arg(savedMaze.height));
    savedList_->blockSignals(true);
    savedList_->setCurrentRow(row);
    savedList_->blockSignals(false);

    handleSelectionChanged(row);

    rightStack_->setCurrentWidget(playPage_);
    refreshActions();
    updateStatus();
    mazeWidget_->clearFocus();
    nameEdit_->clear();
    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }
    persistState();
    if (generatorDialog_) {
        generatorDialog_->accept();
    }
}

void MazeWindow::generateCrossword() {
    const auto entries = collectWords();
    if (entries.empty()) {
        showSizedMessage(this, QMessageBox::Warning, "Invalid Input", "Add at least one word with a hint.");
        return;
    }

    std::vector<std::string> words;
    words.reserve(entries.size());
    lastCrosswordHints_.clear();
    for (const auto& [word, hint] : entries) {
        if (word.empty() || hint.empty()) {
            showSizedMessage(this, QMessageBox::Warning, "Invalid Input", "Each word needs a hint.");
            return;
        }
        words.push_back(word);
        lastCrosswordHints_[word] = hint;
    }

    constexpr int rows = 15;
    constexpr int cols = 15;
    const auto puzzle = ::generateCrossword(words, rows, cols);
    if (!puzzle) {
        showSizedMessage(this, QMessageBox::Warning, "Generation Failed", "Could not create a crossword with the given words.");
        return;
    }

    activeMode_ = ActiveMode::Crossword;
    game_ = MazeGame{};
    loadedIndex_ = -1;
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    coordLabel_->setVisible(false);
    currentCrossword_ = puzzle;
    
    SavedPuzzle savedPuzzle;
    savedPuzzle.type = SavedPuzzle::Type::Crossword;
    savedPuzzle.crossword = SavedCrossword{
        QString("Crossword %1").arg(savedPuzzles_.size() + 1).toStdString(),
        *puzzle,
        lastCrosswordHints_
    };
    savedPuzzles_.push_back(savedPuzzle);
    
    const int row = static_cast<int>(savedPuzzles_.size()) - 1;
    savedList_->addItem(QString("Crossword %1 — %2 words")
        .arg(row + 1)
        .arg(words.size()));
    savedList_->blockSignals(true);
    savedList_->setCurrentRow(row);
    savedList_->blockSignals(false);
    
    puzzleViewStack_->setCurrentWidget(crosswordView_);
    crosswordWidget_->setPuzzle(*puzzle);
    showCrossword(*puzzle);
    rightStack_->setCurrentWidget(playPage_);
    refreshActions();
    updateStatus();
    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }
    persistState();
    if (generatorDialog_) {
        generatorDialog_->accept();
    }
}

void MazeWindow::showCrossword(const CrosswordPuzzle& puzzle) {
    auto formatList = [this](const std::vector<CrosswordEntry>& list) {
        if (list.empty()) {
            return QString("—");
        }
        QString text;
        for (const auto& entry : list) {
            text.append(QString::number(entry.number));
            text.append(". ");
            const auto it = lastCrosswordHints_.find(entry.word);
            if (it != lastCrosswordHints_.end() && !it->second.empty()) {
                text.append(QString::fromStdString(it->second));
            } else {
                text.append("—");
            }
            text.append("<br>");
        }
        return text;
    };

    acrossLabel_->setText(formatList(puzzle.across));
    downLabel_->setText(formatList(puzzle.down));
}

void MazeWindow::generateWordSearch() {
    const int size = wordSearchSizeSpin_->value();
    const int wordCount = wordCountSpin_->value();
    
    WordSearchPuzzle puzzle = wordSearchGen_.generate(size, wordCount);
    
    if (puzzle.words.empty()) {
        if (generatorErrorLabel_) {
            generatorErrorLabel_->clear();
            generatorErrorLabel_->setVisible(false);
        }
        showSizedMessage(this, QMessageBox::Warning, "Generation Failed", "Failed to generate word search. Dictionary may not be loaded.");
        return;
    }
    
    activeMode_ = ActiveMode::WordSearch;
    puzzleViewStack_->setCurrentWidget(wordSearchView_);
    coordLabel_->setVisible(false);
    currentWordSearch_ = puzzle;
    
    SavedPuzzle saved;
    saved.type = SavedPuzzle::Type::WordSearch;
    saved.wordSearch = SavedWordSearch{
        "Word Search " + std::to_string(savedPuzzles_.size() + 1),
        puzzle
    };
    savedPuzzles_.push_back(saved);
    
    savedList_->addItem(QString::fromStdString(saved.wordSearch->name));
    savedList_->setCurrentRow(static_cast<int>(savedPuzzles_.size()) - 1);
    
    showWordSearch(puzzle);
    rightStack_->setCurrentWidget(playPage_);
    refreshActions();
    updateStatus();
    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }
    persistState();
    if (generatorDialog_) {
        generatorDialog_->accept();
    }
}

void MazeWindow::showWordSearch(const WordSearchPuzzle& puzzle) {
    QString wordListText;
    for (size_t i = 0; i < puzzle.words.size(); ++i) {
        wordListText.append(QString::fromStdString(puzzle.words[i]));
        if (i < puzzle.words.size() - 1) {
            wordListText.append(", ");
        }
    }
    wordsLabel_->setText(wordListText);
    wordSearchWidget_->setPuzzle(puzzle);
}

void MazeWindow::generateSudoku() {
    const int difficulty = sudokuDifficultyCombo_
        ? sudokuDifficultyCombo_->currentData().toInt()
        : 0;
    const auto puzzle = ::generateSudoku(difficulty);
    if (!puzzle) {
        showSizedMessage(this, QMessageBox::Warning, "Invalid Input", "Could not create a Sudoku puzzle.");
        return;
    }

    activeMode_ = ActiveMode::Sudoku;
    game_ = MazeGame{};
    loadedIndex_ = -1;
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    coordLabel_->setVisible(false);
    currentSudoku_ = puzzle;
    QString sudokuName = sudokuNameEdit_ ? sudokuNameEdit_->text().trimmed() : QString();
    const int nextIndex = static_cast<int>(savedPuzzles_.size()) + 1;
    if (sudokuName.isEmpty()) {
        sudokuName = QString("Sudoku %1").arg(nextIndex);
    }
    
    SavedPuzzle savedPuzzle;
    savedPuzzle.type = SavedPuzzle::Type::Sudoku;
    savedPuzzle.sudoku = SavedSudoku{
        sudokuName.toStdString(),
        *puzzle,
        difficulty
    };
    savedPuzzles_.push_back(savedPuzzle);
    
    const int row = static_cast<int>(savedPuzzles_.size()) - 1;
    savedList_->addItem(QString("%1 — %2")
        .arg(sudokuName)
        .arg(sudokuDifficultyLabel(difficulty)));
    savedList_->blockSignals(true);
    savedList_->setCurrentRow(row);
    savedList_->blockSignals(false);
    
    puzzleViewStack_->setCurrentWidget(sudokuView_);
    sudokuWidget_->setPuzzle(*puzzle);
    showSudoku(*puzzle);
    rightStack_->setCurrentWidget(playPage_);
    refreshActions();
    updateStatus();
    if (sudokuNameEdit_) {
        sudokuNameEdit_->clear();
    }
    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }
    persistState();
    if (generatorDialog_) {
        generatorDialog_->accept();
    }
}

void MazeWindow::generateCryptogram() {
    QString name = cryptogramNameEdit_ ? cryptogramNameEdit_->text().trimmed() : QString();
    if (name.isEmpty()) {
        name = QString("Cryptogram %1").arg(savedPuzzles_.size() + 1);
    }
    
    if (!cryptogramPlaintextEdit_) {
        return;
    }
    const QString rawPlain = cryptogramPlaintextEdit_->toPlainText();
    if (rawPlain.trimmed().isEmpty()) {
        showSizedMessage(this, QMessageBox::Warning, "Invalid Input", "Enter plaintext for the cryptogram.");
        return;
    }
    
    const bool avoidSelf = cryptogramNoSelfMapCheck_ && cryptogramNoSelfMapCheck_->isChecked();
    const int hintCount = cryptogramHintSpin_ ? cryptogramHintSpin_->value() : 0;
    
    CryptogramPuzzle puzzle = cryptogramGen_.generate(rawPlain.toStdString(), avoidSelf, hintCount);
    
    activeMode_ = ActiveMode::Cryptogram;
    game_ = MazeGame{};
    loadedIndex_ = -1;
    coordLabel_->setVisible(false);
    currentCryptogram_ = puzzle;
    
    SavedPuzzle savedPuzzle;
    savedPuzzle.type = SavedPuzzle::Type::Cryptogram;
    savedPuzzle.cryptogram = SavedCryptogram{
        name.toStdString(),
        puzzle,
        avoidSelf,
        hintCount
    };
    savedPuzzles_.push_back(savedPuzzle);
    savedCryptograms_.push_back(*savedPuzzle.cryptogram);
    
    const int row = static_cast<int>(savedPuzzles_.size()) - 1;
    savedList_->addItem(QString("%1 — %2 chars").arg(name).arg(rawPlain.trimmed().length()));
    savedList_->blockSignals(true);
    savedList_->setCurrentRow(row);
    savedList_->blockSignals(false);
    
    puzzleViewStack_->setCurrentWidget(cryptogramView_);
    cryptogramWidget_->setPuzzle(puzzle);
    cryptogramWidget_->setTestMode(true);
    rightStack_->setCurrentWidget(playPage_);
    refreshActions();
    updateStatus();
    if (cryptogramNameEdit_) cryptogramNameEdit_->clear();
    cryptogramPlaintextEdit_->clear();
    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }
    persistState();
    if (generatorDialog_) {
        generatorDialog_->accept();
    }
}

void MazeWindow::showSudoku(const SudokuPuzzle& puzzle) {
    sudokuWidget_->setPuzzle(puzzle);
}

void MazeWindow::showCryptogram(const CryptogramPuzzle& puzzle) {
    cryptogramWidget_->setPuzzle(puzzle);
    cryptogramWidget_->setTestMode(activeMode_ == ActiveMode::Cryptogram);
}

void MazeWindow::saveSudokuImage() {
    if (!currentSudoku_) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Nothing to export.");
        return;
    }

    const auto& puzzle = *currentSudoku_;
    const int rows = static_cast<int>(puzzle.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle.grid.front().size());
    if (rows == 0 || cols == 0) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Nothing to export.");
        return;
    }

    QString sudokuName = "Sudoku";
    int difficulty = 0;
    const int row = savedList_->currentRow();
    if (row >= 0 && row < static_cast<int>(savedPuzzles_.size())) {
        const auto& saved = savedPuzzles_[row];
        if (saved.type == SavedPuzzle::Type::Sudoku && saved.sudoku) {
            sudokuName = QString::fromStdString(saved.sudoku->name);
            difficulty = saved.sudoku->difficulty;
        }
    }
    if (sudokuName.trimmed().isEmpty()) {
        sudokuName = "Sudoku";
    }
    const QString difficultyLabel = sudokuDifficultyLabel(difficulty);
    const QString title = difficultyLabel.isEmpty()
        ? sudokuName
        : QString("%1 — %2").arg(sudokuName).arg(difficultyLabel);

    const int cell = 48;
    const int gridWidth = cols * cell;
    const int gridHeight = rows * cell;
    const int padding = 24;

    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    QFont numberFont;
    numberFont.setPointSize(20);
    numberFont.setBold(true);

    QFontMetrics titleMetrics(titleFont);
    const int headerHeight = title.isEmpty() ? 0 : titleMetrics.height() + 8;

    const int imageWidth = gridWidth + padding * 2;
    const int imageHeight = gridHeight + padding * 2 + headerHeight;

    QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    if (!title.isEmpty()) {
        painter.setFont(titleFont);
        painter.setPen(Qt::black);
        painter.drawText(QRect(padding, padding / 2, imageWidth - padding * 2, headerHeight),
                         Qt::AlignLeft | Qt::AlignVCenter, title);
    }

    const int offsetX = padding;
    const int offsetY = padding + headerHeight;

    painter.setFont(numberFont);
    const QColor blockShade(245, 245, 245);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const QRect tile(offsetX + c * cell, offsetY + r * cell, cell, cell);
            if (((r / 3) + (c / 3)) % 2 == 0) {
                painter.fillRect(tile, blockShade);
            } else {
                painter.fillRect(tile, Qt::white);
            }

            const int value = puzzle.grid[r][c];
            if (value != 0) {
                painter.setPen(Qt::black);
                painter.drawText(tile, Qt::AlignCenter, QString::number(value));
            }
        }
    }

    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i <= rows; ++i) {
        painter.drawLine(offsetX, offsetY + i * cell, offsetX + gridWidth, offsetY + i * cell);
    }
    for (int i = 0; i <= cols; ++i) {
        painter.drawLine(offsetX + i * cell, offsetY, offsetX + i * cell, offsetY + gridHeight);
    }

    painter.setPen(QPen(Qt::black, 3));
    for (int i = 0; i <= rows; i += 3) {
        painter.drawLine(offsetX, offsetY + i * cell, offsetX + gridWidth, offsetY + i * cell);
    }
    for (int i = 0; i <= cols; i += 3) {
        painter.drawLine(offsetX + i * cell, offsetY, offsetX + i * cell, offsetY + gridHeight);
    }

    const QString suggested = sudokuName.isEmpty()
        ? QString("sudoku.png")
        : sudokuName + ".png";
    const QString path = QFileDialog::getSaveFileName(this, "Save Sudoku Image", suggested, "PNG Images (*.png)");
    if (path.isEmpty()) {
        return;
    }
    if (!image.save(path, "PNG")) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Could not save the sudoku image.");
    }
}

void MazeWindow::saveCryptogramImage() {
    if (!currentCryptogram_) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Nothing to export.");
        return;
    }
    const auto& puzzle = *currentCryptogram_;
    QString cryptoName = "cryptogram";
    const int row = savedList_->currentRow();
    if (row >= 0 && row < static_cast<int>(savedPuzzles_.size())) {
        const auto& saved = savedPuzzles_[row];
        if (saved.type == SavedPuzzle::Type::Cryptogram && saved.cryptogram) {
            cryptoName = QString::fromStdString(saved.cryptogram->name);
        }
    }
    
    const QString suggested = cryptoName.isEmpty()
        ? QString("cryptogram.png")
        : cryptoName + ".png";
    const QString path = QFileDialog::getSaveFileName(this, "Save Cryptogram Image", suggested, "PNG Images (*.png)");
    if (path.isEmpty()) {
        return;
    }
    
    const int padding = 24;
    const int contentWidth = 900;
    const int cellSize = 32;
    const QSize measured = CryptogramWidget::renderPuzzle(nullptr, puzzle, nullptr, nullptr, nullptr, -1, false, contentWidth, padding, cellSize);
    QImage image(measured.width(), measured.height(), QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    CryptogramWidget::renderPuzzle(&painter, puzzle, nullptr, nullptr, nullptr, -1, false, contentWidth, padding, cellSize);
    
    if (!image.save(path, "PNG")) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Could not save the cryptogram image.");
    }
}

void MazeWindow::handleMove(const Direction direction) {
    if (activeMode_ != ActiveMode::Maze || !game_.hasMaze()) {
        return;
    }

    const auto playerBefore = game_.playerCell();
    const int prevRow = playerBefore.first;
    const int prevCol = playerBefore.second;
    if (game_.move(direction)) {
        const auto [r, c] = game_.playerCell();
        mazeWidget_->startMove(prevRow, prevCol, r, c);
        updateStatus();
    }
}

void MazeWindow::updateStatus() {
    if (activeMode_ == ActiveMode::Crossword) {
        updateStatusBarText("Crossword ready. Print or test on paper.");
        return;
    }
    if (activeMode_ == ActiveMode::Sudoku) {
        updateStatusBarText("Sudoku ready. Print or test on paper.");
        return;
    }
    if (activeMode_ == ActiveMode::WordSearch) {
        updateStatusBarText("Word search ready. Print or test on paper.");
        coordLabel_->setVisible(false);
        return;
    }
    if (activeMode_ == ActiveMode::Cryptogram) {
        updateStatusBarText("Cryptogram ready. Decode the substitution.");
        coordLabel_->setVisible(false);
        return;
    }

    QString mazeName = "—";
    if (!game_.hasMaze() || activeMode_ != ActiveMode::Maze) {
    updateStatusBarText(savedList_->count() == 0
        ? "No saved puzzles yet. Click New Puzzle to create one."
        : "Select a puzzle and use Play/Restart to test.");
        coordLabel_->setVisible(false);
        return;
    }
    coordLabel_->setVisible(true);

    const int idx = loadedIndex_;
    if (idx >= 0 && idx < static_cast<int>(savedPuzzles_.size()) && savedPuzzles_[idx].maze) {
        mazeName = QString::fromStdString(savedPuzzles_[idx].maze->name);
    }
    updateStatusBarText("");

    const auto [pr, pc] = game_.playerCell();
    const auto [gr, gc] = game_.exitCell();
    const int totalRows = game_.rows();
    const int displayPr = totalRows > 0 ? totalRows - 1 - pr : pr;
    const int displayGr = totalRows > 0 ? totalRows - 1 - gr : gr;
    const QString coords = QString("Location: (%1, %2)    Exit: (%3, %4)")
                         .arg(pc)
                         .arg(displayPr)
                         .arg(gc)
                         .arg(displayGr);
    coordLabel_->setText(coords);
    coordStatusLabel_->setText(coords);
}

void MazeWindow::updateSizeControls() {
    if (puzzleTypeCombo_->currentData().toString() == "crossword") {
        return;
    }
    const auto algorithm = static_cast<GenerationAlgorithm>(algorithmCombo_->currentData().toInt());
    const bool tessellation = algorithm == GenerationAlgorithm::Tessellation;
    const bool supportsStartExit = true;

    widthSpin_->setEnabled(!tessellation);
    heightSpin_->setEnabled(!tessellation);
    widthSpin_->setVisible(!tessellation);
    widthLabel_->setVisible(!tessellation);
    heightSpin_->setVisible(!tessellation);
    heightLabel_->setVisible(!tessellation);
    if (startModeCombo_) {
        startModeCombo_->setVisible(supportsStartExit);
        startModeCombo_->setEnabled(supportsStartExit);
    }
    if (exitModeCombo_) {
        exitModeCombo_->setVisible(supportsStartExit);
        exitModeCombo_->setEnabled(supportsStartExit);
    }

    const bool startCustom = supportsStartExit && startModeCombo_ && startModeCombo_->currentData().toString() == "custom";
    if (startColSpin_) startColSpin_->setEnabled(startCustom);
    if (startRowSpin_) startRowSpin_->setEnabled(startCustom);
    if (startColSpin_) startColSpin_->setVisible(supportsStartExit);
    if (startRowSpin_) startRowSpin_->setVisible(supportsStartExit);
    if (startColLabel_) startColLabel_->setVisible(supportsStartExit);
    if (startRowLabel_) startRowLabel_->setVisible(supportsStartExit);

    const bool exitCustom = supportsStartExit && exitModeCombo_ && exitModeCombo_->currentData().toString() == "custom";
    if (exitColSpin_) exitColSpin_->setEnabled(exitCustom);
    if (exitRowSpin_) exitRowSpin_->setEnabled(exitCustom);
    if (exitColSpin_) exitColSpin_->setVisible(supportsStartExit);
    if (exitRowSpin_) exitRowSpin_->setVisible(supportsStartExit);
    if (exitColLabel_) exitColLabel_->setVisible(supportsStartExit);
    if (exitRowLabel_) exitRowLabel_->setVisible(supportsStartExit);
    if (exitModeLabel_) exitModeLabel_->setVisible(supportsStartExit);

    tessSizeCombo_->setVisible(tessellation);
    tessSizeLabel_->setVisible(tessellation);
}

void MazeWindow::updateGeneratorView() {
    const QString type = puzzleTypeCombo_->currentData().toString();
    if (type == "crossword") {
        generatorStack_->setCurrentWidget(crosswordGeneratorPage_);
    } else if (type == "wordsearch") {
        generatorStack_->setCurrentWidget(wordSearchGeneratorPage_);
    } else if (type == "sudoku") {
        generatorStack_->setCurrentWidget(sudokuGeneratorPage_);
    } else if (type == "cryptogram") {
        generatorStack_->setCurrentWidget(cryptogramGeneratorPage_);
    } else {
        generatorStack_->setCurrentWidget(mazeGeneratorPage_);
        updateSizeControls();
    }
}

void MazeWindow::addWordRow(const QString& word, const QString& hint) {
    if (!wordListLayout_) return;

    auto* rowWidget = new QWidget(crosswordGeneratorPage_);
    auto* rowLayout = new QHBoxLayout;
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);
    auto* wordEdit = new QLineEdit(rowWidget);
    wordEdit->setPlaceholderText("Word");
    wordEdit->setText(word);
    auto* hintEdit = new QLineEdit(rowWidget);
    hintEdit->setPlaceholderText("Hint");
    hintEdit->setText(hint);
    auto* removeBtn = new QPushButton("Remove", rowWidget);
    connect(removeBtn, &QPushButton::clicked, this, [this, rowWidget]() { removeWordRow(rowWidget); });

    rowLayout->addWidget(wordEdit, 2);
    rowLayout->addWidget(hintEdit, 3);
    rowLayout->addWidget(removeBtn, 0);
    rowWidget->setLayout(rowLayout);

    wordListLayout_->addWidget(rowWidget);
    wordRows_.push_back(WordRow{rowWidget, wordEdit, hintEdit, removeBtn});

    const bool enableRemoval = wordRows_.size() > 1;
    for (auto& row : wordRows_) {
        row.remove->setEnabled(enableRemoval);
    }
}

void MazeWindow::removeWordRow(QWidget* rowWidget) {
    if (!rowWidget || wordRows_.size() <= 1) {
        return;
    }
    auto it = std::find_if(wordRows_.begin(), wordRows_.end(), [rowWidget](const WordRow& row) {
        return row.widget == rowWidget;
    });
    if (it == wordRows_.end()) {
        return;
    }

    wordListLayout_->removeWidget(it->widget);
    it->widget->deleteLater();
    wordRows_.erase(it);

    const bool enableRemoval = wordRows_.size() > 1;
    for (auto& row : wordRows_) {
        row.remove->setEnabled(enableRemoval);
    }
}

std::string MazeWindow::cleanWord(const QString& raw) {
    std::string out;
    const auto utf8 = raw.toUpper().toUtf8();
    for (char ch : utf8) {
        if (std::isalpha(static_cast<unsigned char>(ch))) {
            out.push_back(ch);
        }
    }
    return out;
}

std::vector<std::pair<std::string, std::string>> MazeWindow::collectWords() const {
    std::vector<std::pair<std::string, std::string>> entries;
    for (const auto& row : wordRows_) {
        if (!row.word) continue;
        const auto cleaned = cleanWord(row.word->text());
        if (cleaned.empty()) continue;
        const QString hint = row.hint ? row.hint->text().trimmed() : QString();
        entries.emplace_back(cleaned, hint.toStdString());
    }
    return entries;
}

void MazeWindow::showGenerator() {
    if (!generatorDialog_) {
        generatorDialog_ = new QDialog(this);
        generatorDialog_->setWindowTitle("New Puzzle");
        generatorDialog_->setModal(true);
        generatorDialog_->setSizeGripEnabled(false);
        generatorDialog_->setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);
        auto* layout = new QVBoxLayout(generatorDialog_);
        layout->setContentsMargins(12, 12, 12, 12);
        generatePage_->setParent(generatorDialog_);
        layout->addWidget(generatePage_);
    }

    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }

    generatorDialog_->adjustSize();
    generatorDialog_->setFixedSize(generatorDialog_->sizeHint());
    generatorDialog_->show();
    generatorDialog_->raise();
    generatorDialog_->activateWindow();
}

void MazeWindow::cancelGenerator() {
    if (generatorDialog_) {
        generatorDialog_->reject();
    }
    mazeWidget_->setFocus();
    if (generatorErrorLabel_) {
        generatorErrorLabel_->clear();
        generatorErrorLabel_->setVisible(false);
    }
}

void MazeWindow::handleSelectionChanged(const int ) {
    saveActiveProgress();
    
    const int row = savedList_->currentRow();
    refreshActions();

    if (row >= 0 && row < static_cast<int>(savedPuzzles_.size())) {
        const auto& puzzle = savedPuzzles_[row];
        
        if (puzzle.type == SavedPuzzle::Type::Maze && puzzle.maze) {
            const auto& saved = *puzzle.maze;
            puzzleViewStack_->setCurrentWidget(mazeView_);
            coordLabel_->setVisible(false);
            activeMode_ = ActiveMode::None;
            mazeWidget_->setGraph(saved.graph, false, false, saved.entranceNode, saved.exitNode, saved.playerNode);
            updateStatusBarText("Right-click a puzzle in the list and choose Test to start.");
            crosswordWidget_->clear();
        } else if (puzzle.type == SavedPuzzle::Type::Crossword && puzzle.crossword) {
            const auto& saved = *puzzle.crossword;
            activeMode_ = ActiveMode::None;
            mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
            coordLabel_->setVisible(false);
            puzzleViewStack_->setCurrentWidget(crosswordView_);
            crosswordWidget_->setPuzzle(saved.puzzle);
            lastCrosswordHints_ = saved.hints;
            showCrossword(saved.puzzle);
            updateStatusBarText("Right-click to export crossword image.");
        } else if (puzzle.type == SavedPuzzle::Type::WordSearch && puzzle.wordSearch) {
            const auto& saved = *puzzle.wordSearch;
            activeMode_ = ActiveMode::None;
            mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
            coordLabel_->setVisible(false);
            puzzleViewStack_->setCurrentWidget(wordSearchView_);
            wordSearchWidget_->setPuzzle(saved.puzzle);
            currentWordSearch_ = saved.puzzle;
            showWordSearch(saved.puzzle);
            updateStatusBarText("Word search puzzle ready.");
        } else if (puzzle.type == SavedPuzzle::Type::Sudoku && puzzle.sudoku) {
            const auto& saved = *puzzle.sudoku;
            activeMode_ = ActiveMode::None;
            mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
            coordLabel_->setVisible(false);
            puzzleViewStack_->setCurrentWidget(sudokuView_);
            sudokuWidget_->setPuzzle(saved.puzzle);
            currentSudoku_ = saved.puzzle;
            showSudoku(saved.puzzle);
            updateStatusBarText("Sudoku puzzle ready.");
        } else if (puzzle.type == SavedPuzzle::Type::Cryptogram && puzzle.cryptogram) {
            const auto& saved = *puzzle.cryptogram;
            activeMode_ = ActiveMode::None;
            mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
            coordLabel_->setVisible(false);
            puzzleViewStack_->setCurrentWidget(cryptogramView_);
            cryptogramWidget_->setPuzzle(saved.puzzle);
            cryptogramWidget_->setTestMode(false);
            currentCryptogram_ = saved.puzzle;
            updateStatusBarText("Right-click to test the cryptogram.");
        }
    } else {
        mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
        coordLabel_->setVisible(false);
        updateStatusBarText(savedList_->count() == 0
            ? "No saved puzzles yet. Click New Puzzle to create one."
            : "Right-click a puzzle in the list and choose Test to start.");
    }
    coordStatusLabel_->setText(coordLabel_->isVisible() ? coordLabel_->text() : QString());
    persistState();
}

void MazeWindow::deleteSelected() {
    const int row = savedList_->currentRow();
    if (row < 0 || row >= static_cast<int>(savedPuzzles_.size())) {
        return;
    }
    
    const bool wasInTestMode = inTestMode_;
    const int deletedIndex = loadedIndex_;

    if (savedPuzzles_[row].type == SavedPuzzle::Type::Maze && savedPuzzles_[row].maze) {
        auto it = std::find_if(savedMazes_.begin(), savedMazes_.end(), 
            [&](const SavedMaze& m) { return m.name == savedPuzzles_[row].maze->name; });
        if (it != savedMazes_.end()) {
            savedMazes_.erase(it);
        }
    } else if (savedPuzzles_[row].type == SavedPuzzle::Type::Sudoku) {
        
    } else if (savedPuzzles_[row].type == SavedPuzzle::Type::Cryptogram && savedPuzzles_[row].cryptogram) {
        auto it = std::find_if(savedCryptograms_.begin(), savedCryptograms_.end(),
            [&](const SavedCryptogram& c) { return c.name == savedPuzzles_[row].cryptogram->name; });
        if (it != savedCryptograms_.end()) {
            savedCryptograms_.erase(it);
        }
    }
    
    savedPuzzles_.erase(savedPuzzles_.begin() + row);
    delete savedList_->takeItem(row);
    
    if (wasInTestMode && deletedIndex == row) {
        clearActivePuzzle();
    }

    if (savedList_->count() > 0) {
        const int nextRow = std::min(row, savedList_->count() - 1);
        savedList_->setCurrentRow(nextRow);
    } else {
        clearActivePuzzle();
    }

    refreshActions();
    persistState();
}

void MazeWindow::loadMaze(const int index) {
    if (index < 0 || index >= static_cast<int>(savedPuzzles_.size())) {
        clearActivePuzzle();
        return;
    }

    const auto& puzzle = savedPuzzles_[index];
    if (puzzle.type != SavedPuzzle::Type::Maze || !puzzle.maze) {
        return;
    }

    inTestMode_ = true;
    activeMode_ = ActiveMode::Maze;
    const auto& saved = *puzzle.maze;
    game_.load(saved.graph, saved.entranceNode, saved.exitNode, saved.playerNode);
    mazeWidget_->setGraph(game_.graph(), game_.tested(), true, game_.entranceNode(), game_.exitNode(), game_.playerNode());
    puzzleViewStack_->setCurrentWidget(mazeView_);
    coordLabel_->setVisible(true);
    crosswordWidget_->clear();
    crosswordWidget_->setTestMode(false);
    acrossLabel_->clear();
    downLabel_->clear();
    lastCrosswordHints_.clear();
    currentCrossword_.reset();
    loadedIndex_ = index;
    updateStatus();
    refreshActions();
    rightStack_->setCurrentWidget(playPage_);
    mazeWidget_->setFocus();
    persistState();
}

void MazeWindow::loadCrossword(const int index) {
    if (index < 0 || index >= static_cast<int>(savedPuzzles_.size())) {
        clearActivePuzzle();
        return;
    }

    const auto& puzzle = savedPuzzles_[index];
    if (puzzle.type != SavedPuzzle::Type::Crossword || !puzzle.crossword) {
        return;
    }

    inTestMode_ = true;
    activeMode_ = ActiveMode::Crossword;
    const auto& saved = *puzzle.crossword;
    game_ = MazeGame{};
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    coordLabel_->setVisible(false);
    currentCrossword_ = saved.puzzle;
    lastCrosswordHints_ = saved.hints;
    puzzleViewStack_->setCurrentWidget(crosswordView_);
    crosswordWidget_->setPuzzle(saved.puzzle);
    crosswordWidget_->setTestMode(true);
    showCrossword(saved.puzzle);
    loadedIndex_ = index;
    updateStatus();
    refreshActions();
    rightStack_->setCurrentWidget(playPage_);
    crosswordWidget_->setFocus();
    persistState();
}

void MazeWindow::loadWordSearch(const int index) {
    if (index < 0 || index >= static_cast<int>(savedPuzzles_.size())) {
        clearActivePuzzle();
        return;
    }

    const auto& puzzle = savedPuzzles_[index];
    if (puzzle.type != SavedPuzzle::Type::WordSearch || !puzzle.wordSearch) {
        return;
    }

    inTestMode_ = true;
    activeMode_ = ActiveMode::WordSearch;
    const auto& saved = *puzzle.wordSearch;
    game_ = MazeGame{};
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    coordLabel_->setVisible(false);
    currentWordSearch_ = saved.puzzle;
    puzzleViewStack_->setCurrentWidget(wordSearchView_);
    wordSearchWidget_->setPuzzle(saved.puzzle);
    wordSearchWidget_->setTestMode(true);
    showWordSearch(saved.puzzle);
    loadedIndex_ = index;
    updateStatus();
    refreshActions();
    rightStack_->setCurrentWidget(playPage_);
    wordSearchWidget_->setFocus();
    persistState();
}

void MazeWindow::loadSudoku(const int index) {
    if (index < 0 || index >= static_cast<int>(savedPuzzles_.size())) {
        clearActivePuzzle();
        return;
    }

    const auto& puzzle = savedPuzzles_[index];
    if (puzzle.type != SavedPuzzle::Type::Sudoku || !puzzle.sudoku) {
        return;
    }

    inTestMode_ = true;
    activeMode_ = ActiveMode::Sudoku;
    const auto& saved = *puzzle.sudoku;
    game_ = MazeGame{};
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    coordLabel_->setVisible(false);
    currentSudoku_ = saved.puzzle;
    puzzleViewStack_->setCurrentWidget(sudokuView_);
    sudokuWidget_->setPuzzle(saved.puzzle);
    sudokuWidget_->setTestMode(true);
    showSudoku(saved.puzzle);
    loadedIndex_ = index;
    updateStatus();
    refreshActions();
    rightStack_->setCurrentWidget(playPage_);
    sudokuWidget_->setFocus();
    persistState();
}

void MazeWindow::loadCryptogram(const int index) {
    if (index < 0 || index >= static_cast<int>(savedPuzzles_.size())) {
        clearActivePuzzle();
        return;
    }
    
    const auto& puzzle = savedPuzzles_[index];
    if (puzzle.type != SavedPuzzle::Type::Cryptogram || !puzzle.cryptogram) {
        return;
    }
    
    inTestMode_ = true;
    activeMode_ = ActiveMode::Cryptogram;
    game_ = MazeGame{};
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    coordLabel_->setVisible(false);
    currentCryptogram_ = puzzle.cryptogram->puzzle;
    currentCrossword_.reset();
    currentWordSearch_.reset();
    currentSudoku_.reset();
    puzzleViewStack_->setCurrentWidget(cryptogramView_);
    cryptogramWidget_->setPuzzle(puzzle.cryptogram->puzzle);
    cryptogramWidget_->setTestMode(true);
    loadedIndex_ = index;
    updateStatus();
    refreshActions();
    rightStack_->setCurrentWidget(playPage_);
    cryptogramWidget_->setFocus();
    persistState();
}

void MazeWindow::clearActivePuzzle() {
    inTestMode_ = false;
    savedList_->setEnabled(true);
    game_ = MazeGame{};
    mazeWidget_->setGraph(MazeGraph{}, false, false, -1, -1, -1);
    loadedIndex_ = -1;
    activeMode_ = ActiveMode::None;
    puzzleViewStack_->setCurrentWidget(mazeView_);
    crosswordWidget_->clear();
    crosswordWidget_->setTestMode(false);
    wordSearchWidget_->clear();
    wordSearchWidget_->setTestMode(false);
    sudokuWidget_->clear();
    sudokuWidget_->setTestMode(false);
    if (cryptogramWidget_) {
        cryptogramWidget_->clear();
        cryptogramWidget_->setTestMode(false);
    }
    acrossLabel_->clear();
    downLabel_->clear();
    wordsLabel_->clear();
    lastCrosswordHints_.clear();
    currentCrossword_.reset();
    currentWordSearch_.reset();
    currentSudoku_.reset();
    currentCryptogram_.reset();
    coordLabel_->setVisible(false);
    updateStatus();
    refreshActions();
    rightStack_->setCurrentWidget(playPage_);
    persistState();
}

void MazeWindow::refreshActions() {
    const bool canPlay = savedList_ && savedList_->currentRow() >= 0 && savedList_->currentRow() < savedList_->count();
    if (playAction_) playAction_->setEnabled(canPlay);
    if (endTestAction_) endTestAction_->setEnabled(inTestMode_);
    if (saveImageAction_) saveImageAction_->setEnabled(canPlay);
    updateActionStates();
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

QString MazeWindow::sudokuDifficultyLabel(const int difficulty) const {
    switch (difficulty) {
        case 0: return "Easy";
        case 1: return "Medium";
        case 2: return "Hard";
        default: return "Custom";
    }
}

void MazeWindow::createMenusAndToolbars() {
    auto* fileMenu = menuBar()->addMenu("&File");
    newAction_ = fileMenu->addAction("New Puzzle");
    newAction_->setShortcut(QKeySequence::New);
    connect(newAction_, &QAction::triggered, this, &MazeWindow::showGenerator);

    QAction* printAction = fileMenu->addAction("Print...");
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MazeWindow::printActivePuzzle);

    saveImageAction_ = fileMenu->addAction("Save as Image...");
    connect(saveImageAction_, &QAction::triggered, this, &MazeWindow::saveSelectedImage);

    fileMenu->addSeparator();
    exitAction_ = fileMenu->addAction("Exit");
    exitAction_->setShortcut(QKeySequence::Quit);
    connect(exitAction_, &QAction::triggered, this, &MazeWindow::goHome);
    connect(exitAction_, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto* viewMenu = menuBar()->addMenu("&View");
    playAction_ = viewMenu->addAction("Play/Restart");
    playAction_->setShortcut(Qt::Key_F5);
    connect(playAction_, &QAction::triggered, this, &MazeWindow::testSelected);

    endTestAction_ = viewMenu->addAction("End Test");
    connect(endTestAction_, &QAction::triggered, this, &MazeWindow::goHome);
    viewMenu->addSeparator();
    auto* lineColorAction = viewMenu->addAction("Maze Line Color...");
    connect(lineColorAction, &QAction::triggered, this, &MazeWindow::pickMazeWallColor);
    auto* backgroundColorAction = viewMenu->addAction("Maze Background Color...");
    connect(backgroundColorAction, &QAction::triggered, this, &MazeWindow::pickMazeBackgroundColor);
    auto* hexColorsAction = viewMenu->addAction("Set Maze Colors (Hex)...");
    connect(hexColorsAction, &QAction::triggered, this, &MazeWindow::pickMazeColorsHex);
    viewMenu->addSeparator();
    zoomInAction_ = viewMenu->addAction("Zoom In");
    zoomInAction_->setShortcuts({
        QKeySequence::ZoomIn,                                
        QKeySequence(Qt::CTRL | Qt::Key_Plus),               
        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal),  
        QKeySequence(Qt::CTRL | Qt::Key_Equal)               
    });
    zoomInAction_->setShortcutContext(Qt::ApplicationShortcut);
    connect(zoomInAction_, &QAction::triggered, this, [this]() { if (mazeWidget_) mazeWidget_->zoomIn(); });
    zoomOutAction_ = viewMenu->addAction("Zoom Out");
    QList<QKeySequence> zoomOutShortcuts;
    zoomOutShortcuts << QKeySequence(Qt::CTRL | Qt::Key_Minus);     
    zoomOutAction_->setShortcuts(zoomOutShortcuts);
    zoomOutAction_->setShortcutContext(Qt::ApplicationShortcut);
    connect(zoomOutAction_, &QAction::triggered, this, [this]() { if (mazeWidget_) mazeWidget_->zoomOut(); });
    zoomResetAction_ = viewMenu->addAction("Reset Zoom");
    zoomResetAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    zoomResetAction_->setShortcutContext(Qt::ApplicationShortcut);
    connect(zoomResetAction_, &QAction::triggered, this, [this]() { if (mazeWidget_) mazeWidget_->resetZoom(); });

    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About", this, [this] {
        showSizedMessage(this, QMessageBox::Information, "About Puzzles", "Puzzles — Qt edition");
    });

}

void MazeWindow::updateActionStates() {
    if (endTestAction_) endTestAction_->setEnabled(inTestMode_);
}

void MazeWindow::applyMazeColors() {
    if (mazeWidget_) {
        mazeWidget_->setColors(mazeWallColor_, mazeBackgroundColor_);
    }
}

bool MazeWindow::eventFilter(QObject* watched, QEvent* event) {
    if ((watched == mazeScroll_ || watched == (mazeScroll_ ? mazeScroll_->viewport() : nullptr)) && event->type() == QEvent::Wheel) {
        auto* wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers().testFlag(Qt::ControlModifier)) {
            if (mazeWidget_) {
                if (wheel->angleDelta().y() > 0) {
                    mazeWidget_->zoomIn();
                } else if (wheel->angleDelta().y() < 0) {
                    mazeWidget_->zoomOut();
                }
            }
            wheel->accept();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MazeWindow::pickMazeWallColor() {
    const QColor color = pickColorWithHex(this, mazeWallColor_, "Maze Line Color");
    if (!color.isValid()) {
        return;
    }
    mazeWallColor_ = color;
    applyMazeColors();
}

void MazeWindow::pickMazeBackgroundColor() {
    const QColor color = pickColorWithHex(this, mazeBackgroundColor_, "Maze Background Color");
    if (!color.isValid()) {
        return;
    }
    mazeBackgroundColor_ = color;
    applyMazeColors();
}

void MazeWindow::pickMazeColorsHex() {
    QDialog dlg(this);
    dlg.setWindowTitle("Set Maze Colors (Hex)");
    dlg.setModal(true);
    dlg.setSizeGripEnabled(false);
    dlg.setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* wallRow = new QHBoxLayout();
    wallRow->addWidget(new QLabel("Line color", &dlg));
    auto* wallEdit = new QLineEdit(mazeWallColor_.name(QColor::HexRgb), &dlg);
    wallEdit->setMaxLength(7);
    wallEdit->setPlaceholderText("#000000");
    wallEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^#?[0-9A-Fa-f]{6}$"), wallEdit));
    wallRow->addWidget(wallEdit, 1);
    auto* wallPickBtn = new QPushButton("Pick...", &dlg);
    wallRow->addWidget(wallPickBtn);
    layout->addLayout(wallRow);

    auto* bgRow = new QHBoxLayout();
    bgRow->addWidget(new QLabel("Background", &dlg));
    auto* bgEdit = new QLineEdit(mazeBackgroundColor_.name(QColor::HexRgb), &dlg);
    bgEdit->setMaxLength(7);
    bgEdit->setPlaceholderText("#ffffff");
    bgEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^#?[0-9A-Fa-f]{6}$"), bgEdit));
    bgRow->addWidget(bgEdit, 1);
    auto* bgPickBtn = new QPushButton("Pick...", &dlg);
    bgRow->addWidget(bgPickBtn);
    layout->addLayout(bgRow);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    layout->addWidget(buttons);

    auto parseColor = [](QLineEdit* edit, const QColor& fallback) -> QColor {
        QColor c(edit->text().startsWith('#') ? edit->text() : ("#" + edit->text()));
        return c.isValid() ? c : fallback;
    };

    QObject::connect(wallPickBtn, &QPushButton::clicked, &dlg, [this, wallEdit, &dlg]() {
        const QColor c = pickColorWithHex(&dlg, mazeWallColor_, "Maze Line Color");
        if (c.isValid()) wallEdit->setText(c.name(QColor::HexRgb));
    });
    QObject::connect(bgPickBtn, &QPushButton::clicked, &dlg, [this, bgEdit, &dlg]() {
        const QColor c = pickColorWithHex(&dlg, mazeBackgroundColor_, "Maze Background Color");
        if (c.isValid()) bgEdit->setText(c.name(QColor::HexRgb));
    });

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]() {
        const QColor wall = parseColor(wallEdit, mazeWallColor_);
        const QColor bg = parseColor(bgEdit, mazeBackgroundColor_);
        if (wall.isValid() && bg.isValid()) {
            mazeWallColor_ = wall;
            mazeBackgroundColor_ = bg;
            applyMazeColors();
            dlg.accept();
        }
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    dlg.adjustSize();
    dlg.setFixedSize(dlg.sizeHint());
    dlg.exec();
}

void MazeWindow::updateStatusBarText(const QString& message) {
    if (statusLabel_) {
        statusLabel_->setText(message);
    }
    if (!coordLabel_->isVisible()) {
        coordStatusLabel_->clear();
    }
}

void MazeWindow::closeEvent(QCloseEvent* event) {
    saveActiveProgress();
    persistState();
    QMainWindow::closeEvent(event);
    if (event->isAccepted()) {
        QCoreApplication::quit();
    }
}

void MazeWindow::goHome() {
    saveActiveProgress();
    savedList_->clearSelection();
    savedList_->setCurrentRow(-1);
    clearActivePuzzle();
}

void MazeWindow::renderActivePuzzle(QPrinter& printer) {
QWidget* widgetToRender = nullptr;
switch (activeMode_) {
    case ActiveMode::Crossword:
        widgetToRender = currentCrossword_ ? static_cast<QWidget*>(crosswordWidget_) : nullptr;
        break;
        case ActiveMode::WordSearch:
            widgetToRender = currentWordSearch_ ? static_cast<QWidget*>(wordSearchWidget_) : nullptr;
            break;
        case ActiveMode::Sudoku:
            widgetToRender = currentSudoku_ ? static_cast<QWidget*>(sudokuWidget_) : nullptr;
            break;
        case ActiveMode::Cryptogram:
            widgetToRender = currentCryptogram_ ? static_cast<QWidget*>(cryptogramWidget_) : nullptr;
            break;
        case ActiveMode::Maze:
            widgetToRender = game_.hasMaze() ? static_cast<QWidget*>(mazeWidget_) : nullptr;
            break;
    default:
        break;
}

std::unique_ptr<QWidget> tempWidget;
if (!widgetToRender) {
    const int selectedRow = savedList_ ? savedList_->currentRow() : -1;
    tempWidget = makeRenderWidgetForSelection(selectedRow);
    widgetToRender = tempWidget.get();
}
if (!widgetToRender) {
    return;
}

    QPainter painter(&printer);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();
    QSize widgetSize = widgetToRender->size();
    if (widgetSize.isEmpty()) {
        widgetSize = widgetToRender->sizeHint();
    }
    if (widgetSize.isEmpty()) {
        return;
    }
    const double sx = static_cast<double>(pageRect.width()) / static_cast<double>(widgetSize.width());
    const double sy = static_cast<double>(pageRect.height()) / static_cast<double>(widgetSize.height());
    const double scale = std::min(sx, sy);
    const double tx = pageRect.x() + (pageRect.width() - widgetSize.width() * scale) / 2.0;
    const double ty = pageRect.y() + (pageRect.height() - widgetSize.height() * scale) / 2.0;

    painter.translate(tx, ty);
    painter.scale(scale, scale);
    widgetToRender->render(&painter);
    painter.end();
}

bool MazeWindow::renderActivePuzzleIfAvailable() {
    switch (activeMode_) {
        case ActiveMode::Crossword: return static_cast<bool>(currentCrossword_);
        case ActiveMode::WordSearch: return static_cast<bool>(currentWordSearch_);
        case ActiveMode::Sudoku: return static_cast<bool>(currentSudoku_);
        case ActiveMode::Cryptogram: return static_cast<bool>(currentCryptogram_);
        case ActiveMode::Maze: return game_.hasMaze();
        default: {
            const int row = savedList_ ? savedList_->currentRow() : -1;
            if (row < 0 || row >= static_cast<int>(savedPuzzles_.size())) return false;
            const auto& p = savedPuzzles_[row];
            return (p.type == SavedPuzzle::Type::Maze && p.maze) ||
                   (p.type == SavedPuzzle::Type::Crossword && p.crossword) ||
                   (p.type == SavedPuzzle::Type::WordSearch && p.wordSearch) ||
                   (p.type == SavedPuzzle::Type::Sudoku && p.sudoku) ||
                   (p.type == SavedPuzzle::Type::Cryptogram && p.cryptogram);
        }
    }
}

std::unique_ptr<QWidget> MazeWindow::makeRenderWidgetForSelection(const int row) const {
    if (row < 0 || row >= static_cast<int>(savedPuzzles_.size())) {
        return nullptr;
    }
    const auto& p = savedPuzzles_[row];
    switch (p.type) {
        case SavedPuzzle::Type::Maze:
            if (p.maze) {
                auto w = std::make_unique<MazeWidget>();
                w->setColors(mazeWallColor_, mazeBackgroundColor_);
                w->setGraph(p.maze->graph, false, false, p.maze->entranceNode, p.maze->exitNode, p.maze->playerNode);
                return w;
            }
            break;
        case SavedPuzzle::Type::Crossword:
            if (p.crossword) {
                auto w = std::make_unique<CrosswordWidget>();
                w->setPuzzle(p.crossword->puzzle);
                w->setTestMode(false);
                return w;
            }
            break;
        case SavedPuzzle::Type::WordSearch:
            if (p.wordSearch) {
                auto w = std::make_unique<WordSearchWidget>();
                w->setPuzzle(p.wordSearch->puzzle);
                w->setTestMode(false);
                return w;
            }
            break;
        case SavedPuzzle::Type::Sudoku:
            if (p.sudoku) {
                auto w = std::make_unique<SudokuWidget>();
                w->setPuzzle(p.sudoku->puzzle);
                w->setTestMode(false);
                return w;
            }
            break;
        case SavedPuzzle::Type::Cryptogram:
            if (p.cryptogram) {
                auto w = std::make_unique<CryptogramWidget>();
                w->setPuzzle(p.cryptogram->puzzle);
                w->setTestMode(false);
                w->setFixedSize(w->sizeHint());
                return w;
            }
            break;
    }
    return nullptr;
}

void MazeWindow::printActivePuzzle() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);

    
    QString docName = "puzzle";
    if (savedList_ && savedList_->currentRow() >= 0 && savedList_->currentRow() < savedList_->count()) {
        const int row = savedList_->currentRow();
        if (row >= 0 && row < static_cast<int>(savedPuzzles_.size())) {
            const auto& p = savedPuzzles_[row];
            switch (p.type) {
                case SavedPuzzle::Type::Maze:
                    if (p.maze) docName = QString::fromStdString(p.maze->name);
                    break;
                case SavedPuzzle::Type::Crossword:
                    if (p.crossword) docName = QString::fromStdString(p.crossword->name);
                    break;
                case SavedPuzzle::Type::WordSearch:
                    if (p.wordSearch) docName = QString::fromStdString(p.wordSearch->name);
                    break;
                case SavedPuzzle::Type::Sudoku:
                    if (p.sudoku) docName = QString::fromStdString(p.sudoku->name);
                    break;
                case SavedPuzzle::Type::Cryptogram:
                    if (p.cryptogram) docName = QString::fromStdString(p.cryptogram->name);
                    break;
            }
        }
    }
    printer.setDocName(docName);

    if (!renderActivePuzzleIfAvailable()) {
        showSizedMessage(this, QMessageBox::Information, "Print", "No active puzzle to print.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Print");
    dialog.setModal(true);
    dialog.setSizeGripEnabled(false);
    dialog.setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    auto* layout = new QVBoxLayout(&dialog);
    auto* preview = new QPrintPreviewWidget(&printer, &dialog);
    preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    preview->setZoomMode(QPrintPreviewWidget::FitInView);
    connect(preview, &QPrintPreviewWidget::paintRequested, this, [&](QPrinter* p) {
        renderActivePuzzle(*p);
    });
    layout->addWidget(preview, 1);

    auto* buttonsLayout = new QHBoxLayout();
    auto* settingsBtn = new QPushButton("Settings...", &dialog);
    auto* printBtn = new QPushButton("Print", &dialog);
    auto* cancelBtn = new QPushButton("Cancel", &dialog);
    buttonsLayout->addWidget(settingsBtn);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(printBtn);
    buttonsLayout->addWidget(cancelBtn);
    layout->addLayout(buttonsLayout);

    connect(settingsBtn, &QPushButton::clicked, this, [&]() {
        QPrintDialog settingsDlg(&printer, &dialog);
        settingsDlg.setWindowTitle("Printer Settings");
        if (settingsDlg.exec() == QDialog::Accepted) {
            preview->updatePreview();
        }
    });
    connect(printBtn, &QPushButton::clicked, &dialog, [&]() {
        renderActivePuzzle(printer);
        dialog.accept();
    });
    connect(cancelBtn, &QPushButton::clicked, &dialog, [&]() { dialog.reject(); });

    dialog.resize(1000, 800);
    preview->updatePreview();
    dialog.exec();
}

void MazeWindow::printPreviewActivePuzzle() {
    printActivePuzzle();
}

void MazeWindow::showContextMenu(const QPoint& pos) {
    const auto item = savedList_->itemAt(pos);
    if (!item) {
        return;
    }
    const int row = savedList_->row(item);
    savedList_->setCurrentRow(row);

    QMenu menu(this);
    const bool isActiveRow = inTestMode_ && row == loadedIndex_;
    QAction* testAction = menu.addAction(isActiveRow ? "Restart Test" : "Test");
    connect(testAction, &QAction::triggered, this, &MazeWindow::testSelected);
    if (isActiveRow) {
        QAction* endTestAction = menu.addAction("End Test");
        connect(endTestAction, &QAction::triggered, this, &MazeWindow::goHome);
    }
    QAction* saveImageAction = menu.addAction("Save as Image...");
    connect(saveImageAction, &QAction::triggered, this, &MazeWindow::saveSelectedImage);
    QAction* printAction = menu.addAction("Print...");
    connect(printAction, &QAction::triggered, this, &MazeWindow::printActivePuzzle);
    menu.addSeparator();
    QAction* deleteAction = menu.addAction("Delete");
    connect(deleteAction, &QAction::triggered, this, &MazeWindow::deleteSelected);
    menu.exec(savedList_->viewport()->mapToGlobal(pos));
}

void MazeWindow::testSelected() {
    saveActiveProgress();
    const int row = savedList_->currentRow();
    if (row < 0 || row >= static_cast<int>(savedPuzzles_.size())) {
        return;
    }
    const auto& puzzle = savedPuzzles_[row];
    if (puzzle.type == SavedPuzzle::Type::Maze) {
        loadMaze(row);
    } else if (puzzle.type == SavedPuzzle::Type::Crossword) {
        loadCrossword(row);
    } else if (puzzle.type == SavedPuzzle::Type::WordSearch) {
        loadWordSearch(row);
    } else if (puzzle.type == SavedPuzzle::Type::Sudoku) {
        loadSudoku(row);
    } else if (puzzle.type == SavedPuzzle::Type::Cryptogram) {
        loadCryptogram(row);
    }
}

void MazeWindow::saveSelectedImage() {
    saveActiveProgress();
    const int row = savedList_->currentRow();
    if (row < 0 || row >= static_cast<int>(savedPuzzles_.size())) {
        return;
    }
    
    const auto& puzzle = savedPuzzles_[row];
    if (puzzle.type == SavedPuzzle::Type::Crossword && puzzle.crossword) {
        currentCrossword_ = puzzle.crossword->puzzle;
        lastCrosswordHints_ = puzzle.crossword->hints;
        saveCrosswordImage();
        return;
    }
    
    if (puzzle.type == SavedPuzzle::Type::WordSearch && puzzle.wordSearch) {
        currentWordSearch_ = puzzle.wordSearch->puzzle;
        saveWordSearchImage();
        return;
    }
    
    if (puzzle.type == SavedPuzzle::Type::Sudoku && puzzle.sudoku) {
        currentSudoku_ = puzzle.sudoku->puzzle;
        saveSudokuImage();
        return;
    }
    
    if (puzzle.type == SavedPuzzle::Type::Cryptogram && puzzle.cryptogram) {
        currentCryptogram_ = puzzle.cryptogram->puzzle;
        saveCryptogramImage();
        return;
    }
    
    if (puzzle.type != SavedPuzzle::Type::Maze || !puzzle.maze) {
        return;
    }
    
    const auto& saved = *puzzle.maze;
    const auto& graph = saved.graph;
    const int rows = graph.rows;
    const int cols = graph.cols;
    if (rows <= 0 || cols <= 0 || graph.nodes.empty()) {
        return;
    }

    const int cell = 10;
    QImage image(cols * cell, rows * cell, QImage::Format_ARGB32);
    image.fill(mazeBackgroundColor_);

    QPainter painter(&image);
    const int lineWidth = std::max(2, cell / 8);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(mazeWallColor_, lineWidth, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

    auto nodeId = [cols](int r, int c) { return r * cols + c; };
    auto isOpen = [&](int a, int b) {
        const int idx = edge_index_between(graph, a, b);
        return idx >= 0 && graph.edges[idx].open;
    };

    
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols - 1; ++c) {
            if (!isOpen(nodeId(r, c), nodeId(r, c + 1))) {
                const int x = (c + 1) * cell;
                painter.drawLine(x, r * cell, x, (r + 1) * cell);
            }
        }
    }
    for (int r = 0; r < rows - 1; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (!isOpen(nodeId(r, c), nodeId(r + 1, c))) {
                const int y = (r + 1) * cell;
                painter.drawLine(c * cell, y, (c + 1) * cell, y);
            }
        }
    }

    
    painter.drawLine(0, 0, cols * cell, 0);
    painter.drawLine(0, rows * cell, cols * cell, rows * cell);
    painter.drawLine(0, 0, 0, rows * cell);
    painter.drawLine(cols * cell, 0, cols * cell, rows * cell);

    auto carveGap = [&](int nodeIndex) {
        if (nodeIndex < 0 || nodeIndex >= static_cast<int>(graph.nodes.size())) return;
        const auto& n = graph.nodes[nodeIndex];
        const int x = n.col * cell;
        const int y = n.row * cell;
        painter.setPen(Qt::NoPen);
        painter.setBrush(mazeBackgroundColor_);
        const int gapW = std::max(lineWidth, cell - lineWidth * 2);
        const int outerLineWidth = lineWidth * 2;
        if (n.row == 0) {
            painter.drawRect(QRect(x + (cell - gapW) / 2, y - outerLineWidth, gapW, outerLineWidth * 2));
        } else if (n.row == rows - 1) {
            painter.drawRect(QRect(x + (cell - gapW) / 2, y + cell - outerLineWidth, gapW, outerLineWidth * 2));
        } else if (n.col == 0) {
            painter.drawRect(QRect(x - outerLineWidth, y + (cell - gapW) / 2, outerLineWidth * 2, gapW));
        } else if (n.col == cols - 1) {
            painter.drawRect(QRect(x + cell - outerLineWidth, y + (cell - gapW) / 2, outerLineWidth * 2, gapW));
        }
    };

    carveGap(saved.entranceNode);
    carveGap(saved.exitNode);

    const QString suggested = QString::fromStdString(saved.name).isEmpty()
        ? QString("maze.png")
        : QString::fromStdString(saved.name) + ".png";
    const QString path = QFileDialog::getSaveFileName(this, "Save Maze Image", suggested, "PNG Images (*.png)");
    if (path.isEmpty()) {
        return;
    }

    if (!image.save(path, "PNG")) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Could not save the maze image.");
    }
}

QString MazeWindow::stateFilePath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) {
        dir = QDir::currentPath();
    }
    QDir().mkpath(dir);
    return dir + "/puzzles_state.json";
}

void MazeWindow::persistState() const {
    if (isRestoring_) {
        return;
    }

    
    const_cast<MazeWindow*>(this)->saveActiveProgress();

    QJsonArray puzzles;

    for (const auto& saved : savedPuzzles_) {
        QJsonObject obj;
        switch (saved.type) {
            case SavedPuzzle::Type::Maze:
                if (saved.maze) {
                    const auto& maze = *saved.maze;
                    obj["type"] = "maze";
                    obj["name"] = QString::fromStdString(maze.name);
                    obj["algorithm"] = algorithmToInt(maze.algorithm);
                    obj["width"] = maze.width;
                    obj["height"] = maze.height;
                    obj["graph"] = graphToJson(maze.graph);
                    obj["entranceNode"] = maze.entranceNode;
                    obj["exitNode"] = maze.exitNode;
                    obj["playerNode"] = maze.playerNode;
                    puzzles.append(obj);
                }
                break;
            case SavedPuzzle::Type::Crossword:
                if (saved.crossword) {
                    const auto& cw = *saved.crossword;
                    obj["type"] = "crossword";
                    obj["name"] = QString::fromStdString(cw.name);
                    QJsonArray grid;
                    for (const auto& row : cw.puzzle.grid) {
                        grid.append(QString::fromStdString(row));
                    }
                    obj["grid"] = grid;
                    obj["numbers"] = intGridToJson(cw.puzzle.numbers);
                    QJsonArray across;
                    for (const auto& entry : cw.puzzle.across) {
                        QJsonObject e;
                        e["number"] = entry.number;
                        e["word"] = QString::fromStdString(entry.word);
                        across.append(e);
                    }
                    obj["across"] = across;
                    QJsonArray down;
                    for (const auto& entry : cw.puzzle.down) {
                        QJsonObject e;
                        e["number"] = entry.number;
                        e["word"] = QString::fromStdString(entry.word);
                        down.append(e);
                    }
                    obj["down"] = down;
                    QJsonObject hints;
                    for (const auto& [word, hint] : cw.hints) {
                        hints[QString::fromStdString(word)] = QString::fromStdString(hint);
                    }
                    obj["hints"] = hints;
                    puzzles.append(obj);
                }
                break;
            case SavedPuzzle::Type::WordSearch:
                if (saved.wordSearch) {
                    const auto& ws = *saved.wordSearch;
                    obj["type"] = "wordsearch";
                    obj["name"] = QString::fromStdString(ws.name);
                    obj["size"] = ws.puzzle.size;
                    QJsonArray grid;
                    for (const auto& row : ws.puzzle.grid) {
                        std::string rowStr(row.begin(), row.end());
                        grid.append(QString::fromStdString(rowStr));
                    }
                    obj["grid"] = grid;
                    QJsonArray words;
                    for (const auto& word : ws.puzzle.words) {
                        words.append(QString::fromStdString(word));
                    }
                    obj["words"] = words;
                    puzzles.append(obj);
                }
                break;
            case SavedPuzzle::Type::Sudoku:
                if (saved.sudoku) {
                    const auto& sdk = *saved.sudoku;
                    obj["type"] = "sudoku";
                    obj["name"] = QString::fromStdString(sdk.name);
                    obj["difficulty"] = sdk.difficulty;
                    obj["grid"] = intGridToJson(sdk.puzzle.grid);
                    obj["solution"] = intGridToJson(sdk.puzzle.solution);
                    puzzles.append(obj);
                }
                break;
            case SavedPuzzle::Type::Cryptogram:
                if (saved.cryptogram) {
                    const auto& crypto = *saved.cryptogram;
                    obj["type"] = "cryptogram";
                    obj["name"] = QString::fromStdString(crypto.name);
                    obj["plainText"] = QString::fromStdString(crypto.puzzle.plainText);
                    obj["cipherText"] = QString::fromStdString(crypto.puzzle.cipherText);
                    QJsonObject mapping;
                    for (const auto& [cipher, plain] : crypto.puzzle.cipherToPlain) {
                        mapping[QString(QChar(cipher))] = QString(QChar(plain));
                    }
                    obj["cipherToPlain"] = mapping;
                    QJsonArray revealed;
                    for (char r : crypto.puzzle.revealed) {
                        revealed.append(QString(QChar(r)));
                    }
                    obj["revealed"] = revealed;
                    obj["avoidSelfMapping"] = crypto.avoidSelfMapping;
                    obj["hintCount"] = crypto.hintCount;
                    puzzles.append(obj);
                }
                break;
        }
    }

    QJsonObject root;
    root["version"] = 1;
    root["puzzles"] = puzzles;
    root["selectedIndex"] = savedList_ ? savedList_->currentRow() : -1;
    root["loadedIndex"] = loadedIndex_;
    root["inTestMode"] = inTestMode_;
    root["activeType"] = static_cast<int>(activeMode_);

    QSaveFile file(stateFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    file.commit();
}

void MazeWindow::restoreState() {
    const QString path = stateFilePath();
    QFile file(path);
    if (!file.exists()) {
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }

    const auto root = doc.object();
    const int version = root.value("version").toInt(0);
    if (version != 1) {
        file.close();
        return;
    }
    const QJsonArray puzzles = root.value("puzzles").toArray();

    isRestoring_ = true;
    loadedIndex_ = -1;
    inTestMode_ = false;
    activeMode_ = ActiveMode::None;
    savedPuzzles_.clear();
    savedMazes_.clear();
    savedCryptograms_.clear();
    savedList_->clear();

    for (const auto& entryVal : puzzles) {
        const QJsonObject obj = entryVal.toObject();
        const QString type = obj.value("type").toString();
        if (type == "maze") {
            const auto graph = graphFromJson(obj.value("graph").toObject());
            if (!graph) {
                continue;
            }
            SavedMaze maze {
                obj.value("name").toString("Maze").toStdString(),
                algorithmFromInt(obj.value("algorithm").toInt(static_cast<int>(GenerationAlgorithm::DFS))),
                obj.value("width").toInt(graph->cols),
                obj.value("height").toInt(graph->rows),
                *graph,
                obj.value("entranceNode").toInt(graph->entranceNode),
                obj.value("exitNode").toInt(graph->exitNode),
                obj.value("playerNode").toInt(graph->entranceNode)
            };
            SavedPuzzle saved;
            saved.type = SavedPuzzle::Type::Maze;
            saved.maze = maze;
            savedPuzzles_.push_back(saved);
            savedMazes_.push_back(maze);
            savedList_->addItem(QString("%1 — %2 (%3x%4)")
                .arg(QString::fromStdString(maze.name))
                .arg(algorithmLabel(maze.algorithm))
                .arg(maze.width)
                .arg(maze.height));
        } else if (type == "crossword") {
            SavedCrossword cw;
            cw.name = obj.value("name").toString("Crossword").toStdString();
            for (const auto& rowVal : obj.value("grid").toArray()) {
                cw.puzzle.grid.push_back(rowVal.toString().toStdString());
            }
            cw.puzzle.numbers = intGridFromJson(obj.value("numbers").toArray());
            for (const auto& entryValAcross : obj.value("across").toArray()) {
                const auto eObj = entryValAcross.toObject();
                CrosswordEntry cwEntry;
                cwEntry.number = eObj.value("number").toInt();
                cwEntry.word = eObj.value("word").toString().toStdString();
                cw.puzzle.across.push_back(cwEntry);
            }
            for (const auto& entryValDown : obj.value("down").toArray()) {
                const auto eObj = entryValDown.toObject();
                CrosswordEntry cwEntry;
                cwEntry.number = eObj.value("number").toInt();
                cwEntry.word = eObj.value("word").toString().toStdString();
                cw.puzzle.down.push_back(cwEntry);
            }
            const auto hints = obj.value("hints").toObject();
            for (auto it = hints.begin(); it != hints.end(); ++it) {
                cw.hints[it.key().toStdString()] = it.value().toString().toStdString();
            }
            if (cw.puzzle.grid.empty()) {
                continue;
            }
            SavedPuzzle saved;
            saved.type = SavedPuzzle::Type::Crossword;
            saved.crossword = cw;
            savedPuzzles_.push_back(saved);
            const int wordCount = static_cast<int>(cw.puzzle.across.size() + cw.puzzle.down.size());
            savedList_->addItem(QString("%1 — %2 words")
                .arg(QString::fromStdString(cw.name))
                .arg(wordCount));
        } else if (type == "wordsearch") {
            SavedWordSearch ws;
            ws.name = obj.value("name").toString("Word Search").toStdString();
            ws.puzzle.size = obj.value("size").toInt();
            for (const auto& rowVal : obj.value("grid").toArray()) {
                const auto rowStr = rowVal.toString().toStdString();
                std::vector<char> row(rowStr.begin(), rowStr.end());
                ws.puzzle.grid.push_back(row);
            }
            for (const auto& wordVal : obj.value("words").toArray()) {
                ws.puzzle.words.push_back(wordVal.toString().toStdString());
            }
            if (ws.puzzle.size <= 0 || ws.puzzle.grid.empty()) {
                continue;
            }
            SavedPuzzle saved;
            saved.type = SavedPuzzle::Type::WordSearch;
            saved.wordSearch = ws;
            savedPuzzles_.push_back(saved);
            savedList_->addItem(QString::fromStdString(ws.name));
        } else if (type == "sudoku") {
            SavedSudoku sdk;
            sdk.name = obj.value("name").toString("Sudoku").toStdString();
            sdk.difficulty = obj.value("difficulty").toInt(0);
            sdk.puzzle.grid = intGridFromJson(obj.value("grid").toArray());
            sdk.puzzle.solution = intGridFromJson(obj.value("solution").toArray());
            if (sdk.puzzle.grid.empty()) {
                continue;
            }
            SavedPuzzle saved;
            saved.type = SavedPuzzle::Type::Sudoku;
            saved.sudoku = sdk;
            savedPuzzles_.push_back(saved);
            savedList_->addItem(QString("%1 — %2")
                .arg(QString::fromStdString(sdk.name))
                .arg(sudokuDifficultyLabel(sdk.difficulty)));
        } else if (type == "cryptogram") {
            SavedCryptogram crypto;
            crypto.name = obj.value("name").toString("Cryptogram").toStdString();
            crypto.puzzle.plainText = obj.value("plainText").toString().toStdString();
            crypto.puzzle.cipherText = obj.value("cipherText").toString().toStdString();
            const auto mapping = obj.value("cipherToPlain").toObject();
            for (auto it = mapping.begin(); it != mapping.end(); ++it) {
                if (!it.key().isEmpty() && !it.value().toString().isEmpty()) {
                    crypto.puzzle.cipherToPlain[it.key().at(0).toLatin1()] = it.value().toString().at(0).toLatin1();
                }
            }
            for (const auto& val : obj.value("revealed").toArray()) {
                const auto str = val.toString();
                if (!str.isEmpty()) {
                    crypto.puzzle.revealed.insert(str.at(0).toLatin1());
                }
            }
            crypto.avoidSelfMapping = obj.value("avoidSelfMapping").toBool(true);
            crypto.hintCount = obj.value("hintCount").toInt(0);
            SavedPuzzle saved;
            saved.type = SavedPuzzle::Type::Cryptogram;
            saved.cryptogram = crypto;
            savedPuzzles_.push_back(saved);
            savedCryptograms_.push_back(crypto);
            savedList_->addItem(QString("%1 — %2 chars")
                .arg(QString::fromStdString(crypto.name))
                .arg(static_cast<int>(crypto.puzzle.plainText.size())));
        }
    }

    const int selectedIndex = root.value("selectedIndex").toInt(-1);
    const int loadedIndex = root.value("loadedIndex").toInt(-1);
    const bool wasTesting = root.value("inTestMode").toBool(false);
    const auto priorMode = static_cast<ActiveMode>(root.value("activeType").toInt(0));

    if (savedList_->count() == 0) {
        isRestoring_ = false;
        updateStatus();
        refreshActions();
        return;
    }

    const int clampedSelected = (selectedIndex >= 0 && selectedIndex < savedList_->count())
        ? selectedIndex
        : -1;
    const int clampedLoaded = (loadedIndex >= 0 && loadedIndex < savedList_->count())
        ? loadedIndex
        : -1;

    if (wasTesting && clampedLoaded >= 0) {
        savedList_->setCurrentRow(clampedLoaded);
        switch (priorMode) {
            case ActiveMode::Maze: loadMaze(clampedLoaded); break;
            case ActiveMode::Crossword: loadCrossword(clampedLoaded); break;
            case ActiveMode::WordSearch: loadWordSearch(clampedLoaded); break;
            case ActiveMode::Sudoku: loadSudoku(clampedLoaded); break;
            case ActiveMode::Cryptogram: loadCryptogram(clampedLoaded); break;
            case ActiveMode::None: handleSelectionChanged(clampedLoaded); break;
        }
    } else if (clampedSelected >= 0) {
        savedList_->setCurrentRow(clampedSelected);
        handleSelectionChanged(clampedSelected);
    } else {
        savedList_->setCurrentRow(0);
        handleSelectionChanged(0);
    }

    isRestoring_ = false;
}

void MazeWindow::saveCrosswordImage() {
    if (!currentCrossword_) {
        return;
    }
    const auto& puzzle = *currentCrossword_;
    const int rows = static_cast<int>(puzzle.grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(puzzle.grid.front().size());
    if (rows == 0 || cols == 0) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Nothing to export.");
        return;
    }

    const int cell = 28;
    const int gridWidth = cols * cell;
    const int gridHeight = rows * cell;

    QString acrossText = "ACROSS\n";
    for (const auto& entry : puzzle.across) {
        const auto it = lastCrosswordHints_.find(entry.word);
        const QString hint = (it != lastCrosswordHints_.end() && !it->second.empty())
            ? QString::fromStdString(it->second)
            : QString::fromStdString(entry.word);
        acrossText.append(QString("%1. %2\n").arg(entry.number).arg(hint));
    }
    QString downText = "DOWN\n";
    for (const auto& entry : puzzle.down) {
        const auto it = lastCrosswordHints_.find(entry.word);
        const QString hint = (it != lastCrosswordHints_.end() && !it->second.empty())
            ? QString::fromStdString(it->second)
            : QString::fromStdString(entry.word);
        downText.append(QString("%1. %2\n").arg(entry.number).arg(hint));
    }

    QFont gridFont;
    gridFont.setPointSize(10);
    QFont numberFont;
    numberFont.setPointSize(7);
    QFont clueFont;
    clueFont.setPointSize(10);

    QFontMetrics clueMetrics(clueFont);
    const int clueWidth = std::max(clueMetrics.horizontalAdvance(acrossText), clueMetrics.horizontalAdvance(downText));
    const int cluesHeight = clueMetrics.lineSpacing() * static_cast<int>(puzzle.across.size() + puzzle.down.size() + 4);

    const int padding = 20;
    const int imageWidth = std::max(gridWidth, clueWidth + padding * 2);
    const int imageHeight = gridHeight + cluesHeight + padding * 3;

    QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const int offsetX = (imageWidth - gridWidth) / 2;
    const int offsetY = padding;
    painter.setFont(gridFont);
    painter.setPen(Qt::black);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const QRect tile(offsetX + c * cell, offsetY + r * cell, cell, cell);
            const char ch = puzzle.grid[r][c];
            if (ch == '#') {
                painter.fillRect(tile, QColor(30, 30, 30));
            } else {
                painter.fillRect(tile, Qt::white);
                const int num = puzzle.numbers.empty() ? 0 : puzzle.numbers[r][c];
                if (num > 0) {
                    painter.setFont(numberFont);
                    painter.drawText(tile.adjusted(3, 1, -1, -cell / 2), Qt::AlignLeft | Qt::AlignTop, QString::number(num));
                }
            }
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(tile);
        }
    }

    painter.setFont(clueFont);
    painter.setPen(Qt::black);
    const int clueTop = offsetY + gridHeight + padding;
    painter.drawText(QRect(padding, clueTop, imageWidth - padding * 2, cluesHeight / 2), Qt::AlignLeft | Qt::AlignTop, acrossText);
    painter.drawText(QRect(padding, clueTop + cluesHeight / 2, imageWidth - padding * 2, cluesHeight / 2), Qt::AlignLeft | Qt::AlignTop, downText);

    const QString suggested = QString("crossword.png");
    const QString path = QFileDialog::getSaveFileName(this, "Save Crossword Image", suggested, "PNG Images (*.png)");
    if (path.isEmpty()) {
        return;
    }
    if (!image.save(path, "PNG")) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Could not save the crossword image.");
    }
}

void MazeWindow::saveWordSearchImage() {
    if (!currentWordSearch_) {
        return;
    }
    const auto& puzzle = *currentWordSearch_;
    const int size = puzzle.size;
    if (size == 0) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Nothing to export.");
        return;
    }

    const int cell = 32;
    const int gridWidth = size * cell;
    const int gridHeight = size * cell;

    QString wordsText = "WORDS TO FIND:\n";
    for (const auto& word : puzzle.words) {
        wordsText.append(QString::fromStdString(word) + "\n");
    }

    QFont gridFont;
    gridFont.setPointSize(12);
    gridFont.setBold(true);
    QFont wordFont;
    wordFont.setPointSize(10);

    QFontMetrics wordMetrics(wordFont);
    const int wordListWidth = 200;
    const int wordListHeight = wordMetrics.lineSpacing() * static_cast<int>(puzzle.words.size() + 2);

    const int padding = 20;
    const int imageWidth = gridWidth + wordListWidth + padding * 3;
    const int imageHeight = std::max(gridHeight, wordListHeight) + padding * 2;

    QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int offsetX = padding;
    const int offsetY = padding;
    painter.setFont(gridFont);
    painter.setPen(Qt::black);
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            const QRect tile(offsetX + c * cell, offsetY + r * cell, cell, cell);
            painter.fillRect(tile, Qt::white);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(tile);
            
            const char ch = puzzle.grid[r][c];
            painter.setPen(Qt::black);
            painter.drawText(tile, Qt::AlignCenter, QString(QChar(ch)).toUpper());
        }
    }

    painter.setFont(wordFont);
    painter.setPen(Qt::black);
    const int wordListX = offsetX + gridWidth + padding;
    painter.drawText(QRect(wordListX, offsetY, wordListWidth, wordListHeight), Qt::AlignLeft | Qt::AlignTop, wordsText);

    const QString suggested = QString("wordsearch.png");
    const QString path = QFileDialog::getSaveFileName(this, "Save Word Search Image", suggested, "PNG Images (*.png)");
    if (path.isEmpty()) {
        return;
    }
    if (!image.save(path, "PNG")) {
        showSizedMessage(this, QMessageBox::Warning, "Save Failed", "Could not save the word search image.");
    }
}
void MazeWindow::saveActiveProgress() {
    if (!inTestMode_ || loadedIndex_ < 0 || loadedIndex_ >= static_cast<int>(savedPuzzles_.size())) {
        return;
    }
    auto& puzzle = savedPuzzles_[loadedIndex_];
    if (puzzle.type == SavedPuzzle::Type::Maze && puzzle.maze) {
        puzzle.maze->graph = game_.graph();
        puzzle.maze->entranceNode = game_.entranceNode();
        puzzle.maze->exitNode = game_.exitNode();
        puzzle.maze->playerNode = game_.playerNode();
        auto it = std::find_if(savedMazes_.begin(), savedMazes_.end(),
            [&](const SavedMaze& m) { return m.name == puzzle.maze->name; });
        if (it != savedMazes_.end()) {
            *it = *puzzle.maze;
        }
    } else if (puzzle.type == SavedPuzzle::Type::Crossword && puzzle.crossword) {
        
    } else if (puzzle.type == SavedPuzzle::Type::WordSearch && puzzle.wordSearch) {
        
    } else if (puzzle.type == SavedPuzzle::Type::Sudoku && puzzle.sudoku) {
        
    } else if (puzzle.type == SavedPuzzle::Type::Cryptogram && puzzle.cryptogram) {
        
    }
}
