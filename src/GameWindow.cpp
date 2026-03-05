#include "GameWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include "BaseRenderer.h"

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , stackedWidget(new QStackedWidget(this))
    , startScreen(new StartScreen(this))
    , mapScreen(new MapScreen(this))
    , gameScreen(new GameScreen(this))
{
    setCentralWidget(stackedWidget);
    setupUI();
    
    // 连接信号和槽
    connect(startScreen, &StartScreen::startClicked, this, &GameWindow::startGame);
    connect(startScreen, &StartScreen::settingsClicked, this, &GameWindow::showSettings);
    connect(startScreen, &StartScreen::quitClicked, this, &GameWindow::quitGame);

    // 连接地图界面的信号
    connect(mapScreen, &MapScreen::backToMenu, this, &GameWindow::backToMenu);
    connect(mapScreen, &MapScreen::gameStart, gameScreen, &GameScreen::onGameStart);
    connect(mapScreen, &MapScreen::gameStart, this, [this](SceneType type, int level) {
        stackedWidget->setCurrentIndex(2); // 切换到游戏界面
        qDebug() << "MapScreen triggered game start - type:" << static_cast<int>(type) << " level:" << level;
    });

    // 连接游戏界面信号
    connect(gameScreen, &GameScreen::returnToMapRequested, this, &GameWindow::backToMenu);
}

GameWindow::~GameWindow()
{
}

void GameWindow::setupUI()
{
    // 设置开始界面（0）
    stackedWidget->addWidget(startScreen);
    // 设置地图界面（1）
    stackedWidget->addWidget(mapScreen);
    // 设置游戏界面（2）
    stackedWidget->addWidget(gameScreen);
}

void GameWindow::startGame()
{
    stackedWidget->setCurrentIndex(1);  // 切换到地图
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
