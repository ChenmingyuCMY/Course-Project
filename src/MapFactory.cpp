#include "MapFactory.h"
#include <QRandomGenerator>
#include <QDebug>

MapFactory::MapFactory(QObject *parent)
    : QObject(parent)
{
}

MapFactory::~MapFactory()
{
    
}

GameMap* MapFactory::createMap(SceneType sceneType, int level)
{
    std::random_device rd;
    std::mt19937 gen(rd()); 
    int random = gen();
    return MapFactory::createMap(sceneType, level, random);
}

GameMap* MapFactory::createMap(SceneType sceneType, int level, int seed )
{
    
    // 创建新地图
    MapConfig config = getMapConfig(sceneType, level, seed);

    GameMap *map = GameMap::createMap(config);
    
    // 根据难度调整地图属性
    
    qDebug() << "Created new map for" << static_cast<int>(sceneType) 
             << "level" << level;
    
    return map;
}

MapConfig MapFactory::getMapConfig(SceneType sceneType, int level, int seed) 
{
    MapConfig config;
    
    // 基础配置
    config.width = 50;
    config.height = 50;
    config.level = level;
    config.type = sceneType;
    config.seed = seed;

    return config;
}