#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QStackedWidget>
#include "StartScreen.h"

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
    
private:
    void setupUI();
    
    QStackedWidget *stackedWidget;
    StartScreen *startScreen;
    QWidget *gameScreen;
};

#endif // GAMEWINDOW_H