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
#include <QMouseEvent>

enum class NodeType {
    Empty,
    Start,
    Combat,
    Shop,
    Tavern,
    Event,
    Treasure,
    Boss
};

struct Node {
    NodeType type = NodeType::Empty;
    int row = -1;
    int col = -1;
    bool visited = false;
    bool unlocked = false;
};

class MapScreen : public QWidget
{
    Q_OBJECT
    
public:
    explicit MapScreen(QWidget *parent = nullptr);
    
signals:
    void backToMenu();
    void levelSelected(int level);
    void combatLevelSelected(int level);
    void shopEntered();
    void tavernEntered();
    void randomEventTriggered();
    void treasureFound();
    void bossBattleStarted(int level);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    
private:
    void setupUI();
    void initializeMap();
    void drawMap(QPainter &painter);
    void drawNode(QPainter &painter, const Node &node, int startX, int startY,
                  int hSpacing, int vSpacing, int nodeSize);
    QPoint getNodePosition(int row, int col, int startX, int startY,
                          int hSpacing, int vSpacing);
    void handleNodeClick(int row, int col);
    void unlockAdjacentNodes(int row, int col);
    void updateStatusLabel();
    void triggerNodeEvent(const Node &node);
    
    QLabel *titleLabel;
    QLabel *statusLabel;
    QPushButton *backButton;
    QWidget *mapWidget;
    
    QVector<QVector<Node>> mapNodes;
    int currentNodeRow;
    int currentNodeCol;
    int mapRows;
    int mapCols;
    
public slots:
    void resetMap();
};

#endif // MAPSCREEN_H