#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <QObject>
#include <QVector>
#include <QPoint>
#include <QRect>
#include <QPainter>
#include "SceneType.h"

// 地形类型
enum class TerrainType {
    Plain,     
    Forest,     
    Water,
    stone
};

// 地图配置
struct MapConfig {
    int width = 20;
    int height = 20;
    int level = 1;
    int seed = 0;
    SceneType type = SceneType::Empty;
};

struct Resources{
    int food = 0;
    int wood = 0;
    int stone = 0;
    
    int foodinc = 0;
    int woodinc = 0;
    int stoneinc = 0;
};

// 建筑类型
enum class BuildingType {
    None,       // 无建筑
    Base,       // 基地
    House,      // 房屋
    Farm,       // 农场
    LumberMill, // 伐木场
    StoneMine,  // 石矿
    Wall,       // 城墙
    Tower       // 塔
};

// 网格单元
struct MapCell {
    TerrainType terrain = TerrainType::Plain;
    BuildingType building = BuildingType::None;
    bool isOccupied = false;
    bool isDistroyed = false;
    int buildingLevel = 0;      // 建筑等级
    int health = 0;      // 建筑生命值
    
    // 是否有建筑
    bool hasBuilding() const { return building != BuildingType::None; }
    
    // 是否可以建造
    bool canBuild(BuildingType type) const {
        if (isOccupied || hasBuilding()) return false;
        
        // 根据地形限制建筑
        switch (terrain) {
        case TerrainType::Plain:  // 平地可建其他任何建筑
            return type != BuildingType::StoneMine && 
                   type != BuildingType::LumberMill; 
        case TerrainType::Forest: // 森林只能建伐木场
            return type == BuildingType::LumberMill;  
        case TerrainType::stone:  // 石头只能建采石场
            return type == BuildingType::StoneMine;
        case TerrainType::Water:  // 水池不能建建筑
            return false;  
        default:
            return false;
        }
    }
};

class PerlinNoise {
public:
    explicit PerlinNoise(int seed);
    ~PerlinNoise(){};
    
    // 获取排列值
    int getIndex(int i) const { return permutation[i & 255]; };
    
    // 梯度函数
    double grad(int hash, double x, double y) const {
        int h = hash & 7;
        double u = h < 4 ? x : y;
        double v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    };
    
    // 线性插值
    double lerp(double t, double a, double b) const {
        return a + t * (b - a);
    };
    
    // 缓动函数
    double fade(double t) const {
        return t * t * t * (t * (t * 6 - 15) + 10);
    };
    // 2D柏林噪声
    double noise(double x, double y) const ;
private:
    int permutation[512];
};

class GameMap : public QObject
{
    Q_OBJECT
    
public:
    explicit GameMap(int width = 20, int height = 20, QObject *parent = nullptr);
    ~GameMap();
    
    // 地图初始化
    void initialize(SceneType sceneType, int level, int seed);
    void clear();
    
    // 地图信息
    int getWidth() const { return mapWidth; }
    int getHeight() const { return mapHeight; }
    SceneType getSceneType() const { return sceneType; }
    int getLevel() const { return level; }
    const std::vector<std::vector<MapCell> >* const getTiles(){return &tiles;};
    
    // 单元格访问
    MapCell* getCell(int x, int y);
    const MapCell* getCell(int x, int y) const;
    bool isValidCoord(int x, int y) const;
    
    // 地形操作
    TerrainType getTerrain(int x, int y) const;
    void setTerrain(int x, int y, TerrainType terrain);
    
    // 建筑操作
    bool build(int x, int y, BuildingType type);
    bool removeBuilding(int x, int y);
    bool upgradeBuilding(int x, int y);
    BuildingType getBuilding(int x, int y) const;
    bool hasBuilding(int x, int y) const;
    void damageBuilding(int x, int y, int damage);
    
    // 建筑属性
    float getBuildingHealth(int x, int y) const;
    void damageBuilding(int x, int y, float damage);
    bool isBuildingDestroyed(int x, int y) const;
    
    // 资源相关
    Resources getResourceProduction() const;
    
    // 工厂方法
    static GameMap* createMap(MapConfig config);
    
private:
    void generateTerrain();  // 生成地形
    void postProcessTerrain();
    Resources resourceStatus;

    int mapWidth;
    int mapHeight;
    SceneType sceneType;
    int level;
    int seed;

    std::vector<std::vector<int> > depth;
    std::vector<std::vector<MapCell> > tiles;  // 二维网格
};

#endif // GAMEMAP_H