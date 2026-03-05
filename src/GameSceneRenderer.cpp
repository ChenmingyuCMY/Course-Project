// src/GameSceneRenderer.cpp
#include "GameSceneRenderer.h"
#include "GameMap.h"
#include <QOpenGLShaderProgram>

GameSceneRenderer::GameSceneRenderer(QWidget *parent)
    : BaseRenderer(parent)
{
    // 初始化渲染器参数（适配BaseRenderer的相机逻辑）
    setCameraZoom(2.0f);
}

GameSceneRenderer::~GameSceneRenderer() = default;

void GameSceneRenderer::setGameMap(const std::vector<std::vector<MapCell> >* mapTiles)
{
    tiles = mapTiles;
    refreshRender(); // 设置地图后立即刷新
}

void GameSceneRenderer::initializeGL()
{
    BaseRenderer::initializeGL(); // 调用父类初始化（已创建四边形几何体+预设着色器）

    if (!hasGlobalShader("outline")) {
        qDebug()<<"outlineshader lost\n";
    }
}

void GameSceneRenderer::resizeGL(int w, int h)
{
    BaseRenderer::resizeGL(w, h); // 父类自动更新视口+投影/视图矩阵
}

void GameSceneRenderer::paintGL()
{
    BaseRenderer::paintGL(); // 父类清屏（GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT）

    if (!tiles) return;

    // 核心：仅负责渲染游戏地图
    renderGameMap();
}

void GameSceneRenderer::renderGameMap()
{
    const int tileCountX = 20; 
    const int tileCountY = 20;
    const float tileSize = 0.1f;

    QOpenGLShaderProgram* shader = getGlobalShader("outline");
    if (!shader) return;    
    // 设置边框属性
    QVector3D outlineColor(0.0f, 0.0f, 0.0f); // 黑色边框
    float outlineSize = 0.05f; // 边框宽度
    bool outlineEnabled = true;



    for (int x = 0; x < tileCountX; ++x) {
        for (int y = 0; y < tileCountY; ++y) {
            // 构建瓦片模型矩阵（位置+大小）
            QMatrix4x4 modelMatrix;
            modelMatrix.translate(x * tileSize, y * tileSize, 0.0f);
            modelMatrix.scale(tileSize);

            // 根据瓦片类型设置颜色
            QVector3D tileColor(1.0f, 1.0f, 0.0f); // 默认黄色
            TerrainType tileType = (*tiles)[x][y].terrain;
            if(tileType == TerrainType::Forest) {
                tileColor = QVector3D(0.2f, 0.8f, 0.2f); // 绿色
            } else if(tileType == TerrainType::Water) {
                tileColor = QVector3D(0.2f, 0.6f, 0.8f); // 水蓝色
            } else if(tileType == TerrainType::stone) {
                tileColor = QVector3D(0.5f, 0.5f, 0.5f); // 山灰色
            }

            // 复用BaseRenderer的渲染接口
            renderWithShader("outline", modelMatrix, [=](QOpenGLShaderProgram* shader) {
                shader->setUniformValue("color", tileColor);
                shader->setUniformValue("alpha", 1.0f); 
                shader->setUniformValue("outlineColor", outlineColor);
                shader->setUniformValue("outlineSize", outlineSize);
                shader->setUniformValue("outlineAlpha", outlineEnabled ? 1.0f : 0.0f);
            });
        }
    }

    shader->release();
}
