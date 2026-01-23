#include "GameScreen.h"
#include <QPainter>
#include <QStyleOption>
#include <QMessageBox>
#include <QApplication>
#include <QFont>
#include <QFontDatabase>

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent)
    , currentSceneType(GameSceneType::NONE)
    , gamePaused(false)
    , currentLevel(1)
    , playerHealth(100.0f)
    , playerMaxHealth(100.0f)
    , playerScore(0)
    , playerCoins(0)
    , bossScene(nullptr)
    // , combatScene(nullptr)
    // , shopScene(nullptr)
    , currentSceneWidget(nullptr)
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
        if (currentSceneType != GameSceneType::NONE) {
            loadScene(currentSceneType, currentLevel);
        }
    });
    
    hudLayout->addWidget(pauseButton);
    hudLayout->addWidget(menuButton);
    hudLayout->addWidget(retryButton);
    
    // 场景堆栈
    sceneStack = new QStackedWidget(this);
    sceneStack->setObjectName("sceneStack");
    
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
    // 可以添加场景预加载等逻辑
}

void GameScreen::loadScene(GameSceneType sceneType, int level)
{
    // 卸载之前场景
    unloadCurrentScene();
    
    currentSceneType = sceneType;
    currentLevel = level;
    
    switch (sceneType) {
    case GameSceneType::COMBAT:
        break;
        
    case GameSceneType::BOSS_BATTLE:
        createBossScene(level);
        sceneLabel->setText(QString("Scene: Boss Battle (Level %1)").arg(level));
        break;
    case GameSceneType::SHOP:
        break;
        
    case GameSceneType::MENU:
        // 可以加载菜单场景
        sceneLabel->setText("Scene: Menu");
        break;
        
    default:
        sceneLabel->setText("Scene: None");
        return;
    }
    
    // 显示场景
    if (currentSceneWidget) {
        sceneStack->setCurrentWidget(currentSceneWidget);
        emit sceneChanged(sceneType);
    }
    
    // 恢复游戏状态
    if (gamePaused) {
        resumeGame();
    }
}

void GameScreen::unloadCurrentScene()
{
    if (!currentSceneWidget) return;
    
    // 根据场景类型进行清理
    switch (currentSceneType) {
    case GameSceneType::BOSS_BATTLE:
        if (bossScene) {
            // 断开所有连接
            bossScene->disconnect();
            // 停止更新
            bossScene->setVisible(false);
        }
        break;
    case GameSceneType::SHOP:

        break;
    }
    
    // 从堆栈中移除
    sceneStack->removeWidget(currentSceneWidget);
    currentSceneWidget = nullptr;
}

void GameScreen::createBossScene(int level)
{
    if (!bossScene) {
        bossScene = new BossScene(this);
        
        // 连接Boss场景信号
        connect(bossScene, &BossScene::battleWon, this, [this, level]() {
            int reward = level * 100;
            playerScore += reward;
            playerCoins += reward / 5;
            
            emit playerScoreChanged(playerScore);
            emit playerCoinsChanged(playerCoins);
            
            QMessageBox::information(this, "Victory!", 
                QString("Boss Defeated!\nReward: %1 points, %2 coins\nNew Level: %3")
                .arg(reward).arg(reward / 5).arg(currentLevel));
            
            emit gameOver(true);
        });
        
        connect(bossScene, &BossScene::battleLost, this, [this]() {
            playerHealth -= 30.0f; // Boss战失败扣除更多生命值
            if (playerHealth <= 0) {
                playerHealth = 0;
                onGameOver(false);
            } else {
                QMessageBox::information(this, "Defeat", 
                    "You were defeated by the boss!");
            }
            emit playerHealthChanged(playerHealth);
        });
        
        // connect(bossScene, &BossScene::playerHealthUpdated, this, 
        //         [this](float health) {
        //     playerHealth = health;
        //     emit playerHealthChanged(playerHealth);
        // });
        
        // connect(bossScene, &BossScene::scoreUpdated, this, 
        //         [this](int score) {
        //     playerScore += score;
        //     emit playerScoreChanged(playerScore);
        // });
    }
    
    bossScene->setBossLevel(level);
    currentSceneWidget = bossScene;
    sceneStack->addWidget(bossScene);
}

void GameScreen::pauseGame()
{
    gamePaused = true;
    
    // 暂停当前场景
    switch (currentSceneType) {
    case GameSceneType::BOSS_BATTLE:
        if (bossScene) bossScene->setUpdatesEnabled(false);
        break;
    case GameSceneType::COMBAT:
        // if (combatScene) combatScene->setUpdatesEnabled(false);
        break;
    }
    
    emit gamePausedChanged(true);
}

void GameScreen::resumeGame()
{
    gamePaused = false;
    
    // 恢复当前场景
    switch (currentSceneType) {
    case GameSceneType::BOSS_BATTLE:
        if (bossScene) bossScene->setUpdatesEnabled(true);
        break;
    case GameSceneType::COMBAT:

        break;
    case GameSceneType::SHOP:
        
        break;
    }
    
    emit gamePausedChanged(false);
}

void GameScreen::setPlayerHealth(float health)
{
    playerHealth = qBound(0.0f, health, playerMaxHealth);
    emit playerHealthChanged(playerHealth);
}

void GameScreen::setPlayerScore(int score)
{
    playerScore = score;
    emit playerScoreChanged(playerScore);
}

void GameScreen::setPlayerCoins(int coins)
{
    playerCoins = coins;
    emit playerCoinsChanged(playerCoins);
}

void GameScreen::updateHUD()
{
    if (!gamePaused) {
        // 更新HUD显示
        healthLabel->setText(QString("Health: %1/%2")
            .arg(static_cast<int>(playerHealth))
            .arg(static_cast<int>(playerMaxHealth)));
        
        scoreLabel->setText(QString("Score: %1").arg(playerScore));
        coinsLabel->setText(QString("Coins: %1").arg(playerCoins));
    }
}

void GameScreen::onGameOver(bool victory)
{
    pauseGame();
    
    QString message = victory ? 
        QString("Victory!\nFinal Score: %1\nCoins: %2").arg(playerScore).arg(playerCoins) :
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
        loadScene(currentSceneType, currentLevel);
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

void GameScreen::onSceneChangeRequested(GameSceneType newScene, int level)
{
    loadScene(newScene, level);
}