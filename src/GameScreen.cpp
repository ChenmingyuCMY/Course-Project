#include "GameScreen.h"
#include <QPainter>
#include <QStyleOption>
#include <QMessageBox>
#include <QApplication>
#include <QFont>
#include <QFontDatabase>

#include "MapFactory.h"
#include "GameMap.h"

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent)
    , currentSceneType(SceneType::Empty)
    , gamePaused(false)
    , currentLevel(1)
    , playerHealth(100.0f)
    , playerMaxHealth(100.0f)
    , playerScore(0)
    , playerCoins(0)
    , currentGameScene(nullptr)
{
    setupUI();
    setupHUD();
    setupControlPanel();
    setupSceneManager();
    
    // 初始化HUD更新定时器
    hudUpdateTimer = new QTimer(this);
    
    connect(hudUpdateTimer, &QTimer::timeout, this, &GameScreen::updateHUD);
    hudUpdateTimer->start(100); // 每100ms更新一次HUD
}

GameScreen::~GameScreen()
{
    // 清理场景
    unloadCurrentScene();
    
    if (hudUpdateTimer) {
        hudUpdateTimer->stop();
        hudUpdateTimer->deleteLater();
    }
}

void GameScreen::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // HUD区域 (顶部)
    hudWidget = new QWidget(this);
    hudWidget->setObjectName("hudWidget");
    hudWidget->setStyleSheet(
        "QWidget#hudWidget {"
        "background-color: rgba(0, 0, 0, 180);"
        "border-bottom: 2px solid #8B4513;"
        "}"
        "QLabel {"
        "color: white;"
        "font-weight: bold;"
        "padding: 5px;"
        "}"
        "QPushButton {"
        "background-color: #8B4513;"
        "color: white;"
        "border: 2px solid #A0522D;"
        "border-radius: 5px;"
        "padding: 5px 10px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #A0522D;"
        "}"
    );
    
    QHBoxLayout *hudLayout = new QHBoxLayout(hudWidget);
    
    // 玩家状态
    healthLabel = new QLabel("Health: 100/100", hudWidget);
    scoreLabel = new QLabel("Score: 0", hudWidget);
    coinsLabel = new QLabel("Coins: 0", hudWidget);
    sceneLabel = new QLabel("Scene: None", hudWidget);
    
    hudLayout->addWidget(healthLabel);
    hudLayout->addWidget(scoreLabel);
    hudLayout->addWidget(coinsLabel);
    hudLayout->addStretch();
    hudLayout->addWidget(sceneLabel);
    hudLayout->addStretch();
    
    // 控制按钮
    pauseButton = new QPushButton("Pause", hudWidget);
    menuButton = new QPushButton("Menu", hudWidget);
    retryButton = new QPushButton("Retry", hudWidget);
    
    connect(pauseButton, &QPushButton::clicked, [this]() {
        if (gamePaused) {
            resumeGame();
            pauseButton->setText("Pause");
        } else {
            pauseGame();
            pauseButton->setText("Resume");
        }
    });
    
    connect(menuButton, &QPushButton::clicked, this, &GameScreen::onReturnToMap);
    connect(retryButton, &QPushButton::clicked, [this]() {
        if (currentSceneType != SceneType::Empty) {
            loadScene(currentSceneType, currentLevel);
        }
    });
    
    hudLayout->addWidget(pauseButton);
    hudLayout->addWidget(menuButton);
    hudLayout->addWidget(retryButton);
    
    // 场景堆栈
    sceneStack = new QStackedWidget(this);
    sceneStack->setObjectName("sceneStack");
    sceneStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 默认的空页面
    QWidget* emptyWidget = new QWidget();
    sceneStack->addWidget(emptyWidget);

    // 控制面板 (底部)
    controlPanel = new QWidget(this);
    controlPanel->setObjectName("controlPanel");
    controlPanel->setStyleSheet(
        "QWidget#controlPanel {"
        "background-color: rgba(0, 0, 0, 150);"
        "border-top: 2px solid #8B4513;"
        "}"
    );
    
    // 组装主布局
    mainLayout->addWidget(hudWidget, 0);  // HUD
    mainLayout->addWidget(sceneStack, 1);  // 游戏场景
    mainLayout->addWidget(controlPanel, 0);  // 控制面板
    
    setLayout(mainLayout);
}

void GameScreen::setupHUD()
{
    // 自定义字体
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(10);
    font.setBold(true);
    
    healthLabel->setFont(font);
    scoreLabel->setFont(font);
    coinsLabel->setFont(font);
    sceneLabel->setFont(font);
}

void GameScreen::setupControlPanel()
{
    // 控制面板设置
}

void GameScreen::setupSceneManager()
{
    // 初始化场景管理器
}

void GameScreen::loadScene(SceneType sceneType, int level)
{
    currentSceneType = sceneType;
    currentLevel = level;
    
    switch (sceneType) {
    case SceneType::Empty:
        break;
    default:
        sceneLabel->setText("Scene: None");
        return;
    }
    
    // 显示场景
    if (currentGameScene) {
        sceneStack->setCurrentWidget(currentGameScene);
        emit sceneChanged(sceneType);
    }
    
    // 恢复游戏状态
    if (gamePaused) {
        resumeGame();
    }
}

void GameScreen::unloadCurrentScene()
{
    if (!currentGameScene) return;
    
    // 断开所有连接
    currentGameScene->disconnect();
    
    // 从堆栈中移除并删除
    sceneStack->removeWidget(currentGameScene);
    currentGameScene->deleteLater();
    currentGameScene = nullptr;
    
    // 清空场景类型
    currentSceneType = SceneType::Empty;
}

void GameScreen::createScene(int level, SceneType sceneType)
{
    
}

void GameScreen::pauseGame()
{
    gamePaused = true;
    
    emit gamePausedChanged(true);
}

void GameScreen::resumeGame()
{
    gamePaused = false;
    
    emit gamePausedChanged(false);
}

void GameScreen::updateHUD()
{
    if (!gamePaused) {
        // 更新HUD显示
        // stoneLabel->setText();
        // woodLabel->setText();
        // foodLabel->setText();
    }
}
void GameScreen::setCurrentLevel(int level)
{
    currentLevel = level;
}

void GameScreen::onGameStart(SceneType type, int level)
{
    qDebug()<<"GameScreen::onGameStart - type:" << static_cast<int>(type) << " level:" << level;
    setCurrentLevel(level);
    unloadCurrentScene();

    // 创建新场景并挂载
    currentGameScene = new GameScene(level, type, this);
    loadScene(type, level);

    sceneStack->addWidget(currentGameScene);
    sceneStack->setCurrentWidget(currentGameScene);
    

    // 启动游戏
    currentGameScene->startGame();

    // 连接游戏结束信号
    connect(currentGameScene, &GameScene::gameFinished, this, [=](bool victory) {
        qDebug() << "Game finished, victory:" << victory;
        // 处理结算逻辑
    });
}

void GameScreen::onGameOver(bool victory)
{
    pauseGame();
    
    QString message = victory ? 
        QString("Victory!") :
        "Game Over!\nYou have been defeated.";
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(victory ? "Victory!" : "Game Over");
    msgBox.setText(message);
    msgBox.setIcon(victory ? QMessageBox::Information : QMessageBox::Critical);
    
    QPushButton *retryButton = msgBox.addButton("Retry", QMessageBox::ActionRole);
    QPushButton *menuButton = msgBox.addButton("Return to Map", QMessageBox::ActionRole);
    QPushButton *quitButton = msgBox.addButton("Quit", QMessageBox::RejectRole);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == retryButton) {
        resumeGame();
    } else if (msgBox.clickedButton() == menuButton) {
        onReturnToMap();
    } else if (msgBox.clickedButton() == quitButton) {
        QApplication::quit();
    }
}

void GameScreen::onReturnToMap()
{
    unloadCurrentScene();
    emit returnToMapRequested();
}
