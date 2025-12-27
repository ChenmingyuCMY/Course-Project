#include "GameWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "GameRenderer.h"

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , stackedWidget(new QStackedWidget(this))
    , startScreen(new StartScreen(this))
    , gameScreen(new QWidget(this))
{
    setCentralWidget(stackedWidget);
    setupUI();
    
    // 连接信号和槽
    connect(startScreen, &StartScreen::startClicked, this, &GameWindow::startGame);
    connect(startScreen, &StartScreen::settingsClicked, this, &GameWindow::showSettings);
    connect(startScreen, &StartScreen::quitClicked, this, &GameWindow::quitGame);
}

GameWindow::~GameWindow()
{
}

void GameWindow::setupUI()
{
    // 设置开始界面
    stackedWidget->addWidget(startScreen);
    
    // 设置游戏界面
    QVBoxLayout *gameLayout = new QVBoxLayout(gameScreen);
    
    // 添加返回按钮
    QPushButton *backButton = new QPushButton("Back to Menu", gameScreen);
    connect(backButton, &QPushButton::clicked, this, &GameWindow::backToMenu);
    
    // 添加游戏渲染器
    GameRenderer *gameRenderer = new GameRenderer(gameScreen);
    
    gameLayout->addWidget(backButton);
    gameLayout->addWidget(gameRenderer, 1);
    
    gameScreen->setLayout(gameLayout);
    stackedWidget->addWidget(gameScreen);
}

void GameWindow::startGame()
{
    stackedWidget->setCurrentIndex(1); // 切换到游戏界面
}

void GameWindow::showSettings()
{
    QMessageBox::information(this, "Settings", "Settings menu will be implemented here.");
}

void GameWindow::quitGame()
{
    QApplication::quit();
}

void GameWindow::backToMenu()
{
    stackedWidget->setCurrentIndex(0); // 切换回开始界面
}