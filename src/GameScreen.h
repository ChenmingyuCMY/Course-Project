#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "BossScene.h"
// #include "CombatScene.h" 
// #include "ShopScene.h"  

enum class GameSceneType {
    NONE,
    COMBAT,
    PLATFORMER,
    BOSS_BATTLE,
    SHOP,
    MENU
};

class GameScreen : public QWidget
{
    Q_OBJECT
    
public:
    explicit GameScreen(QWidget *parent = nullptr);
    ~GameScreen();
    
    // 场景控制方法
    void loadScene(GameSceneType sceneType, int level = 1);
    void unloadCurrentScene();
    void pauseGame();
    void resumeGame();
    
    // 游戏状态
    bool isGamePaused() const { return gamePaused; }
    GameSceneType getCurrentSceneType() const { return currentSceneType; }
    
    // 玩家数据
    void setPlayerHealth(float health);
    void setPlayerScore(int score);
    void setPlayerCoins(int coins);
    
    float getPlayerHealth() const { return playerHealth; }
    int getPlayerScore() const { return playerScore; }
    int getPlayerCoins() const { return playerCoins; }
    
signals:
    void gamePausedChanged(bool paused);
    void sceneChanged(GameSceneType newScene);
    void playerHealthChanged(float health);
    void playerScoreChanged(int score);
    void playerCoinsChanged(int coins);
    void gameOver(bool victory);
    void returnToMapRequested();
    
public slots:
    void onGameOver(bool victory);
    void onReturnToMap();
    
private slots:
    void updateHUD();
    void onSceneChangeRequested(GameSceneType newScene, int level = 1);
    
private:
    void setupUI();
    void setupHUD();
    void setupControlPanel();
    void setupSceneManager();
    // void createCombatScene(int level);
    void createBossScene(int level);
    // void createShopScene(int level);
    
    // UI元素
    QWidget *hudWidget;
    QLabel *healthLabel;
    QLabel *scoreLabel;
    QLabel *coinsLabel;
    QLabel *sceneLabel;
    QPushButton *pauseButton;
    QPushButton *menuButton;
    QPushButton *retryButton;
    
    QWidget *controlPanel;
    QStackedWidget *sceneStack;
    
    // 游戏场景
    BossScene *bossScene;
    // CombatScene *combatScene;
    // ShopScene *shopScene;
    QWidget *currentSceneWidget;
    
    // 游戏状态
    GameSceneType currentSceneType;
    bool gamePaused;
    int currentLevel;
    
    // 玩家数据
    float playerHealth;
    float playerMaxHealth;
    int playerScore;
    int playerCoins;
    
    // 定时器
    QTimer *hudUpdateTimer;
};

#endif // GAMESCREEN_H