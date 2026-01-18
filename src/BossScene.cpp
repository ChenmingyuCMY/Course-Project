#include "BossScene.h"
#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QPainter>
#include <QImage>
#include <QLinearGradient>
#include <QDebug>
#include <cmath>
#include <random>
#include <ctime>

// 顶点数据（矩形）
static const GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f
};

static const GLfloat texCoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

BossScene::BossScene(QWidget *parent)
    : QOpenGLWidget(parent)
    , simpleShader(nullptr)
    , textureShader(nullptr)
    , groundTexture(0)
    , backgroundColor(0.2f, 0.3f, 0.4f)
    , groundLevel(-0.8f)
    , groundWidth(20.0f)
    , groundSegments(50)
    , bossLevel(1)
    , battleActive(true)
    , cameraPosition(0.0f, 0.0f)
    , cameraZoom(1.0f)
{
    setFocusPolicy(Qt::StrongFocus);
    
    // 初始化背景层次
    backgroundLayers.resize(3);
    backgroundLayers[0] = QVector2D(0.0f, 0.0f); // 远景
    backgroundLayers[1] = QVector2D(0.0f, 0.0f); // 中景
    backgroundLayers[2] = QVector2D(0.0f, 0.0f); // 近景
    
    midgroundLayers.resize(2);
    foregroundLayers.resize(2);
    
    // 层次颜色
    layerColors.resize(7);
    layerColors[0] = QColor(80, 100, 140);    // 最远背景
    layerColors[1] = QColor(100, 120, 160);   // 背景
    layerColors[2] = QColor(120, 140, 180);   // 后景
    layerColors[3] = QColor(140, 160, 200);   // 中景
    layerColors[4] = QColor(60, 80, 100);     // 前景
    layerColors[5] = QColor(80, 60, 40);      // 地面
    layerColors[6] = QColor(120, 100, 80);    // 地面细节
    
    // 初始化玩家
    player.position = QVector2D(-2.0f, groundLevel + 0.5f);
    player.velocity = QVector2D(0.0f, 0.0f);
    player.isGrounded = true;
    player.facingRight = true;
    player.health = 100.0f;
    player.maxHealth = 100.0f;
    player.currentAnimation = "idle";
    player.animationTime = 0.0f;
    
    // 初始化Boss
    boss.position = QVector2D(2.0f, groundLevel + 1.0f);
    boss.velocity = QVector2D(0.0f, 0.0f);
    boss.isGrounded = true;
    boss.facingRight = false;
    boss.health = 200.0f + bossLevel * 50.0f;
    boss.maxHealth = boss.health;
    boss.currentAnimation = "idle";
    boss.animationTime = 0.0f;
    
    // 设置更新定时器
    connect(&updateTimer, &QTimer::timeout, this, &BossScene::updateGame);
    updateTimer.start(16); // ~60 FPS
    
    // 初始化随机种子
    std::srand(std::time(nullptr));
}

BossScene::~BossScene()
{
    makeCurrent();
    
    if (simpleShader) {
        delete simpleShader;
        simpleShader = nullptr;
    }
    if (textureShader) {
        delete textureShader;
        textureShader = nullptr;
    }
    
    if (groundTexture) {
        glDeleteTextures(1, &groundTexture);
        groundTexture = 0;
    }
    
    vao.destroy();
    vbo.destroy();
    uvbo.destroy();
    
    // 清理骨骼内存
    for (Bone* bone : player.bones) {
        delete bone;
    }
    player.bones.clear();
    
    for (Bone* bone : boss.bones) {
        delete bone;
    }
    boss.bones.clear();
    
    doneCurrent();
}

void BossScene::setBossLevel(int level)
{
    bossLevel = level;
    boss.health = 200.0f + bossLevel * 50.0f;
    boss.maxHealth = boss.health;
    battleActive = true;
}

void BossScene::initializeGL()
{
    initializeOpenGLFunctions();
    
    glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    setupShaders();
    setupBuffers();
    generateGroundTexture();
    setupScene();
    
    gameTimer.start();
}

void BossScene::setupShaders()
{
    // 简单颜色着色器
    simpleShader = new QOpenGLShaderProgram(this);
    if (!simpleShader->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\n"
        "layout(location = 0) in vec3 position;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 model;\n"
        "void main() {\n"
        "    gl_Position = projection * model * vec4(position, 1.0);\n"
        "}")) {
        qDebug() << "Vertex shader compile error:" << simpleShader->log();
    }
    
    if (!simpleShader->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\n"
        "uniform vec3 color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vec4(color, 1.0);\n"
        "}")) {
        qDebug() << "Fragment shader compile error:" << simpleShader->log();
    }
    
    if (!simpleShader->link()) {
        qDebug() << "Shader link error:" << simpleShader->log();
    }
    
    // 纹理着色器
    textureShader = new QOpenGLShaderProgram(this);
    if (!textureShader->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 model;\n"
        "out vec2 vTexCoord;\n"
        "void main() {\n"
        "    gl_Position = projection * model * vec4(position, 1.0);\n"
        "    vTexCoord = texCoord;\n"
        "}")) {
        qDebug() << "Texture vertex shader compile error:" << textureShader->log();
    }
    
    if (!textureShader->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\n"
        "in vec2 vTexCoord;\n"
        "uniform sampler2D textureSampler;\n"
        "uniform vec4 tintColor;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    vec4 texColor = texture(textureSampler, vTexCoord);\n"
        "    fragColor = texColor * tintColor;\n"
        "}")) {
        qDebug() << "Texture fragment shader compile error:" << textureShader->log();
    }
    
    if (!textureShader->link()) {
        qDebug() << "Texture shader link error:" << textureShader->log();
    }
}

void BossScene::setupBuffers()
{
    vao.create();
    vao.bind();
    
    // 顶点缓冲区
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    // 纹理坐标缓冲区
    uvbo.create();
    uvbo.bind();
    uvbo.allocate(texCoords, sizeof(texCoords));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    vao.release();
}

void BossScene::generateGroundTexture()
{
    const int width = 512;
    const int height = 128;
    QImage texture(width, height, QImage::Format_ARGB32);
    
    // 创建渐变背景
    QLinearGradient gradient(0, 0, 0, height);
    gradient.setColorAt(0, QColor(80, 60, 40));
    gradient.setColorAt(1, QColor(120, 100, 80));
    
    QPainter painter(&texture);
    painter.fillRect(0, 0, width, height, gradient);
    
    // 添加细节
    painter.setPen(QPen(QColor(100, 80, 60), 2));
    for (int i = 0; i < width; i += 32) {
        painter.drawLine(i, 0, i, height);
    }
    
    // 添加噪声
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 30);
    
    for (int i = 0; i < width; i += 4) {
        for (int j = 0; j < height; j += 4) {
            int noise = dis(gen);
            QColor noiseColor = QColor(noise + 80, noise + 60, noise + 40);
            painter.setPen(noiseColor);
            painter.drawPoint(i, j);
        }
    }
    
    painter.end();
    
    // 上传到GPU
    makeCurrent();
    
    if (groundTexture) {
        glDeleteTextures(1, &groundTexture);
    }
    
    glGenTextures(1, &groundTexture);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    
    // 转换图像格式为RGBA
    QImage glTexture = texture.convertToFormat(QImage::Format_RGBA8888);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glTexture.width(), glTexture.height(), 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, glTexture.bits());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BossScene::setupScene()
{
    // 初始化玩家骨骼（简化版）
    Bone* playerRoot = new Bone{"root", QVector2D(0, 0), 0.0f, QVector2D(1, 1), nullptr};
    Bone* playerBody = new Bone{"body", QVector2D(0, 0.5f), 0.0f, QVector2D(0.6f, 1.0f), playerRoot};
    Bone* playerHead = new Bone{"head", QVector2D(0, 0.3f), 0.0f, QVector2D(0.5f, 0.5f), playerBody};
    
    playerRoot->children.append(playerBody);
    playerBody->children.append(playerHead);
    
    player.bones.append(playerRoot);
    player.bones.append(playerBody);
    player.bones.append(playerHead);
    
    // Boss骨骼
    Bone* bossRoot = new Bone{"root", QVector2D(0, 0), 0.0f, QVector2D(1.2f, 1.2f), nullptr};
    Bone* bossBody = new Bone{"body", QVector2D(0, 0.8f), 0.0f, QVector2D(1.0f, 1.5f), bossRoot};
    Bone* bossHead = new Bone{"head", QVector2D(0, 0.5f), 0.0f, QVector2D(0.8f, 0.8f), bossBody};
    
    bossRoot->children.append(bossBody);
    bossBody->children.append(bossHead);
    
    boss.bones.append(bossRoot);
    boss.bones.append(bossBody);
    boss.bones.append(bossHead);
}

void BossScene::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    
    // 更新投影矩阵（保持横版2D视角）
    projectionMatrix.setToIdentity();
    float aspectRatio = float(w) / float(h);
    float viewHeight = 10.0f / cameraZoom;
    float viewWidth = viewHeight * aspectRatio;
    projectionMatrix.ortho(-viewWidth/2, viewWidth/2, -viewHeight/2, viewHeight/2, -1.0f, 1.0f);
}

void BossScene::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderScene();
}

void BossScene::renderScene()
{
    vao.bind();
    
    // 更新相机位置（跟随玩家）
    cameraPosition.setX(player.position.x());
    cameraPosition.setY(player.position.y() * 0.5f);
    
    // 绘制背景层次
    drawBackground();
    drawMidground();
    drawForeground();
    
    // 绘制地面
    drawGround();
    
    // 绘制角色
    drawCharacter(player);
    drawCharacter(boss);
    
    // 绘制碰撞体（调试用）
    drawHitboxes();
    
    // 绘制血条
    drawHealthBars();
    
    vao.release();
}

void BossScene::drawBackground()
{
    if (!simpleShader) return;
    
    simpleShader->bind();
    simpleShader->setUniformValue("projection", projectionMatrix);
    
    // 绘制远景
    for (int i = 0; i < 3; i++) {
        QMatrix4x4 model;
        model.translate(backgroundLayers[i].x() + cameraPosition.x() * (0.2f * (i + 1)), 
                       backgroundLayers[i].y() + cameraPosition.y() * (0.1f * (i + 1)));
        model.scale(20.0f, 6.0f, 1.0f);
        
        simpleShader->setUniformValue("model", model);
        simpleShader->setUniformValue("color", 
            QVector3D(layerColors[i].redF(), layerColors[i].greenF(), layerColors[i].blueF()));
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    
    simpleShader->release();
}

void BossScene::drawMidground()
{
    if (!simpleShader) return;
    
    simpleShader->bind();
    simpleShader->setUniformValue("projection", projectionMatrix);
    
    // 绘制中景元素（山、云等）
    for (int i = 3; i < 5; i++) {
        QMatrix4x4 model;
        model.translate(midgroundLayers[i-3].x() + cameraPosition.x() * (0.5f * (i-2)), 
                       -2.0f + cameraPosition.y() * 0.2f);
        model.scale(5.0f, 3.0f, 1.0f);
        
        simpleShader->setUniformValue("model", model);
        simpleShader->setUniformValue("color", 
            QVector3D(layerColors[i].redF(), layerColors[i].greenF(), layerColors[i].blueF()));
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    
    simpleShader->release();
}

void BossScene::drawForeground()
{
    if (!simpleShader) return;
    
    simpleShader->bind();
    simpleShader->setUniformValue("projection", projectionMatrix);
    
    // 绘制前景元素
    for (int i = 5; i < 7 && i-5 < foregroundLayers.size(); i++) {
        QMatrix4x4 model;
        model.translate(foregroundLayers[i-5].x() + cameraPosition.x() * 0.8f, 
                       groundLevel + 0.5f);
        model.scale(1.0f, 1.5f, 1.0f);
        
        simpleShader->setUniformValue("model", model);
        simpleShader->setUniformValue("color", 
            QVector3D(layerColors[i].redF(), layerColors[i].greenF(), layerColors[i].blueF()));
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    
    simpleShader->release();
}

void BossScene::drawGround()
{
    if (!textureShader) return;
    
    textureShader->bind();
    textureShader->setUniformValue("projection", projectionMatrix);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    textureShader->setUniformValue("textureSampler", 0);
    textureShader->setUniformValue("tintColor", QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    
    // 绘制地面
    QMatrix4x4 groundModel;
    groundModel.translate(cameraPosition.x(), groundLevel);
    groundModel.scale(groundWidth, 0.5f, 1.0f);
    
    textureShader->setUniformValue("model", groundModel);
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    textureShader->release();
}

void BossScene::drawCharacter(const Character &character)
{
    if (!simpleShader) return;
    
    simpleShader->bind();
    simpleShader->setUniformValue("projection", projectionMatrix);
    
    // 绘制身体
    for (Bone* bone : character.bones) {
        QMatrix4x4 model;
        model.translate(character.position.x() + bone->position.x(), 
                       character.position.y() + bone->position.y());
        if (!character.facingRight) {
            model.scale(-1.0f, 1.0f, 1.0f);
        }
        model.rotate(bone->rotation, 0.0f, 0.0f, 1.0f);
        model.scale(bone->scale.x() * 0.5f, bone->scale.y() * 0.5f, 1.0f);
        
        // 设置颜色
        QColor charColor;
        if (&character == &player) {
            charColor = QColor(100, 150, 255); // 蓝色玩家
        } else {
            charColor = QColor(255, 100, 100); // 红色Boss
        }
        
        simpleShader->setUniformValue("model", model);
        simpleShader->setUniformValue("color", 
            QVector3D(charColor.redF(), charColor.greenF(), charColor.blueF()));
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    
    simpleShader->release();
}

void BossScene::drawHitboxes()
{
    if (!battleActive || !simpleShader) return;
    
    simpleShader->bind();
    simpleShader->setUniformValue("projection", projectionMatrix);
    
    // 绘制玩家碰撞体
    for (const Hitbox& hitbox : player.hitboxes) {
        if (hitbox.isAttack) {
            QMatrix4x4 model;
            model.translate(player.position.x() + hitbox.position.x(), 
                          player.position.y() + hitbox.position.y());
            model.scale(hitbox.size.x(), hitbox.size.y(), 1.0f);
            
            simpleShader->setUniformValue("model", model);
            simpleShader->setUniformValue("color", QVector3D(1.0f, 0.0f, 0.0f)); // 红色攻击框
            
            // 使用线框模式
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    
    // 绘制Boss碰撞体
    for (const Hitbox& hitbox : boss.hitboxes) {
        QMatrix4x4 model;
        model.translate(boss.position.x() + hitbox.position.x(), 
                       boss.position.y() + hitbox.position.y());
        model.scale(hitbox.size.x(), hitbox.size.y(), 1.0f);
        
        simpleShader->setUniformValue("model", model);
        if (hitbox.isAttack) {
            simpleShader->setUniformValue("color", QVector3D(1.0f, 0.5f, 0.0f)); // 橙色攻击框
        } else {
            simpleShader->setUniformValue("color", QVector3D(0.0f, 1.0f, 0.0f)); // 绿色受击框
        }
        
        // 使用线框模式
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    simpleShader->release();
}

void BossScene::drawHealthBars()
{
    if (!simpleShader) return;
    
    simpleShader->bind();
    simpleShader->setUniformValue("projection", projectionMatrix);
    
    // 玩家血条背景
    QMatrix4x4 playerHealthBg;
    playerHealthBg.translate(cameraPosition.x() - 4.5f, cameraPosition.y() + 4.0f);
    playerHealthBg.scale(4.0f, 0.3f, 1.0f);
    
    simpleShader->setUniformValue("model", playerHealthBg);
    simpleShader->setUniformValue("color", QVector3D(0.2f, 0.2f, 0.2f));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // 玩家血条
    float playerHealthRatio = std::max(0.0f, player.health / player.maxHealth);
    QMatrix4x4 playerHealth;
    playerHealth.translate(cameraPosition.x() - 4.5f + (playerHealthRatio * 2.0f - 2.0f), 
                          cameraPosition.y() + 4.0f);
    playerHealth.scale(playerHealthRatio * 4.0f, 0.25f, 1.0f);
    
    simpleShader->setUniformValue("model", playerHealth);
    QVector3D healthColor(1.0f - playerHealthRatio, playerHealthRatio, 0.0f);
    simpleShader->setUniformValue("color", healthColor);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Boss血条背景
    QMatrix4x4 bossHealthBg;
    bossHealthBg.translate(cameraPosition.x() + 4.5f, cameraPosition.y() + 4.0f);
    bossHealthBg.scale(4.0f, 0.3f, 1.0f);
    
    simpleShader->setUniformValue("model", bossHealthBg);
    simpleShader->setUniformValue("color", QVector3D(0.2f, 0.2f, 0.2f));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Boss血条
    float bossHealthRatio = std::max(0.0f, boss.health / boss.maxHealth);
    QMatrix4x4 bossHealth;
    bossHealth.translate(cameraPosition.x() + 4.5f - (2.0f - bossHealthRatio * 2.0f), 
                        cameraPosition.y() + 4.0f);
    bossHealth.scale(bossHealthRatio * 4.0f, 0.25f, 1.0f);
    
    simpleShader->setUniformValue("model", bossHealth);
    healthColor = QVector3D(1.0f - bossHealthRatio, bossHealthRatio, 0.0f);
    simpleShader->setUniformValue("color", healthColor);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    simpleShader->release();
}

void BossScene::updateGame()
{
    if (!battleActive) return;
    
    float deltaTime = 0.016f; // 假设60FPS
    
    // 更新物理
    updatePhysics(deltaTime);
    
    // 更新动画
    updateAnimations(deltaTime);
    
    // 检查碰撞
    checkCollisions();
    
    // 更新背景移动（视差效果）
    for (int i = 0; i < backgroundLayers.size(); i++) {
        backgroundLayers[i].setX(backgroundLayers[i].x() - deltaTime * (i + 1) * 0.1f);
    }
    
    // 检查游戏结束条件
    if (player.health <= 0) {
        battleActive = false;
        emit battleLost();
    } else if (boss.health <= 0) {
        battleActive = false;
        emit battleWon();
    }
    
    update();
}

void BossScene::updatePhysics(float deltaTime)
{
    // 重力
    player.velocity.setY(player.velocity.y() - 9.8f * deltaTime);
    boss.velocity.setY(boss.velocity.y() - 9.8f * deltaTime);
    
    // 玩家输入
    float playerSpeed = 5.0f;
    if (pressedKeys.contains(Qt::Key_A)) {
        player.velocity.setX(-playerSpeed);
        player.facingRight = false;
    } else if (pressedKeys.contains(Qt::Key_D)) {
        player.velocity.setX(playerSpeed);
        player.facingRight = true;
    } else {
        player.velocity.setX(player.velocity.x() * 0.9f); // 摩擦
    }
    
    if (pressedKeys.contains(Qt::Key_W) && player.isGrounded) {
        player.velocity.setY(8.0f);
        player.isGrounded = false;
    }
    
    // 更新位置
    player.position += player.velocity * deltaTime;
    boss.position += boss.velocity * deltaTime;
    
    // 地面碰撞
    if (player.position.y() < groundLevel) {
        player.position.setY(groundLevel);
        player.velocity.setY(0.0f);
        player.isGrounded = true;
    }
    
    if (boss.position.y() < groundLevel) {
        boss.position.setY(groundLevel);
        boss.velocity.setY(0.0f);
        boss.isGrounded = true;
    }
    
    // 边界检查
    float boundary = groundWidth / 2;
    if (player.position.x() < -boundary) player.position.setX(-boundary);
    if (player.position.x() > boundary) player.position.setX(boundary);
    if (boss.position.x() < -boundary) boss.position.setX(-boundary);
    if (boss.position.x() > boundary) boss.position.setX(boundary);
}

void BossScene::updateAnimations(float deltaTime)
{
    player.animationTime += deltaTime;
    boss.animationTime += deltaTime;
    
    // 更新骨骼动画（简化版）
    for (Bone* bone : player.bones) {
        if (bone->name == "body") {
            // 简单的呼吸动画
            bone->position.setY(0.5f + sin(player.animationTime * 2.0f) * 0.05f);
        }
    }
    
    for (Bone* bone : boss.bones) {
        if (bone->name == "body") {
            // Boss的威胁动画
            float scale = 1.0f + sin(boss.animationTime * 1.5f) * 0.1f;
            bone->scale = QVector2D(scale, scale);
        }
    }
}

void BossScene::checkCollisions()
{
    // 简单的碰撞检测
    float distance = (player.position - boss.position).length();
    if (distance < 1.0f) {
        // 简单的伤害
        if (pressedKeys.contains(Qt::Key_Space)) {
            boss.health -= 10.0f * 0.016f; // 使用固定帧时间
        }
        
        // Boss反击（10%几率）
        if (std::rand() % 100 < 10) {
            player.health -= 5.0f;
        }
    }
}

void BossScene::keyPressEvent(QKeyEvent *event)
{
    pressedKeys.insert(event->key());
    
    // 攻击
    if (event->key() == Qt::Key_Space) {
        // 创建攻击碰撞体
        Hitbox attack;
        attack.position = QVector2D(player.facingRight ? 0.5f : -0.5f, 0.2f);
        attack.size = QVector2D(0.8f, 0.4f);
        attack.isAttack = true;
        attack.damage = 10;
        
        player.hitboxes.append(attack);
    }
    
    QOpenGLWidget::keyPressEvent(event);
}

void BossScene::keyReleaseEvent(QKeyEvent *event)
{
    pressedKeys.remove(event->key());
    
    // 移除攻击碰撞体
    if (event->key() == Qt::Key_Space) {
        player.hitboxes.clear();
    }
    
    QOpenGLWidget::keyReleaseEvent(event);
}

void BossScene::mousePressEvent(QMouseEvent *event)
{
    // 可以添加鼠标控制
    QOpenGLWidget::mousePressEvent(event);
}