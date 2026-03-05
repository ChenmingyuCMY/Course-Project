#ifndef MAPFACTORY_H
#define MAPFACTORY_H

#include <QObject>
#include <QMap>
#include "GameMap.h"
#include "SceneType.h"

class MapFactory : public QObject
{
    Q_OBJECT
    
public:
    explicit MapFactory(QObject *parent = nullptr);
    ~MapFactory();
    
    // 创建地图
    static GameMap* createMap(SceneType sceneType, int level);
    static GameMap* createMap(SceneType sceneType, int level, int seed);
    
    static MapConfig getMapConfig(SceneType sceneType, int level, int seed);

};

#endif // MAPFACTORY_H
