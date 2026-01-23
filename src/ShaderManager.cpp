#include "ShaderManager.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

ShaderManager* ShaderManager::m_instance = nullptr;
QMutex ShaderManager::m_mutex;

ShaderManager::ShaderManager(QObject* parent) 
    : QObject(parent)
{
    initializePresetSources();
}

ShaderManager::~ShaderManager()
{
    qDeleteAll(m_shaders);
    m_shaders.clear();
}

ShaderManager* ShaderManager::instance()
{
    if (!m_instance) {
        QMutexLocker locker(&m_mutex);
        if (!m_instance) {
            m_instance = new ShaderManager();
        }
    }
    return m_instance;
}

void ShaderManager::initializePresetSources()
{
    // 简单颜色着色器
    ShaderSource simple;
    simple.vertex = R"(#version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        void main() {
            gl_Position = projection * view * model * vec4(position, 1.0);
        }
    )";
    
    simple.fragment = R"(#version 330 core
        uniform vec3 color;
        uniform float alpha;
        out vec4 fragColor;
        void main() {
            fragColor = vec4(color, alpha);
        }
    )";
    m_presetSources[SimpleColorShader] = simple;
    
    // 纹理着色器
    ShaderSource texture;
    texture.vertex = R"(#version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        out vec2 vTexCoord;
        void main() {
            gl_Position = projection * view * model * vec4(position, 1.0);
            vTexCoord = texCoord;
        }
    )";
    
    texture.fragment = R"(#version 330 core
        in vec2 vTexCoord;
        uniform sampler2D textureSampler;
        uniform vec4 tintColor;
        out vec4 fragColor;
        void main() {
            vec4 texColor = texture(textureSampler, vTexCoord);
            // 如果 alpha 值很小，直接丢弃片段
            if (texColor.a < 0.01) {
                discard;
            }
            fragColor = vec4(texColor.rgb * tintColor.rgb, texColor.a * tintColor.a);
        }
    )";
    m_presetSources[TextureShader] = texture;
    
    // 描边着色器
    ShaderSource outline;
    outline.vertex = R"(#version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        uniform float outlineSize;
        void main() {
            vec4 pos = view * model * vec4(position, 1.0);
            vec3 normal = normalize(cross(dFdx(pos.xyz), dFdy(pos.xyz)));
            pos.xyz += normal * outlineSize;
            gl_Position = projection * pos;
        }
    )";
    
    outline.fragment = R"(#version 330 core
        uniform vec3 outlineColor;
        uniform float outlineAlpha;
        out vec4 fragColor;
        void main() {
            fragColor = vec4(outlineColor, outlineAlpha);
        }
    )";
    m_presetSources[OutlineShader] = outline;
    
    // 粒子着色器
    ShaderSource particle;
    particle.vertex = R"(#version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        layout(location = 2) in vec4 color;
        layout(location = 3) in float size;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        uniform float time;
        out vec2 vTexCoord;
        out vec4 vColor;
        void main() {
            float wave = sin(time * 3.0 + position.x * 2.0) * 0.1;
            vec4 pos = view * model * vec4(position.x, position.y + wave, position.z, 1.0);
            gl_Position = projection * pos;
            gl_PointSize = size;
            vTexCoord = texCoord;
            vColor = color;
        }
    )";
    
    particle.fragment = R"(#version 330 core
        in vec2 vTexCoord;
        in vec4 vColor;
        uniform sampler2D textureSampler;
        out vec4 fragColor;
        void main() {
            vec4 texColor = texture(textureSampler, vTexCoord);
            fragColor = texColor * vColor;
        }
    )";
    m_presetSources[ParticleShader] = particle;
}

void ShaderManager::loadPresetShaders()
{
    for (auto it = m_presetSources.begin(); it != m_presetSources.end(); ++it) {
        QString name;
        switch (it.key()) {
            case SimpleColorShader: name = "simple"; break;
            case TextureShader: name = "texture"; break;
            case OutlineShader: name = "outline"; break;
            case ParticleShader: name = "particle"; break;
            case BlurShader: name = "blur"; break;
            case PostProcessShader: name = "postprocess"; break;
        }
        
        if (!m_shaders.contains(name)) {
            registerShader(name, it.value().vertex, it.value().fragment);
        }
    }
}

bool ShaderManager::registerShader(const QString& name,
                                  const QString& vertexSource,
                                  const QString& fragmentSource)
{
    if (m_shaders.contains(name)) {
        qDebug() << "Shader already registered globally:" << name;
        return true; // 已存在，返回成功
    }
    
    QOpenGLShaderProgram* shader = compileShader(name, vertexSource, fragmentSource);
    if (!shader) {
        return false;
    }
    
    m_shaders[name] = shader;
    qDebug() << "Shader registered globally:" << name;
    return true;
}

bool ShaderManager::registerShaderFromFiles(const QString& name,
                                           const QString& vertexFile,
                                           const QString& fragmentFile)
{
    if (m_shaders.contains(name)) {
        return true; // 已存在
    }
    
    QFile vFile(vertexFile);
    QFile fFile(fragmentFile);
    
    if (!vFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open vertex shader file:" << vertexFile;
        return false;
    }
    
    if (!fFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open fragment shader file:" << fragmentFile;
        vFile.close();
        return false;
    }
    
    QTextStream vStream(&vFile);
    QTextStream fStream(&fFile);
    
    QString vertexSource = vStream.readAll();
    QString fragmentSource = fStream.readAll();
    
    vFile.close();
    fFile.close();
    
    return registerShader(name, vertexSource, fragmentSource);
}

QOpenGLShaderProgram* ShaderManager::compileShader(const QString& name,
                                                  const QString& vertexSource,
                                                  const QString& fragmentSource)
{
    QOpenGLShaderProgram* shader = new QOpenGLShaderProgram();
    
    if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource)) {
        qDebug() << "Failed to compile vertex shader:" << name;
        qDebug() << "Error:" << shader->log();
        delete shader;
        return nullptr;
    }
    
    if (!shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
        qDebug() << "Failed to compile fragment shader:" << name;
        qDebug() << "Error:" << shader->log();
        delete shader;
        return nullptr;
    }
    
    if (!shader->link()) {
        qDebug() << "Failed to link shader:" << name;
        qDebug() << "Error:" << shader->log();
        delete shader;
        return nullptr;
    }
    
    return shader;
}

QOpenGLShaderProgram* ShaderManager::getShader(const QString& name)
{
    return m_shaders.value(name, nullptr);
}

bool ShaderManager::hasShader(const QString& name) const
{
    return m_shaders.contains(name);
}

QStringList ShaderManager::getShaderNames() const
{
    return m_shaders.keys();
}