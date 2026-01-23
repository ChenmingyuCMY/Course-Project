#include "BossScene.h"
#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QPainter>
#include <QImage>
#include <QLinearGradient>
#include <QDebug>
#include <QElapsedTimer>
#include <cmath>
#include <random>
#include <ctime>
#include <QFile>

BossScene::BossScene(QWidget *parent)
    : BaseRenderer(parent)
    , groundTexture(0)
    , brazierTexture(0)
    , groundLevel(-5.0f)
    , groundWidth(20.0f)
    , groundSegments(50)
    , bossLevel(1)
    , battleActive(true)
{
    // 设置相机初始位置
    setCameraPosition(QVector2D(0.0f, 0.0f));
    setCameraZoom(1.0f);
    
    // 初始化背景层次
    backgroundLayers.resize(3);
    backgroundLayers[0] = QVector2D(0.0f, 0.0f); // 远景
    backgroundLayers[1] = QVector2D(0.0f, 0.0f); // 中景
    backgroundLayers[2] = QVector2D(0.0f, 0.0f); // 近景
    
    midgroundLayers.resize(2);
    foregroundLayers.resize(2);
    
    // 初始化火把位置
    brazierPositions.resize(4);
    brazierPositions[0] = QVector2D(-8.0f, groundLevel);  // 左1
    brazierPositions[1] = QVector2D(-5.0f, groundLevel);  // 左2
    brazierPositions[2] = QVector2D(5.0f, groundLevel);   // 右1
    brazierPositions[3] = QVector2D(8.0f, groundLevel);   // 右2
    
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
    player.position = QVector2D(-2.0f, groundLevel);
    player.velocity = QVector2D(0.0f, 0.0f);
    player.isGrounded = true;
    player.facingRight = true;
    player.health = 100.0f;
    player.maxHealth = 100.0f;
    player.currentAnimation = "idle";
    player.animationTime = 0.0f;
    
    // 初始化Boss
    boss.position = QVector2D(2.0f, groundLevel);
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
    
    if (groundTexture) {
        glDeleteTextures(1, &groundTexture);
        groundTexture = 0;
    }
    
    if (brazierTexture) {
        glDeleteTextures(1, &brazierTexture);
        brazierTexture = 0;
    }
    
    // 清理骨骼内存
    for (Bone* bone : player.bones) {
        delete bone;
    }
    player.bones.clear();
    
    // 清理状态效果
    for (Status* status : player.statuses) {
        delete status;
    }
    player.statuses.clear();
    
    for (Bone* bone : boss.bones) {
        delete bone;
    }
    boss.bones.clear();
    
    for (Status* status : boss.statuses) {
        delete status;
    }
    boss.statuses.clear();
    
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
    BaseRenderer::initializeGL(); // 调用基类初始化
    
    // 创建纹理
    createTextures();
    
    // 设置场景
    setupScene();
    
    gameTimer.start();
}

void BossScene::createTextures()
{
    // 生成地面纹理
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
    
    // 上传地面纹理到GPU
    createTextureFromImage(texture, groundTexture, GL_LINEAR, GL_LINEAR);
    
    // 加载火把纹理
    QString brazierPath = "../assets/brazier.png";
    if (QFile::exists(brazierPath)) {
        QImage brazierImage(brazierPath);
        createTextureFromImage(brazierImage, brazierTexture, GL_LINEAR, GL_LINEAR);
        // debugTextureAlpha(brazierTexture, "brazier");
        qDebug() << "Loaded brazier texture:" << brazierPath;
    }

    // 加载墙纹理
    QString wallPath = "../assets/wall.jpg";
    if (QFile::exists(wallPath)) {
        QImage wallImage(wallPath);
        createTextureFromImage(wallImage, wallTexture, GL_LINEAR, GL_LINEAR);
        qDebug() << "Loaded wall texture:" << wallPath;
    }
}

void BossScene::debugTextureAlpha(GLuint textureId, const QString& name)
{
    makeCurrent();
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // 获取纹理尺寸
    GLint width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    
    // 读取纹理数据
    QVector<unsigned char> pixels(width * height * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // 检查alpha值
    bool hasAlpha = false;
    for (int i = 3; i < pixels.size(); i += 4) {
        if (pixels[i] < 255) {
            hasAlpha = true;
            qDebug() << "Pixel at index" << i/4 << "has alpha:" << pixels[i];
            break;
        }
    }
    
    qDebug() << "Texture" << name << "size:" << width << "x" << height 
             << "has alpha values:" << hasAlpha;
    
    glBindTexture(GL_TEXTURE_2D, 0);
    doneCurrent();
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
    BaseRenderer::resizeGL(w, h); // 调用基类方法
}

void BossScene::paintGL()
{
    BaseRenderer::paintGL(); // 调用基类清屏
    
    // 更新相机位置（跟随玩家）
    QVector2D cameraPos = getCameraPosition();
    cameraPos.setX(player.position.x());
    cameraPos.setY(player.position.y() * 0.5f);
    setCameraPosition(cameraPos);
    
    // 渲染场景
    renderScene();
}

void BossScene::renderScene()
{
    // 绘制背景层次
    // drawBackground();
    drawMidground();
    
    
    // 绘制地面
    drawGround();
    
    // 绘制角色
    drawCharacter(player);
    drawCharacter(boss);

    drawForeground();

    // 绘制碰撞体（调试用）
    drawHitboxes();
    
    // 绘制血条
    drawHealthBars();
}

void BossScene::drawBackground()
{
    // 绘制远景
    for (int i = 0; i < 3; i++) {
        QMatrix4x4 model;
        model.translate(backgroundLayers[i].x() + getCameraPosition().x() * (0.2f * (i + 1)), 
                       backgroundLayers[i].y() + getCameraPosition().y() * (0.1f * (i + 1)));
        model.scale(20.0f, 6.0f, 1.0f);
        
        renderColoredQuad(model, 
            QVector3D(layerColors[i].redF(), layerColors[i].greenF(), layerColors[i].blueF()),
            1.0f, "simple");
    }
}

void BossScene::drawMidground()
{
    if (wallTexture) {
        // 计算纹理的宽高比
        float aspectRatio = 2048.0f / 1024.0f;
        
        // 设置墙的宽度和高度
        float wallWidth = groundWidth * 1.5f; // 比地面宽一些
        float wallHeight = wallWidth / aspectRatio; // 保持纹理比例
        
        // 墙的位置（在地面上方）
        float wallY = groundLevel + wallHeight * 0.5f - 0.5f;
        
        QMatrix4x4 model;
        // 应用视差效果（
        float parallaxFactor = 0.4f; // 中景的视差系数
        model.translate(getCameraPosition().x() * parallaxFactor, 
                       wallY + getCameraPosition().y() * 0.05f, -0.2f);
        
        // 设置墙的大小
        model.scale(wallWidth, wallHeight, 1.0f);
        
        // 渲染纹理四边形
        renderTexturedQuad(model, wallTexture, QVector4D(1.0f, 1.0f, 1.0f, 1.0f), "texture");
    }
}

void BossScene::drawForeground()
{
    // 使用纹理绘制火把
    if (brazierTexture) {
        float aspectRatio = 256.0f / 128.0f; // 宽高比
        float brazierWidth = 1.2f;
        float brazierHeight = brazierWidth / aspectRatio;
        float parallaxFactor = 0.8f;

        for (int i = 0; i < brazierPositions.size(); i++) {
            QMatrix4x4 model;
            float parallaxX = brazierPositions[i].x() - getCameraPosition().x() * parallaxFactor;
            float parallaxY = brazierPositions[i].y() + getCameraPosition().y() * 0.1f;
            
            model.translate(parallaxX, parallaxY, 0.2f);
            model.scale(brazierWidth, brazierHeight, 1.0f);
            renderTexturedQuad(model, brazierTexture, QVector4D(1.0f, 1.0f, 1.0f, 1.0f), "texture");
        }
    }
}

void BossScene::drawGround()
{
    QMatrix4x4 groundModel;
    groundModel.translate(getCameraPosition().x(), groundLevel - 0.5f);
    groundModel.scale(groundWidth, 0.5f, 1.0f);
    
    renderTexturedQuad(groundModel, groundTexture, QVector4D(1.0f, 1.0f, 1.0f, 1.0f), "texture");
}

void BossScene::drawCharacter(const Character &character)
{
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
        QVector3D charColor;
        if (&character == &player) {
            charColor = QVector3D(0.4f, 0.6f, 1.0f); // 蓝色玩家
        } else {
            charColor = QVector3D(1.0f, 0.4f, 0.4f); // 红色Boss
        }
        
        renderColoredQuad(model, charColor, 1.0f, "simple");
    }
    
    // 绘制状态效果
    for (Status* status : character.statuses) {
        if (status->bone) {
            QMatrix4x4 model;
            model.translate(character.position.x() + status->bone->position.x(), 
                            character.position.y() + status->bone->position.y());
            model.rotate(status->bone->rotation, 0.0f, 0.0f, 1.0f);
            model.scale(0.3f, 0.3f, 1.0f);
            
            renderColoredQuad(model, 
                QVector3D(status->color.redF(), status->color.greenF(), status->color.blueF()),
                0.7f, "simple");
        }
    }
}

void BossScene::drawHitboxes()
{
    if (!battleActive) return;
    
    // 绘制玩家碰撞体
    for (const Hitbox& hitbox : player.hitboxes) {
        if (hitbox.isAttack) {
            QMatrix4x4 model;
            model.translate(player.position.x() + hitbox.position.x(), 
                          player.position.y() + hitbox.position.y());
            model.scale(hitbox.size.x(), hitbox.size.y(), 1.0f);
            
            renderColoredQuad(model, QVector3D(1.0f, 0.0f, 0.0f), 0.5f, "simple");
        }
    }
    
    // 绘制Boss碰撞体
    for (const Hitbox& hitbox : boss.hitboxes) {
        QMatrix4x4 model;
        model.translate(boss.position.x() + hitbox.position.x(), 
                       boss.position.y() + hitbox.position.y());
        model.scale(hitbox.size.x(), hitbox.size.y(), 1.0f);
        
        QVector3D color = hitbox.isAttack ? QVector3D(1.0f, 0.5f, 0.0f) : QVector3D(0.0f, 1.0f, 0.0f);
        renderColoredQuad(model, color, 0.5f, "simple");
    }
}

void BossScene::drawHealthBars()
{
    QVector2D cameraPos = getCameraPosition();
    
    // 玩家血条背景
    QMatrix4x4 playerHealthBg;
    playerHealthBg.translate(cameraPos.x() - 4.5f, cameraPos.y() + 4.0f);
    playerHealthBg.scale(4.0f, 0.3f, 1.0f);
    
    renderColoredQuad(playerHealthBg, QVector3D(0.2f, 0.2f, 0.2f), 1.0f, "simple");
    
    // 玩家血条
    float playerHealthRatio = std::max(0.0f, player.health / player.maxHealth);
    QMatrix4x4 playerHealth;
    playerHealth.translate(cameraPos.x() - 4.5f + (playerHealthRatio * 2.0f - 2.0f), 
                          cameraPos.y() + 4.0f);
    playerHealth.scale(playerHealthRatio * 4.0f, 0.25f, 1.0f);
    
    QVector3D healthColor(1.0f - playerHealthRatio, playerHealthRatio, 0.0f);
    renderColoredQuad(playerHealth, healthColor, 1.0f, "simple");
    
    // Boss血条背景
    QMatrix4x4 bossHealthBg;
    bossHealthBg.translate(cameraPos.x() + 4.5f, cameraPos.y() + 4.0f);
    bossHealthBg.scale(4.0f, 0.3f, 1.0f);
    
    renderColoredQuad(bossHealthBg, QVector3D(0.2f, 0.2f, 0.2f), 1.0f, "simple");
    
    // Boss血条
    float bossHealthRatio = std::max(0.0f, boss.health / boss.maxHealth);
    QMatrix4x4 bossHealth;
    bossHealth.translate(cameraPos.x() + 4.5f - (2.0f - bossHealthRatio * 2.0f), 
                        cameraPos.y() + 4.0f);
    bossHealth.scale(bossHealthRatio * 4.0f, 0.25f, 1.0f);
    
    healthColor = QVector3D(1.0f - bossHealthRatio, bossHealthRatio, 0.0f);
    renderColoredQuad(bossHealth, healthColor, 1.0f, "simple");
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
    if (isKeyPressed(Qt::Key_A)) {
        player.velocity.setX(-playerSpeed);
        player.facingRight = false;
    } else if (isKeyPressed(Qt::Key_D)) {
        player.velocity.setX(playerSpeed);
        player.facingRight = true;
    } else {
        player.velocity.setX(player.velocity.x() * 0.9f); // 摩擦
    }
    
    if (isKeyPressed(Qt::Key_W) && player.isGrounded) {
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
        if (isKeyPressed(Qt::Key_Space)) {
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
    BaseRenderer::keyPressEvent(event); // 调用基类处理键盘输入
    
    // 攻击
    if (event->key() == Qt::Key_Space) {
        // 创建攻击碰撞体
        Hitbox attack;
        attack.name = "attack";
        attack.position = QVector2D(player.facingRight ? 0.5f : -0.5f, 0.2f);
        attack.size = QVector2D(0.8f, 0.4f);
        attack.isAttack = true;
        attack.damage = 10;
        
        player.hitboxes.append(attack);
    }
}

void BossScene::keyReleaseEvent(QKeyEvent *event)
{
    BaseRenderer::keyReleaseEvent(event); // 调用基类处理键盘输入
    
    // 移除攻击碰撞体
    if (event->key() == Qt::Key_Space) {
        player.hitboxes.clear();
    }
}

void BossScene::mousePressEvent(QMouseEvent *event)
{
    BaseRenderer::mousePressEvent(event); // 调用基类处理鼠标输入
}