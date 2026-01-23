#ifndef BASERENDERER_H
#define BASERENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>

#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

#include <QTimer>
#include <QElapsedTimer>

#include <QKeyEvent> 
#include <QMouseEvent>

#include <QSet>
#include <functional>
#include "ShaderManager.h"

class BaseRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    
public:
    explicit BaseRenderer(QWidget *parent = nullptr);
    virtual ~BaseRenderer();

    // 全局着色器管理
    static bool registerGlobalShader(const QString& name,
                                    const QString& vertexSource,
                                    const QString& fragmentSource);
    
    static bool registerGlobalShaderFromFiles(const QString& name,
                                             const QString& vertexFile,
                                             const QString& fragmentFile);
    
    static QOpenGLShaderProgram* getGlobalShader(const QString& name);
    static bool hasGlobalShader(const QString& name);
    static QStringList getGlobalShaderNames();

    // 核心渲染方法
    void renderWithShader(const QString& shaderName,
                         const QMatrix4x4 &modelMatrix,
                         const std::function<void(QOpenGLShaderProgram*)>& setupUniforms = nullptr);
    
    // 快捷渲染方法（基于renderWithShader）
    void renderColoredQuad(const QMatrix4x4 &modelMatrix,
                          const QVector3D &color = QVector3D(1.0f, 1.0f, 1.0f),
                          float alpha = 1.0f,
                          const QString& shaderName = "simple");
    
    void renderTexturedQuad(const QMatrix4x4 &modelMatrix,
                           GLuint textureId,
                           const QVector4D &tintColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f),
                           const QString& shaderName = "texture");
                           
    // 投影设置函数
    void setOrthoProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void setPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane);
    
    // 缩放比
    void setAspectRatio(float ratio);
    float getAspectRatio() const;
    
    // 相机控制接口
    void setCameraPosition(const QVector2D &position);
    void setCameraZoom(float zoom);
    QVector2D getCameraPosition() const { return cameraPosition; }
    float getCameraZoom() const { return cameraZoom; }

    // 公共工具方法
    void createQuadGeometry(float width = 1.0f, float height = 1.0f);
    void createTextureFromImage(const QImage &image, GLuint &textureId, 
                               GLenum minFilter = GL_LINEAR, GLenum magFilter = GL_LINEAR);
    
    // 输入状态
    bool isKeyPressed(int key) const;
    
protected:
    // 虚函数
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
    
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    virtual void updateViewMatrix();
    virtual void updateProjectionMatrix();
    
    // 工具方法
    QOpenGLVertexArrayObject* getQuadVAO() { return &quadVAO; }
    QMatrix4x4 getProjectionMatrix() const { return projectionMatrix; }
    QMatrix4x4 getViewMatrix() const { return viewMatrix; }

    // 相机参数
    QVector2D cameraPosition = QVector2D(0.0f, 0.0f);
    float cameraZoom = 1.0f;
    
    // 投影和视图矩阵
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;
    float aspectRatio = 1.0f;

    // 输入状态
    QSet<int> pressedKeys;
    QVector2D mousePosition;
    
private:
    // 几何体
    QOpenGLVertexArrayObject quadVAO;
    QOpenGLBuffer quadVBO;
    QOpenGLBuffer quadUVBO;
    
    // 视口参数
    int viewportWidth = 0;
    int viewportHeight = 0;

    // 默认顶点数据
    static const GLfloat defaultQuadVertices[12];
    static const GLfloat defaultQuadTexCoords[8];
};

#endif // BASERENDERER_H