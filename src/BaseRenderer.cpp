#include "BaseRenderer.h"
#include <QDebug>
#include <QOpenGLContext>
#include <QImage>
#include <QPainter>
#include <QLinearGradient>
#include <random>

// 默认四边形顶点数据
const GLfloat BaseRenderer::defaultQuadVertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};

const GLfloat BaseRenderer::defaultQuadTexCoords[] = {
    0.0f, 1.0f, 
    1.0f, 1.0f,  
    1.0f, 0.0f, 
    0.0f, 0.0f   
};

BaseRenderer::BaseRenderer(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

BaseRenderer::~BaseRenderer()
{
    makeCurrent();
    
    quadVAO.destroy();
    quadVBO.destroy();
    quadUVBO.destroy();
    
    doneCurrent();
}

bool BaseRenderer::registerGlobalShader(const QString& name,
                                       const QString& vertexSource,
                                       const QString& fragmentSource)
{
    return ShaderManager::instance()->registerShader(name, vertexSource, fragmentSource);
}

bool BaseRenderer::registerGlobalShaderFromFiles(const QString& name,
                                                const QString& vertexFile,
                                                const QString& fragmentFile)
{
    return ShaderManager::instance()->registerShaderFromFiles(name, vertexFile, fragmentFile);
}

QOpenGLShaderProgram* BaseRenderer::getGlobalShader(const QString& name)
{
    return ShaderManager::instance()->getShader(name);
}

bool BaseRenderer::hasGlobalShader(const QString& name)
{
    return ShaderManager::instance()->hasShader(name);
}

QStringList BaseRenderer::getGlobalShaderNames()
{
    return ShaderManager::instance()->getShaderNames();
}

void BaseRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    
    // 基本OpenGL状态
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 确保预设着色器已加载
    ShaderManager::instance()->loadPresetShaders();
    
    // 创建四边形几何体
    createQuadGeometry();
}

void BaseRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    viewportWidth = w;
    viewportHeight = h;
    aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    // 更新投影和视图矩阵
    updateProjectionMatrix();
    updateViewMatrix();
}

void BaseRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 由子类实现具体渲染
}

void BaseRenderer::renderWithShader(const QString& shaderName,
                                   const QMatrix4x4 &modelMatrix,
                                   const std::function<void(QOpenGLShaderProgram*)>& setupUniforms)
{
    QOpenGLShaderProgram* shader = getGlobalShader(shaderName);
    if (!shader) {
        qWarning() << "Global shader not found:" << shaderName;
        return;
    }
    
    quadVAO.bind();
    shader->bind();
    
    // 设置标准矩阵
    shader->setUniformValue("projection", projectionMatrix);
    shader->setUniformValue("view", viewMatrix);
    shader->setUniformValue("model", modelMatrix);
    
    // 调用自定义uniform设置
    if (setupUniforms) {
        setupUniforms(shader);
    }
    
    // 绘制
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    shader->release();
    quadVAO.release();
}

void BaseRenderer::renderColoredQuad(const QMatrix4x4 &modelMatrix,
                                    const QVector3D &color,
                                    float alpha,
                                    const QString& shaderName)
{
    renderWithShader(shaderName, modelMatrix, [=](QOpenGLShaderProgram* shader) {
        shader->setUniformValue("color", color);
        
        // 尝试设置alpha，如果着色器支持
        int alphaLoc = shader->uniformLocation("alpha");
        if (alphaLoc != -1) {
            shader->setUniformValue(alphaLoc, alpha);
        }
    });
}

void BaseRenderer::renderTexturedQuad(const QMatrix4x4 &modelMatrix,
                                     GLuint textureId,
                                     const QVector4D &tintColor,
                                     const QString& shaderName)
{
    renderWithShader(shaderName, modelMatrix, [=](QOpenGLShaderProgram* shader) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        shader->setUniformValue("textureSampler", 0);
        
        // 尝试设置tintColor，如果着色器支持
        int tintLoc = shader->uniformLocation("tintColor");
        if (tintLoc != -1) {
            shader->setUniformValue(tintLoc, tintColor);
        }
    });
}

void BaseRenderer::setAspectRatio(float ratio)
{
    aspectRatio = ratio;
    updateProjectionMatrix();
}

float BaseRenderer::getAspectRatio() const
{
    return aspectRatio;
}

void BaseRenderer::updateProjectionMatrix()
{
    projectionMatrix.setToIdentity();
    float viewHeight = 10.0f / cameraZoom;
    float viewWidth = viewHeight * aspectRatio;
    
    // 考虑相机位置
    float left = cameraPosition.x() - viewWidth/2;
    float right = cameraPosition.x() + viewWidth/2;
    float bottom = cameraPosition.y() - viewHeight/2;
    float top = cameraPosition.y() + viewHeight/2;
    
    projectionMatrix.ortho(left, right, bottom, top, -1.0f, 1.0f);
}

void BaseRenderer::updateViewMatrix()
{
    viewMatrix.setToIdentity();
    // 2D 相机平移
    viewMatrix.translate(-cameraPosition.x(), -cameraPosition.y(), 0.0f);

}

void BaseRenderer::setCameraPosition(const QVector2D &position)
{
    cameraPosition = position;
    updateProjectionMatrix();  // 相机移动更新投影
    update();
}

void BaseRenderer::setCameraZoom(float zoom)
{
    cameraZoom = std::max(0.1f, std::min(zoom, 10.0f));  // 限制范围
    updateProjectionMatrix();
    update();
}

void BaseRenderer::createQuadGeometry(float width, float height)
{
    quadVAO.create();
    quadVAO.bind();
    
    // 顶点数据
    GLfloat vertices[] = {
        -width/2, -height/2, 0.0f,
         width/2, -height/2, 0.0f,
         width/2,  height/2, 0.0f,
        -width/2,  height/2, 0.0f
    };
    
    GLfloat texCoords[] = {
        0.0f, 1.0f, 
        1.0f, 1.0f,  
        1.0f, 0.0f, 
        0.0f, 0.0f   
    };
    
    // 顶点缓冲区
    quadVBO.create();
    quadVBO.bind();
    quadVBO.allocate(vertices, sizeof(vertices));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    // 纹理坐标缓冲区
    quadUVBO.create();
    quadUVBO.bind();
    quadUVBO.allocate(texCoords, sizeof(texCoords));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    quadVAO.release();
}

void BaseRenderer::createTextureFromImage(const QImage &image, GLuint &textureId, 
                                         GLenum minFilter, GLenum magFilter)
{
    makeCurrent();
    
    if (textureId) {
        glDeleteTextures(1, &textureId);
    }
    
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    QImage glImage = image.convertToFormat(QImage::Format_RGBA8888);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BaseRenderer::setOrthoProjection(float left, float right, float bottom, float top,
                                     float nearPlane, float farPlane)
{
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(left, right, bottom, top, nearPlane, farPlane);
    update();
}

void BaseRenderer::setPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
{
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(fov, aspect, nearPlane, farPlane);
    update();
}

bool BaseRenderer::isKeyPressed(int key) const
{
    return pressedKeys.contains(key);
}

void BaseRenderer::keyPressEvent(QKeyEvent *event)
{
    pressedKeys.insert(event->key());
    QOpenGLWidget::keyPressEvent(event);
}

void BaseRenderer::keyReleaseEvent(QKeyEvent *event)
{
    pressedKeys.remove(event->key());
    QOpenGLWidget::keyReleaseEvent(event);
}

void BaseRenderer::mousePressEvent(QMouseEvent *event)
{
    mousePosition = QVector2D(event->pos());
    QOpenGLWidget::mousePressEvent(event);
}

void BaseRenderer::mouseMoveEvent(QMouseEvent *event)
{
    mousePosition = QVector2D(event->pos());
    QOpenGLWidget::mouseMoveEvent(event);
}