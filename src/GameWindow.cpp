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
    , mapScreen(new MapScreen(this))
    , gameScreen(new QWidget(this))
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
    QVBoxLayout *gameLayout = new QVBoxLayout(gameScreen);
    
    // 添加返回按钮
    QPushButton *backButton = new QPushButton("Back to Menu", gameScreen);
    connect(backButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(1); // 返回地图界面
    });
    
    // 添加游戏渲染器
    GameRenderer *gameRenderer = new GameRenderer(gameScreen);
    
    gameLayout->addWidget(backButton);
    gameLayout->addWidget(gameRenderer, 1);
    
    gameScreen->setLayout(gameLayout);
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
    // 创建Boss场景
    BossScene *bossScene = new BossScene();
    bossScene->setBossLevel(level);
    
    // 创建游戏界面容器
    QWidget *bossScreen = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(bossScreen);
    
    // 添加UI控件
    QPushButton *backButton = new QPushButton("Back to Map", bossScreen);
    connect(backButton, &QPushButton::clicked, [this]() {
        stackedWidget->setCurrentIndex(1); // 返回地图
    });
    
    layout->addWidget(backButton);
    layout->addWidget(bossScene, 1);
    
    // 创建新的堆叠页面
    stackedWidget->addWidget(bossScreen);
    stackedWidget->setCurrentWidget(bossScreen);
    
    // 连接Boss场景信号
    connect(bossScene, &BossScene::battleWon, this, [this, level]() {
        QMessageBox::information(this, "Victory!", 
                               QString("You defeated Boss Level %1!").arg(level));
        stackedWidget->setCurrentIndex(1); // 返回地图
    });
    
    connect(bossScene, &BossScene::battleLost, this, [this]() {
        QMessageBox::information(this, "Defeat", "You were defeated by the boss!");
        stackedWidget->setCurrentIndex(1); // 返回地图
    });
}