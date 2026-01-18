#include "MapScreen.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <algorithm>
#include <math.h>
#include <random>

using std::max;
using std::min;
using std::abs;

MapScreen::MapScreen(QWidget *parent)
    : QWidget(parent)
    , currentNodeRow(-1)
    , currentNodeCol(-1)
    , mapRows(7)
    , mapCols(7)
{
    // 设置背景色
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(40, 40, 60));
    setAutoFillBackground(true);
    setPalette(pal);
    
    initializeMap();
    setupUI();
}

void MapScreen::initializeMap()
{
    // 初始化地图节点
    mapNodes.clear();
    mapNodes.resize(mapRows);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> typeDist(0, 4); // 0-4种类型
    std::uniform_int_distribution<> rowDist(0, mapRows - 1);
    
    // 生成起始节点（第一列中间位置）
    int startRow = mapRows / 2;
    int startCol = mapCols / 2;
    Node startNode;
    startNode.type = NodeType::Start;
    startNode.row = startRow;
    startNode.col = startCol;
    startNode.visited = true;
    startNode.unlocked = true;
    mapNodes[startRow].resize(mapCols);
    mapNodes[startRow][startCol] = startNode;
    
    {//临时boss节点供调试、
        Node endNode;
        endNode.type = NodeType::Boss;
        endNode.row = startRow;
        endNode.col = startCol + 1;
        endNode.visited = false;
        endNode.unlocked = true;
        mapNodes[startRow][startCol + 1] = endNode;
    }

    currentNodeRow = startRow;
    currentNodeCol = startCol;
}

void MapScreen::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 标题
    titleLabel = new QLabel("ADVENTURE MAP", this);
    QFont titleFont("Arial", 36, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #FFD700; padding: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // 返回按钮
    backButton = new QPushButton("Back to Menu", this);
    backButton->setFixedSize(150, 40);
    backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #666699;"
        "   color: white;"
        "   border: 2px solid #8888CC;"
        "   border-radius: 8px;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7777AA;"
        "}"
    );
    
    // 地图容器
    mapWidget = new QWidget(this);
    mapWidget->setFixedSize(700, 500);
    
    // 状态标签
    statusLabel = new QLabel("当前位置: 起始点", this);
    statusLabel->setStyleSheet("color: #AAAAFF; font-size: 14px; padding: 10px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    
    // 添加组件到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(statusLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(mapWidget, 0, Qt::AlignCenter);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    
    // 连接信号
    connect(backButton, &QPushButton::clicked, this, &MapScreen::backToMenu);
}

void MapScreen::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制渐变背景
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(30, 30, 50));
    gradient.setColorAt(1, QColor(50, 50, 80));
    painter.fillRect(rect(), gradient);
    
    // 绘制地图区域
    drawMap(painter);
}

void MapScreen::drawMap(QPainter &painter)
{
    // 设置地图绘制区域
    int mapWidth = 600;
    int mapHeight = 400;
    int startX = (width() - mapWidth) / 2;
    int startY = 120;
    
    // 绘制地图背景
    painter.fillRect(startX, startY, mapWidth, mapHeight, QColor(30, 30, 40, 200));
    
    // 计算节点位置和大小
    int nodeSize = 20;
    int horizontalSpacing = mapWidth / (mapCols + 1);
    int verticalSpacing = mapHeight / (mapRows + 1);
    
    // 绘制连接线
    painter.setPen(QPen(QColor(100, 150, 200, 150), 3));
    
    for (int col = 0; col < mapCols - 1; col++) {
        for (int row = 0; row < mapRows; row++) {
            if (!mapNodes[row].isEmpty() && mapNodes[row][col].type != NodeType::Empty) {
                Node currentNode = mapNodes[row][col];
                
                // 连接到右侧相邻节点
                for (int nextRow = max(0, row-1); nextRow <= min(mapRows-1, row+1); nextRow++) {
                    if (col + 1 < mapCols && !mapNodes[nextRow].isEmpty() && 
                        mapNodes[nextRow][col+1].type != NodeType::Empty) {
                        
                        QPoint currentPos = getNodePosition(currentNode.row, currentNode.col,
                                                           startX, startY, 
                                                           horizontalSpacing, verticalSpacing);
                        QPoint nextPos = getNodePosition(nextRow, col+1,
                                                        startX, startY,
                                                        horizontalSpacing, verticalSpacing);
                        
                        painter.drawLine(currentPos, nextPos);
                    }
                }
            }
        }
    }
    
    // 绘制所有节点
    for (int row = 0; row < mapRows; row++) {
        for (int col = 0; col < mapCols; col++) {
            if (!mapNodes[row].isEmpty() && mapNodes[row][col].type != NodeType::Empty) {
                drawNode(painter, mapNodes[row][col], startX, startY, 
                        horizontalSpacing, verticalSpacing, nodeSize);
            }
        }
    }
}

QPoint MapScreen::getNodePosition(int row, int col, int startX, int startY, 
                                 int hSpacing, int vSpacing)
{
    static std::map<std::pair<int, int>, QPoint> positionCache;
    auto key = std::make_pair(row, col);
    
    if (positionCache.find(key) == positionCache.end()) {
        int baseX = startX + (col + 1) * hSpacing;
        int baseY = startY + (row + 1) * vSpacing;
        positionCache[key] = QPoint(baseX, baseY);
    }
    
    return positionCache[key];
}

void MapScreen::drawNode(QPainter &painter, const Node &node, int startX, int startY,
                        int hSpacing, int vSpacing, int nodeSize)
{
    QPoint center = getNodePosition(node.row, node.col, startX, startY, hSpacing, vSpacing);
    QRect nodeRect(center.x() - nodeSize/2, center.y() - nodeSize/2, nodeSize, nodeSize);
    
    // 设置节点颜色和样式
    QColor nodeColor;
    QString nodeSymbol;
    
    switch (node.type) {
        case NodeType::Start:
            nodeColor = QColor(100, 200, 100); // 绿色
            nodeSymbol = "S";
            break;
        case NodeType::Combat:
            nodeColor = QColor(200, 100, 100); // 红色
            nodeSymbol = "⚔";
            break;
        case NodeType::Shop:
            nodeColor = QColor(100, 100, 200); // 蓝色
            nodeSymbol = "$";
            break;
        case NodeType::Tavern:
            nodeColor = QColor(200, 200, 100); // 黄色
            nodeSymbol = "🍺";
            break;
        case NodeType::Event:
            nodeColor = QColor(200, 100, 200); // 紫色
            nodeSymbol = "?";
            break;
        case NodeType::Treasure:
            nodeColor = QColor(255, 215, 0); // 金色
            nodeSymbol = "💎";
            break;
        case NodeType::Boss:
            nodeColor = QColor(150, 50, 50); // 暗红色
            nodeSymbol = "👑";
            break;
        case NodeType::Empty:
            return; // 不绘制空节点
    }
    
    // 绘制节点背景
    if (node.visited) {
        // 已访问的节点
        painter.setBrush(QBrush(nodeColor.lighter(130)));
        painter.setPen(QPen(nodeColor.darker(150), 3));
    } else if (node.unlocked) {
        // 已解锁但未访问
        painter.setBrush(QBrush(nodeColor));
        painter.setPen(QPen(nodeColor.darker(), 3));
    } else {
        // 未解锁
        painter.setBrush(QBrush(QColor(80, 80, 100)));
        painter.setPen(QPen(QColor(60, 60, 80), 2));
    }
    
    // 绘制节点圆
    painter.drawEllipse(nodeRect);
    
    // 绘制当前节点高亮
    if (node.row == currentNodeRow && node.col == currentNodeCol) {
        painter.setPen(QPen(QColor(255, 255, 100), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(nodeRect.adjusted(-5, -5, 5, 5));
    }
    
    // 绘制节点符号
    painter.setPen(QPen(Qt::white, 2));
    QFont symbolFont("Arial", 12, QFont::Bold);
    painter.setFont(symbolFont);
    painter.drawText(nodeRect, Qt::AlignCenter, nodeSymbol);
}

void MapScreen::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    
    // 检查是否点击了节点
    int startX = (width() - 600) / 2;
    int startY = 120;
    int nodeSize = 40;
    int horizontalSpacing = 600 / (mapCols + 1);
    int verticalSpacing = 400 / (mapRows + 1);
    
    for (int row = 0; row < mapRows; row++) {
        for (int col = 0; col < mapCols; col++) {
            if (!mapNodes[row].isEmpty() && mapNodes[row][col].type != NodeType::Empty) {
                QPoint nodeCenter = getNodePosition(row, col, startX, startY, 
                                                   horizontalSpacing, verticalSpacing);
                QRect nodeRect(nodeCenter.x() - nodeSize/2, nodeCenter.y() - nodeSize/2,
                              nodeSize, nodeSize);
                
                if (nodeRect.contains(event->pos())) {
                    handleNodeClick(row, col);
                    return;
                }
            }
        }
    }
}

void MapScreen::handleNodeClick(int row, int col)
{
    if (mapNodes[row][col].unlocked && !mapNodes[row][col].visited) {
        // 检查是否是相邻节点
        if (abs(row - currentNodeRow) + abs(col - currentNodeCol) == 1) {
            // 移动到新节点
            mapNodes[currentNodeRow][currentNodeCol].visited = true;
            currentNodeRow = row;
            currentNodeCol = col;
            mapNodes[row][col].visited = true;
            
            // 解锁相邻节点
            unlockAdjacentNodes(row, col);
            
            // 更新状态显示
            updateStatusLabel();
            
            // 触发节点事件
            triggerNodeEvent(mapNodes[row][col]);
            
            // 重绘地图
            update();
        }
    }
}

void MapScreen::unlockAdjacentNodes(int row, int col)
{
    // 解锁上下左右四个方向的节点
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (auto &dir : directions) {
        int newRow = row + dir[0];
        int newCol = col + dir[1];
        
        if (newRow >= 0 && newRow < mapRows && 
            newCol >= 0 && newCol < mapCols &&
            !mapNodes[newRow].isEmpty() && 
            mapNodes[newRow][newCol].type != NodeType::Empty) {
            
            mapNodes[newRow][newCol].unlocked = true;
        }
    }
}

void MapScreen::updateStatusLabel()
{
    QString statusText = "当前位置: ";
    
    switch (mapNodes[currentNodeRow][currentNodeCol].type) {
        case NodeType::Start: statusText += "起始点"; break;
        case NodeType::Combat: statusText += "战斗关卡"; break;
        case NodeType::Shop: statusText += "商店"; break;
        case NodeType::Tavern: statusText += "酒馆"; break;
        case NodeType::Event: statusText += "随机事件"; break;
        case NodeType::Treasure: statusText += "宝藏房间"; break;
        case NodeType::Boss: statusText += "Boss战"; break;
    }
    
    statusLabel->setText(statusText);
}

void MapScreen::triggerNodeEvent(const Node &node)
{
    switch (node.type) {
        case NodeType::Combat:
            emit combatLevelSelected(node.row * mapCols + node.col + 1);
            break;
        case NodeType::Shop:
            emit shopEntered();
            break;
        case NodeType::Tavern:
            emit tavernEntered();
            break;
        case NodeType::Event:
            emit randomEventTriggered();
            break;
        case NodeType::Treasure:
            emit treasureFound();
            break;
        case NodeType::Boss:
            emit bossBattleStarted(1);
            break;
        default:
            break;
    }
}

void MapScreen::resetMap()
{
    initializeMap();
    updateStatusLabel();
    update();
}