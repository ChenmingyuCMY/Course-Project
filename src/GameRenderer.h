#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>

class GameRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
    
public:
    explicit GameRenderer(QWidget *parent = nullptr);
    ~GameRenderer();
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
private:
    void initShaders();
    void initGeometry();
    
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QMatrix4x4 projection;
};

#endif // GAMERENDERER_H