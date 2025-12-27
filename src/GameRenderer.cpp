#include "GameRenderer.h"
#include <QFile>
#include <QDebug>
#include <QCoreApplication>

GameRenderer::GameRenderer(QWidget *parent)
    : QOpenGLWidget(parent)
    , program(nullptr)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
}

GameRenderer::~GameRenderer()
{
    makeCurrent();
    vbo.destroy();
    vao.destroy();
    delete program;
    doneCurrent();
}

void GameRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    initShaders();
    initGeometry();
}

void GameRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    
    // 设置正交投影矩阵
    projection.setToIdentity();
    projection.ortho(0, w, h, 0, -1, 1);
}

void GameRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    program->bind();
    program->setUniformValue("projection", projection);
    
    vao.bind();
    
    // 绘制一个简单的矩形作为示例
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    vao.release();
    program->release();
}

void GameRenderer::initShaders()
{
    program = new QOpenGLShaderProgram(this);
    
    // 尝试从文件加载着色器，如果失败则使用默认着色器
    QString vertexShaderSource;
    QString fragmentShaderSource;
    
    QFile vertexFile("shaders/vertex_shader.glsl");
    if (vertexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        vertexShaderSource = vertexFile.readAll();
        vertexFile.close();
    } else {
        // 默认顶点着色器
        vertexShaderSource = R"(
            #version 330 core
            layout(location = 0) in vec2 position;
            layout(location = 1) in vec3 color;
            
            out vec3 fragColor;
            
            uniform mat4 projection;
            
            void main()
            {
                gl_Position = projection * vec4(position, 0.0, 1.0);
                fragColor = color;
            }
        )";
    }
    
    QFile fragmentFile("shaders/fragment_shader.glsl");
    if (fragmentFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fragmentShaderSource = fragmentFile.readAll();
        fragmentFile.close();
    } else {
        // 默认片段着色器
        fragmentShaderSource = R"(
            #version 330 core
            in vec3 fragColor;
            out vec4 outColor;
            
            void main()
            {
                outColor = vec4(fragColor, 1.0);
            }
        )";
    }
    
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qDebug() << "Vertex shader compile error:" << program->log();
    }
    
    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qDebug() << "Fragment shader compile error:" << program->log();
    }
    
    if (!program->link()) {
        qDebug() << "Shader program link error:" << program->log();
    }
}

void GameRenderer::initGeometry()
{
    // 定义三角形数据（两个三角形组成一个矩形）
    GLfloat vertices[] = {
        // 位置       // 颜色
        100.0f, 100.0f,  1.0f, 0.0f, 0.0f,  // 红色
        300.0f, 100.0f,  0.0f, 1.0f, 0.0f,  // 绿色
        100.0f, 300.0f,  0.0f, 0.0f, 1.0f,  // 蓝色
        
        300.0f, 100.0f,  0.0f, 1.0f, 0.0f,  // 绿色
        300.0f, 300.0f,  1.0f, 1.0f, 0.0f,  // 黄色
        100.0f, 300.0f,  0.0f, 0.0f, 1.0f   // 蓝色
    };
    
    vao.create();
    vao.bind();
    
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));
    
    // 设置顶点属性指针
    program->bind();
    
    // 位置属性
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 2, 5 * sizeof(GLfloat));
    
    // 颜色属性
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(GLfloat), 3, 5 * sizeof(GLfloat));
    
    program->release();
    vao.release();
}