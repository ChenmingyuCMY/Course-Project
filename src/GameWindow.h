#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QStackedWidget>
#include "StartScreen.h"
#include "MapScreen.h"
#include "GameScreen.h"

class GameWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();
    
public slots:
    void startGame();
    void showSettings();
    void quitGame();
    void backToMenu();
    void showGameLevel(int level);
    void showBossBattle(int level);
    
private:
    void setupUI();
    
    QStackedWidget *stackedWidget;
    StartScreen *startScreen;
    MapScreen *mapScreen;
    GameScreen *gameScreen;
};

#endif // GAMEWINDOW_H