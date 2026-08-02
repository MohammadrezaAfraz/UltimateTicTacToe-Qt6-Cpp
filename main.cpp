/************************************************************************
 * *
 * فایل ترکیبی کد بازی Ultimate Tic-Tac-Toe با استفاده از Qt           *
 * این فایل برای نمایش یکجا است و نباید به صورت مستقیم کامپایل شود.    *
 * *
 ************************************************************************/

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

// =======================================================================
// |                        GameLogic Class                              |
// =======================================================================

class GameLogic
{
public:
    std::vector<std::vector<char>> board;
    std::vector<char> bigBoard;
    int currentBoard;
    char currentPlayer;
    int scoreX;
    int scoreO;

    GameLogic()
    {
        scoreX = 0;
        scoreO = 0;
        reset();
    }

    void reset()
    {
        board = std::vector<std::vector<char>>(9, std::vector<char>(9, '\0'));
        bigBoard = std::vector<char>(9, '\0');
        currentBoard = -1;
        currentPlayer = 'X';
    }

    bool isBoardPlayable(int boardIndex)
    {
        return bigBoard[boardIndex] == '\0' && !isFull(board[boardIndex]);
    }

    bool checkWin(const std::vector<char> &b, char player)
    {
        const int wins[8][3] = {{0, 1, 2},
                                {3, 4, 5},
                                {6, 7, 8},
                                {0, 3, 6},
                                {1, 4, 7},
                                {2, 5, 8},
                                {0, 4, 8},
                                {2, 4, 6}};
        for (int i = 0; i < 8; ++i) {
            if (b[wins[i][0]] == player && b[wins[i][1]] == player && b[wins[i][2]] == player) {
                return true;
            }
        }
        return false;
    }

    bool isFull(const std::vector<char> &b)
    {
        for (char cell : b) {
            if (cell == '\0')
                return false;
        }
        return true;
    }

    void switchPlayer() { currentPlayer = (currentPlayer == 'X') ? 'O' : 'X'; }
};

// =======================================================================
// |                        GameWidget Class                             |
// =======================================================================
class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    void startGame();

private slots:
    void onCellClicked();

private:
    void setupUI();
    void updateBoardUI();
    void handleMove(int boardIdx, int cellIdx);
    void checkSmallBoardWin(int boardIdx);
    void checkBigBoardWin();

    GameLogic logic;

    std::vector<QPushButton *> cellButtons;
    std::vector<QFrame *> smallBoards;
    QLabel *statusLabel;
    QLabel *infoLabel;
};

// =======================================================================
// |                        MainWindow Class                             |
// =======================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showGame();

private:
    QStackedWidget *stackedWidget;
    QWidget *menuWidget;
    GameWidget *gameWidget;
    QPushButton *startButton;
};

// =======================================================================
// |                        GameWidget Implementation                    |
// =======================================================================

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void GameWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    statusLabel = new QLabel("برای شروع بازی از منوی اصلی اقدام کنید.");
    statusLabel->setAlignment(Qt::AlignCenter);
    QFont labelFont("Arial", 16, QFont::Bold);
    statusLabel->setFont(labelFont);

    infoLabel = new QLabel(" ");
    infoLabel->setAlignment(Qt::AlignCenter);
    QFont infoFont("Arial", 12);
    infoLabel->setFont(infoFont);

    QGridLayout *boardLayout = new QGridLayout();
    boardLayout->setSpacing(10);

    for (int i = 0; i < 9; ++i) {
        QFrame *smallBoardFrame = new QFrame();
        smallBoardFrame->setFrameStyle(QFrame::Box | QFrame::Plain);
        smallBoardFrame->setLineWidth(2);

        QGridLayout *smallLayout = new QGridLayout(smallBoardFrame);
        smallLayout->setSpacing(2);

        for (int j = 0; j < 9; ++j) {
            QPushButton *button = new QPushButton();
            button->setMinimumSize(40, 40);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            QFont font("Arial", 20, QFont::Bold);
            button->setFont(font);

            button->setProperty("boardIndex", i);
            button->setProperty("cellIndex", j);

            connect(button, &QPushButton::clicked, this, &GameWidget::onCellClicked);

            cellButtons.push_back(button);
            smallLayout->addWidget(button, j / 3, j % 3);
        }
        smallBoards.push_back(smallBoardFrame);
        boardLayout->addWidget(smallBoardFrame, i / 3, i % 3);
    }

    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(boardLayout);
    setLayout(mainLayout);
}

void GameWidget::startGame()
{
    logic.reset();
    for (auto *button : cellButtons) {
        button->setText("");
        button->setEnabled(true);
        button->setStyleSheet("");
    }
    updateBoardUI();
}

void GameWidget::onCellClicked()
{
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton)
        return;

    int boardIdx = clickedButton->property("boardIndex").toInt();
    int cellIdx = clickedButton->property("cellIndex").toInt();

    if (logic.currentBoard != -1 && logic.currentBoard != boardIdx) {
        QMessageBox::warning(this,
                             "خطا",
                             QString("حرکت نامعتبر! شما باید در برد شماره %1 بازی کنید.")
                                 .arg(logic.currentBoard + 1));
        return;
    }

    if (logic.board[boardIdx][cellIdx] != '\0') {
        QMessageBox::warning(this, "خطا", "این خانه قبلاً پر شده است!");
        return;
    }

    if (logic.bigBoard[boardIdx] != '\0') {
        QMessageBox::warning(this, "خطا", "این برد قبلاً تمام شده است!");
        return;
    }

    handleMove(boardIdx, cellIdx);
}

void GameWidget::handleMove(int boardIdx, int cellIdx)
{
    logic.board[boardIdx][cellIdx] = logic.currentPlayer;

    checkSmallBoardWin(boardIdx);

    logic.switchPlayer();

    logic.currentBoard = cellIdx;
    if (!logic.isBoardPlayable(logic.currentBoard)) {
        logic.currentBoard = -1;
    }

    updateBoardUI();

    // بررسی برنده شدن بعد از آپدیت UI و تعویض بازیکن انجام می‌شود
    // تا بازیکن قبلی به عنوان برنده اعلام شود
    checkBigBoardWin();
}

void GameWidget::checkSmallBoardWin(int boardIdx)
{
    // Note: check win for the player who just moved, which is still the currentPlayer
    if (logic.checkWin(logic.board[boardIdx], logic.currentPlayer)) {
        logic.bigBoard[boardIdx] = logic.currentPlayer;
        if (logic.currentPlayer == 'X')
            logic.scoreX++;
        else
            logic.scoreO++;

        for (int i = 0; i < 9; ++i) {
            int button_idx = boardIdx * 9 + i;
            cellButtons[button_idx]->setEnabled(false);
        }
    } else if (logic.isFull(logic.board[boardIdx])) {
        logic.bigBoard[boardIdx] = 'D';
    }
}

void GameWidget::checkBigBoardWin()
{
    // The winner is the player who just made the move, so we check for the *previous* player
    char lastPlayer = (logic.currentPlayer == 'X') ? 'O' : 'X';
    if (logic.checkWin(logic.bigBoard, lastPlayer)) {
        QMessageBox::information(this, "پایان بازی", QString("بازیکن %1 برنده شد!").arg(lastPlayer));
        startGame();
    } else if (logic.isFull(logic.bigBoard)) {
        QMessageBox::information(this, "پایان بازی", "بازی مساوی شد!");
        startGame();
    }
}

void GameWidget::updateBoardUI()
{
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            int button_idx = i * 9 + j;
            if (logic.board[i][j] != '\0') {
                cellButtons[button_idx]->setText(QString(logic.board[i][j]));
                QString style = (logic.board[i][j] == 'X') ? "color: red;" : "color: blue;";
                cellButtons[button_idx]
                    ->setStyleSheet(style + "font-size: 20px; font-weight: bold;");
                cellButtons[button_idx]->setEnabled(false);
            }
        }
    }

    for (int i = 0; i < 9; ++i) {
        smallBoards[i]->setStyleSheet("background-color: white; border: 2px solid black;");
        if (logic.bigBoard[i] != '\0') {
            QString color = (logic.bigBoard[i] == 'X')
                                ? "rgba(255, 0, 0, 0.2)"
                                : (logic.bigBoard[i] == 'O' ? "rgba(0, 0, 255, 0.2)" : "lightgray");
            smallBoards[i]
                ->setStyleSheet("background-color: " + color + "; border: 2px solid black;");
        }
    }

    if (logic.currentBoard != -1) {
        smallBoards[logic.currentBoard]
            ->setStyleSheet("background-color: lightgreen; border: 3px solid darkgreen;");
        infoLabel->setText(QString("نوبت شماست که در برد هایلایت شده بازی کنید."));
    } else {
        infoLabel->setText(
            "شما می‌توانید در هر برد آزادی (زرد رنگ) " "بازی " "کنید"
                                                                                              ".");
        for (int i = 0; i < 9; ++i) {
            if (logic.isBoardPlayable(i)) {
                smallBoards[i]
                    ->setStyleSheet("background-color: lightyellow; border: 3px solid orange;");
            }
        }
    }

    QString playerColor = (logic.currentPlayer == 'X') ? "red" : "blue";
    statusLabel
        ->setText(
            QString("امتیاز: <span style='color:red;'>X (%1)</span> - <span " "style='color:blue;'>"
                                                                              "O " "(%2)</span> | "
                                                                                   "نوبت بازیکن: "
                                                                                   "<span " "style="
                                                                                            "'color"
                                                                                            ":%3;'>"
                                                                                            "%4</"
                                                                                            "span>")
                .arg(logic.scoreX)
                .arg(logic.scoreO)
                .arg(playerColor)
                .arg(logic.currentPlayer));
}

// =======================================================================
// |                        MainWindow Implementation                    |
// =======================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    menuWidget = new QWidget;
    startButton = new QPushButton("شروع بازی");
    startButton->setMinimumSize(200, 80);
    QFont buttonFont("Arial", 20);
    startButton->setFont(buttonFont);

    QVBoxLayout *menuLayout = new QVBoxLayout(menuWidget);
    menuLayout->addWidget(startButton, 0, Qt::AlignCenter);
    menuWidget->setLayout(menuLayout);

    gameWidget = new GameWidget;

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(menuWidget);
    stackedWidget->addWidget(gameWidget);

    setCentralWidget(stackedWidget);
    setMinimumSize(600, 600);
    setWindowTitle("Ultimate Tic-Tac-Toe");

    connect(startButton, &QPushButton::clicked, this, &MainWindow::showGame);
}

void MainWindow::showGame()
{
    stackedWidget->setCurrentWidget(gameWidget);
    gameWidget->startGame();
}

MainWindow::~MainWindow() {}

// =======================================================================
// |                        main.cpp Entry Point                         |
// =======================================================================

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#include "main.moc"
