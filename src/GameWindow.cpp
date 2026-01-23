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
    connect(mapScreen, &MapScreen::levelSelected, this, &GameWindow::showGameLevel);
    connect(mapScreen, &MapScreen::backToMenu, this, &GameWindow::backToMenu);
    connect(mapScreen, &MapScreen::bossBattleStarted, this, &GameWindow::showBossBattle);
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
void GameWindow::showGameLevel(int level)
{
    // 这里可以加载对应的关卡数据
    qDebug() << "Starting level:" << level;
    
    // 显示关卡信息
    QMessageBox::information(this, "Level Start", 
                           QString("Starting Level %1\n\nPrepare for adventure!").arg(level));
    
    stackedWidget->setCurrentIndex(2); // 切换到游戏界面
}
void GameWindow::showBossBattle(int level)
{
    connect(gameScreen, &GameScreen::returnToMapRequested, this, &GameWindow::backToMenu);
    connect(gameScreen, &GameScreen::gameOver, this, [this](bool victory) {
        if (victory) {
            QMessageBox::information(this, "Victory!", "You won the battle!");
        } else {
            QMessageBox::information(this, "Defeat", "You were defeated!");
        }
        stackedWidget->setCurrentIndex(1); // 返回地图
    });
    gameScreen->loadScene(GameSceneType::BOSS_BATTLE, level);
    
    // 切换到GameScreen界面
    stackedWidget->setCurrentIndex(2);
}