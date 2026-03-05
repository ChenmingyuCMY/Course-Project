// src/GameScene.h
#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QWidget>
#include <QVBoxLayout>
#include <memory>
#include <QMouseEvent>

#include "SceneType.h"

#include "GameSceneRenderer.h"
#include "GameMap.h"
#include "MapFactory.h"

class GameScene : public QWidget
{
    Q_OBJECT

public:
    explicit GameScene(int level, SceneType sceneType, QWidget *parent = nullptr);
    ~GameScene() override;

    void startGame();
    void pauseGame();
    void resumeGame();
    void stopGame();

    int getLevel() const { return level; }
    SceneType getSceneType() const { return sceneType; }

signals:
    void gameFinished(bool victory);
    void sceneClosed();

protected:
    // 重写鼠标事件，接管所有交互
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    // 响应地图更新后的渲染刷新
    void onMapUpdated();

private:
    void setupUI();
    // 从地图工厂获取对应关卡的地图
    void loadMapFromFactory();
    // 屏幕坐标转地图坐标（适配渲染器相机）
    QPoint screenToMapCoord(const QPoint& screenPos);

    int level;
    SceneType sceneType;
    bool gameActive = false;
    bool isDragging = false;       // 鼠标拖拽标记
    QPoint lastMousePos;           // 上一帧鼠标位置

    GameSceneRenderer *renderer = nullptr;
    QVBoxLayout *layout = nullptr;
    GameMap* currentMap; // 当前游戏地图
};

#endif // GAMESCENE_H
