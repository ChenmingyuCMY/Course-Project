#ifndef BOSSSCENE_H
#define BOSSSCENE_H

#include "BaseRenderer.h"

// 简单的骨骼动画结构
struct Bone {
    QString name;
    QVector2D position;
    float rotation;
    QVector2D scale;
    Bone* parent;
    QVector<Bone*> children;
};

struct Status {
    QString name;
    Bone* bone;
    QString attachment;
    QColor color;
};

// 碰撞体
struct Hitbox {
    QString name;
    QVector2D position;
    QVector2D size;
    bool isAttack;
    int damage;
};

// 角色状态
struct Character {
    QVector2D position;
    QVector2D velocity;
    bool isGrounded;
    bool facingRight;
    float health;
    float maxHealth;
    QVector<Hitbox> hitboxes;
    QVector<Bone*> bones;
    QVector<Status*> statuses;
    QString currentAnimation;
    float animationTime;
};

class BossScene : public BaseRenderer
{
    Q_OBJECT
    
public:
    explicit BossScene(QWidget *parent = nullptr);
    ~BossScene();
    
    void setBossLevel(int level);
    
signals:
    void battleWon();
    void battleLost();
    
protected:
    // 重写基类方法
    void initializeGL() override;    
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    
private slots:
    void updateGame();
    
private:
    void setupScene();
    void updatePhysics(float deltaTime);
    void checkCollisions();
    void updateAnimations(float deltaTime);
    void renderScene();
    void drawBackground();
    void drawMidground();
    void drawForeground();
    void drawCharacter(const Character &character);
    void drawHitboxes();
    void drawHealthBars();
    void drawGround();
    
    // 游戏对象
    Character player;
    Character boss;
    
    // 场景层次
    QVector<QVector2D> backgroundLayers;
    QVector<QVector2D> midgroundLayers;
    QVector<QVector2D> foregroundLayers;
    QVector<QColor> layerColors;
    
    // 地面参数
    float groundLevel;
    float groundWidth;
    int groundSegments;
    
    // 纹理
    GLuint groundTexture;
    GLuint brazierTexture;
    GLuint wallTexture;
    
    // 游戏状态
    int bossLevel;
    bool battleActive;
    QElapsedTimer gameTimer;
    QTimer updateTimer;
    
    // 前景火把位置
    QVector<QVector2D> brazierPositions;
    
    void createTextures();


    void debugTextureAlpha(GLuint textureId, const QString& name);
};

#endif // BOSSSCENE_H