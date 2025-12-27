#include "StartScreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPalette>
#include <QSpacerItem>
#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>

StartScreen::StartScreen(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    
    // 设置背景色
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(30, 30, 60));
    setAutoFillBackground(true);
    setPalette(pal);
}

void StartScreen::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 添加顶部弹簧
    mainLayout->addStretch(2);
    
    // 游戏标题
    titleLabel = new QLabel("2D ADVENTURE", this);
    QFont titleFont("Arial", 48, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #FFD700;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch(1);
    
    // 按钮容器
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(20);
    buttonLayout->setAlignment(Qt::AlignCenter);
    
    // 开始按钮
    startButton = new QPushButton("START GAME", this);
    startButton->setFixedSize(200, 50);
    startButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 10px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"
        "}"
    );
    
    // 设置按钮
    settingsButton = new QPushButton("SETTINGS", this);
    settingsButton->setFixedSize(200, 50);
    settingsButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #2196F3;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 10px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0b7dda;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0a6ebd;"
        "}"
    );
    
    // 退出按钮
    quitButton = new QPushButton("QUIT", this);
    quitButton->setFixedSize(200, 50);
    quitButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 10px;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #da190b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #bd0a00;"
        "}"
    );
    
    // 添加按钮到布局
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(settingsButton);
    buttonLayout->addWidget(quitButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 添加底部弹簧
    mainLayout->addStretch(3);
    
    // 版本标签
    QLabel *versionLabel = new QLabel("Version 1.0.0", this);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("color: #888888;");
    mainLayout->addWidget(versionLabel);
    
    setLayout(mainLayout);
    
    // 连接按钮信号
    connect(startButton, &QPushButton::clicked, this, &StartScreen::startClicked);
    connect(settingsButton, &QPushButton::clicked, this, &StartScreen::settingsClicked);
    connect(quitButton, &QPushButton::clicked, this, &StartScreen::quitClicked);
}

void StartScreen::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    // 绘制背景装饰
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制渐变背景
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(20, 20, 40));
    gradient.setColorAt(1, QColor(40, 40, 80));
    painter.fillRect(rect(), gradient);
    
    // 绘制装饰性星星
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
    for (int i = 0; i < 50; ++i) {
        int x = rand() % width();
        int y = rand() % height();
        painter.drawPoint(x, y);
    }
}