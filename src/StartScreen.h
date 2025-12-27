#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class StartScreen : public QWidget
{
    Q_OBJECT
    
public:
    explicit StartScreen(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event);
signals:
    void startClicked();
    void settingsClicked();
    void quitClicked();
    
private:
    void setupUI();
    
    QPushButton *startButton;
    QPushButton *settingsButton;
    QPushButton *quitButton;
    QLabel *titleLabel;
};

#endif // STARTSCREEN_H