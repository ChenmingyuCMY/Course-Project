#ifndef GAMESCENERENDERER_H
#define GAMESCENERENDERER_H

#include "BaseRenderer.h"
#include "GameMap.h"
#include <QPointer>

class GameMap;

class GameSceneRenderer : public BaseRenderer
{
    Q_OBJECT

public:
    explicit GameSceneRenderer(QWidget *parent = nullptr);
    ~GameSceneRenderer() override;

    // 设置要渲染的地图
    void setGameMap(const std::vector<std::vector<MapCell> >* mapTiles);
    // 强制刷新渲染（供GameScene调用）
    void refreshRender() { update(); }

    // 设置选中和高亮状态
    void setSelectedTile(int x, int y);
    void clearSelectedTile();
    void setHighlightedTile(int x, int y);
    void clearHighlightedTile();

protected:
    // 仅重写渲染生命周期函数，不处理交互
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    // 渲染地图的核心逻辑
    void renderGameMap();
    void renderTile(int x, int y, const MapCell& cell, bool isSelected, bool isHighlighted);

    QPoint selectedTile;      // 当前选中的格子
    QPoint highlightedTile;   // 当前高亮的格子
    bool hasSelectedTile;
    bool hasHighlightedTile;
    const std::vector<std::vector<MapCell> >* tiles; 
};

#endif // GAMESCENERENDERER_H
