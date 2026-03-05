#include "MapScreen.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QComboBox>
#include <QMenu>
#include <algorithm>
#include <math.h>
#include <random>

using std::max;
using std::min;
using std::abs;

MapScreen::MapScreen(QWidget *parent)
    : QWidget(parent)
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
    
    // 生成起始节点
    Node startNode;
    startNode.type = SceneType::Base;
    startNode.unlocked = true;
    startNode.center = QPoint(230,230);
    mapNodes.push_back(startNode);
    
    {//临时节点供调试
        Node endNode;
        endNode.type = SceneType::Forest;
        endNode.unlocked = true;
        endNode.center = QPoint(300,300);
        mapNodes.push_back(endNode);
    }
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

        // 难度选择区域 - 使用方法2：按钮+菜单
    QHBoxLayout *levelSelectLayout = new QHBoxLayout();
    levelSelectLayout->setAlignment(Qt::AlignCenter);
    
    // 创建难度选择按钮
    levelButton = new QPushButton("选择难度", this);
    levelButton->setFixedSize(150, 35);
    levelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #444477;"
        "   color: white;"
        "   border: 2px solid #6666AA;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #555599;"
        "}"
    );
    
    // 创建菜单
    QMenu *levelMenu = new QMenu(this);
    levelMenu->setStyleSheet(
        "QMenu {"
        "   background-color: #444477;"
        "   color: white;"
        "   border: 2px solid #6666AA;"
        "}"
        "QMenu::item {"
        "   padding: 8px 20px;"
        "   font-size: 14px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #6666AA;"
        "}"
    );
    
    // 添加难度选项到菜单
    for (int i = 1; i <= maxLevel; i++) {
        QAction *action = levelMenu->addAction(QString("难度 %1").arg(i));
        action->setData(i);  // 存储难度值
        
        // 连接动作信号
        connect(action, &QAction::triggered, [this, action]() {
            int level = action->data().toInt();
            onLevelSelected(level);
        });
    }
    
    // 将菜单设置到按钮
    levelButton->setMenu(levelMenu);
    
    // 将按钮添加到水平布局
    levelSelectLayout->addWidget(levelButton);
    
    // 状态标签
    levelLabel = new QLabel("当前难度: 未选择", this);
    levelLabel->setStyleSheet("color: #AAAAFF; font-size: 14px; padding: 10px;");
    levelLabel->setAlignment(Qt::AlignCenter);
    
    // 创建水平布局放置levelLabel和难度选择布局
    QHBoxLayout *topControlsLayout = new QHBoxLayout();
    topControlsLayout->addStretch(); // 左侧弹簧
    topControlsLayout->addWidget(levelLabel);
    topControlsLayout->addLayout(levelSelectLayout);
    topControlsLayout->addStretch(); // 右侧弹簧
    
    // 地图容器
    mapWidget = new QWidget(this);
    mapWidget->setFixedSize(700, 500);

    // 添加组件到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(topControlsLayout);
    mainLayout->addWidget(mapWidget, 0, Qt::AlignCenter);
    mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    
    // 连接信号
    connect(backButton, &QPushButton::clicked, this, &MapScreen::backToMenu);
    
    // 设置初始难度
    onLevelSelected(1);
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
    int nodeSize = 40;
    
    for(auto node : mapNodes) {
        drawNode(painter, node, nodeSize);
    }

}

void MapScreen::drawNode(QPainter &painter, const Node &node, int nodeSize)
{
    QPoint center = node.center;
    QRect nodeRect(center.x() - nodeSize/2, center.y() - nodeSize/2, nodeSize, nodeSize);
    
    // 设置节点颜色和样式
    QColor nodeColor;
    QString nodeSymbol;
    
    switch (node.type) {
        case SceneType::Base:
            nodeColor = QColor(100, 200, 100); 
            nodeSymbol = "B";
            break;
        case SceneType::Forest:
            nodeColor = QColor(200, 100, 100); 
            nodeSymbol = "F";
            break;
        case SceneType::Empty:
            return; // 不绘制空节点
    }
    
    // 绘制节点背景
    if (node.unlocked) {
        // 已解锁
        painter.setBrush(QBrush(nodeColor));
        painter.setPen(QPen(nodeColor.darker(), 3));
    } else {
        // 未解锁
        painter.setBrush(QBrush(QColor(80, 80, 100)));
        painter.setPen(QPen(QColor(60, 60, 80), 2));
    }
    
    // 绘制节点圆
    painter.drawEllipse(nodeRect);

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
    int nodeSize = 40;
    
    for (int id = 0; id < mapNodes.size(); id++) {
        QPoint nodeCenter = mapNodes[id].center;
        QRect nodeRect(nodeCenter.x() - nodeSize/2, nodeCenter.y() - nodeSize/2, nodeSize, nodeSize);
            
        if (nodeRect.contains(event->pos())) {
            handleNodeClick(id);
            return;
        }
    }
}

void MapScreen::handleNodeClick(int id)
{
    if(!mapNodes[id].unlocked){
        return ;
    }
    qDebug()<<"MapScreen::handleNodeClick - id:" << id;
    // 触发节点事件
    triggerNodeEvent(mapNodes[id]);
    
    update();
}

void MapScreen::unlockNode(int id)
{
    mapNodes[id].unlocked = true;
}

void MapScreen::setGameLevel(int level)
{
    currentLevel = level;
}

void MapScreen::updateLevelLabel()
{
    QString levelText = QString("当前难度: %1").arg(currentLevel);
    
    levelLabel->setText(levelText);
}

void MapScreen::triggerNodeEvent(const Node &node)
{
    emit gameStart(node.type, currentLevel);
}

void MapScreen::onLevelSelected(int level)
{
    currentLevel = level;
    levelButton->setText("选择难度");
    levelLabel->setText(QString("当前难度: %1").arg(level));
}