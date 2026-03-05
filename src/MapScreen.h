#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include <algorithm>
#include <math.h>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QVector>
#include <QComboBox>
#include <QMouseEvent>

#include "SceneType.h"

struct Node {
    SceneType type = SceneType::Empty;
    bool unlocked = false;
    QPoint center;
};

class MapScreen : public QWidget
{
    Q_OBJECT
    
public:
    explicit MapScreen(QWidget *parent = nullptr);
    
signals:
    void backToMenu();
    void gameStart(SceneType type, int level);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    
private:
    void setupUI();
    void initializeMap();
    void drawMap(QPainter &painter);
    void drawNode(QPainter &painter, const Node &node, int nodeSize);
    void handleNodeClick(int id);
    void unlockNode(int id);
    void updateLevelLabel();
    void triggerNodeEvent(const Node &node);
    void setGameLevel(int level);
    
    QLabel *titleLabel;
    QLabel *levelLabel;
    QPushButton *backButton;
    QPushButton *levelButton;
    QWidget *mapWidget;
    QComboBox *levelComboBox;

    QVector<Node>mapNodes;
    int currentLevel = 1;
    int lockedLevel = 1;
    const int maxLevel = 20;
    
private slots:
    void onLevelSelected(int level);
};

#endif // MAPSCREEN_H