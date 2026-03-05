// src/GameScene.cpp
#include "GameScene.h"
#include "GameSceneRenderer.h"
#include "MapFactory.h"
#include "GameMap.h"
#include <QMouseEvent>
#include <QDebug>

GameScene::GameScene(int level, SceneType sceneType, QWidget *parent)
    : QWidget(parent)
    , level(level)
    , sceneType(sceneType)
{
    setupUI();
    loadMapFromFactory(); // 从MapFactory加载地图
}

GameScene::~GameScene()
{
    stopGame();
    emit sceneClosed();
    
    // 确保渲染器被正确清理
    if (renderer) {
        renderer->setEnabled(false);
    }
}


void GameScene::startGame()
{
    gameActive = true;
    if (renderer) {
        renderer->setEnabled(true);
        renderer->refreshRender(); // 启动游戏时刷新渲染
    }
}

void GameScene::pauseGame()
{
    gameActive = false;
    if (renderer) {
        renderer->setEnabled(false);
    }
}

void GameScene::resumeGame()
{
    gameActive = true;
    if (renderer) {
        renderer->setEnabled(true);
        renderer->refreshRender();
    }
}

void GameScene::stopGame()
{
    gameActive = false;
    if (renderer) {
        renderer->setEnabled(false);
    }
}

void GameScene::setupUI()
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建渲染器并挂载到UI
    renderer = new GameSceneRenderer(this);
    layout->addWidget(renderer);

    setLayout(layout);
    setFocusPolicy(Qt::StrongFocus); // 确保能接收鼠标/键盘事件
}

void GameScene::loadMapFromFactory()
{
    // 从MapFactory获取对应关卡的地图（使用实际工厂接口）
    currentMap = MapFactory::createMap(sceneType, level);
    
    // 将地图绑定到渲染器
    if (renderer && currentMap) {
        renderer->setGameMap(currentMap->getTiles());
    }
}

void GameScene::onMapUpdated()
{
    // 地图数据更新后，通知渲染器刷新
    if (renderer) {
        renderer->refreshRender();
    }
    
    // 游戏胜利/失败判定逻辑
    // bool victory = currentMap->checkCompletion();
    // if (victory) {
    //     emit gameFinished(true);
    //     stopGame();
    // }
}

// 鼠标按下：处理地图修改/拖拽开始
void GameScene::mousePressEvent(QMouseEvent *event)
{
    if (!gameActive || !currentMap) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        // 左键开始拖拽相机
        isDragging = true;
        lastMousePos = event->pos();
    } else if (event->button() == Qt::RightButton) {
        // 右键修改地图瓦片
        QPoint mapCoord = screenToMapCoord(event->pos());
        // currentMap->updateTile(mapCoord.x(), mapCoord.y(), 1); // 修改瓦片类型
        onMapUpdated(); // 触发地图更新→渲染刷新
    }

    QWidget::mousePressEvent(event);
}

// 鼠标移动：处理相机拖拽
void GameScene::mouseMoveEvent(QMouseEvent *event)
{
    if (!gameActive || !isDragging || !renderer) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    // 计算鼠标偏移，更新渲染器相机位置
    QPoint delta = event->pos() - lastMousePos;
    QVector2D cameraPos = renderer->getCameraPosition();
    cameraPos -= QVector2D(delta.x() * 0.02f, -delta.y() * 0.02f) * (1.0f / renderer->getCameraZoom());
    renderer->setCameraPosition(cameraPos); // 调用BaseRenderer的相机接口
    lastMousePos = event->pos();

    QWidget::mouseMoveEvent(event);
}

// 鼠标释放：结束拖拽
void GameScene::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

// 屏幕坐标转地图坐标（适配渲染器的相机/缩放）
QPoint GameScene::screenToMapCoord(const QPoint& screenPos)
{
    if (!renderer) return QPoint();

    // 适配BaseRenderer的正交投影逻辑
    float zoom = renderer->getCameraZoom();
    QVector2D cameraPos = renderer->getCameraPosition();
    int viewW = renderer->getViewportWidth();
    int viewH = renderer->getViewportHeight();

    // 屏幕坐标 → 世界坐标 → 地图坐标
    float worldX = (screenPos.x() - viewW/2) / zoom + cameraPos.x();
    float worldY = -(screenPos.y() - viewH/2) / zoom + cameraPos.y();

    return QPoint(static_cast<int>(worldX), static_cast<int>(worldY));
}
