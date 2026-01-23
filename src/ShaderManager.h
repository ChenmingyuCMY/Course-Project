#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <QOpenGLShaderProgram>
#include <QObject>
#include <QMap>
#include <QString>
#include <QSharedPointer>
#include <QMutex>

class ShaderManager : public QObject
{
    Q_OBJECT
    
public:
    static ShaderManager* instance();
    
    // 注册全局着色器
    bool registerShader(const QString& name,
                       const QString& vertexSource,
                       const QString& fragmentSource);
    
    bool registerShaderFromFiles(const QString& name,
                                const QString& vertexFile,
                                const QString& fragmentFile);
    
    // 获取着色器
    QOpenGLShaderProgram* getShader(const QString& name);
    bool hasShader(const QString& name) const;
    QStringList getShaderNames() const;
    
    // 预设着色器
    enum PresetShader {
        SimpleColorShader,
        TextureShader,
        OutlineShader,
        ParticleShader,
        BlurShader,
        PostProcessShader
    };
    
    void loadPresetShaders();
    
private:
    ShaderManager(QObject* parent = nullptr);
    ~ShaderManager();
    
    static ShaderManager* m_instance;
    static QMutex m_mutex;
    
    QMap<QString, QOpenGLShaderProgram*> m_shaders;
    
    // 编译着色器
    QOpenGLShaderProgram* compileShader(const QString& name,
                                       const QString& vertexSource,
                                       const QString& fragmentSource);
    
    // 预设着色器源码
    struct ShaderSource {
        QString vertex;
        QString fragment;
    };
    QMap<PresetShader, ShaderSource> m_presetSources;
    void initializePresetSources();
};

#endif // SHADERMANAGER_H