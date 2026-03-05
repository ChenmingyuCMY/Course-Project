#include "GameMap.h"
#include <QRandomGenerator>
#include <QDebug>

PerlinNoise::PerlinNoise(int seed)
{
    std::vector<int> p(256);
    std::iota(p.begin(), p.end(), 0);
    std::shuffle(p.begin(), p.end(), std::mt19937{(unsigned int)seed});
        
    for (int i = 0; i < 256; i++) {
        permutation[i] = p[i];
        permutation[i ^ 256] = p[i];
    }
}
double PerlinNoise::noise(double x, double y) const
{
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    
    // 获取相对坐标
    x -= floor(x);
    y -= floor(y);
    
    // 计算缓动曲线
    double u = fade(x);
    double v = fade(y);
    
    // 获取方格的四个角的哈希值
    int aa = permutation[permutation[X] + Y];
    int ab = permutation[permutation[X] + Y + 1];
    int ba = permutation[permutation[X + 1] + Y];
    int bb = permutation[permutation[X + 1] + Y + 1];
    
    // 插值计算
    double y1 = lerp(u, grad(aa, x, y    ), grad(ba, x - 1, y    ));
    double y2 = lerp(u, grad(ab, x, y - 1), grad(bb, x - 1, y - 1));
    
    return (lerp(v, y1, y2) + 1.0) / 2.0;  // 映射到0-1范围
}


GameMap::GameMap(int width, int height, QObject *parent)
    : QObject(parent)
    , mapWidth(width)
    , mapHeight(height)
{
    // 初始化网格
    tiles.resize(height);
    depth.resize(height);
    for (int y = 0; y < height; ++y) {
        tiles[y].resize(width);
        depth[y].resize(width);
    }
}

GameMap::~GameMap()
{
}

void GameMap::initialize(SceneType type, int level, int seed)
{
    this->sceneType = type;
    this->level = level;
    this->seed = seed;
    srand(seed);
    // 清空地图
    clear();
    
    // 根据场景类型生成地形
    generateTerrain();

}

void GameMap::clear()
{
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            tiles[y][x] = MapCell();
        }
    }
}

void GameMap::generateTerrain()
{
    PerlinNoise perlin(seed);
    const double scale = 0.03;           // 缩放因子
    const int octaves = 2;                // 八度数量
    const double persistence = 0.5;       // 振幅衰减
    const double lacunarity = 2.0;      // 频率增加

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            // 基础噪声值
            double nx = x * scale;
            double ny = y * scale;
            
            // 分形布朗运动
            double noiseValue = 0.0;
            double amplitude = 1.0;
            double frequency = 1.0;
            double maxAmplitude = 0.0;
            
            // 叠加噪声
            for (int octave = 0; octave < octaves; octave++) {
                noiseValue += amplitude * perlin.noise(
                    nx * frequency, 
                    ny * frequency
                );
                maxAmplitude += amplitude;
                amplitude *= persistence;
                frequency *= lacunarity;
            }
            
            // 归一化到0-1
            noiseValue /= maxAmplitude;
            
            // 增加一些扰动
            // double nx2 = x * scale * 2.0 + 100.0;
            // double ny2 = y * scale * 2.0 + 100.0;
            // double disturbance = perlin.noise(nx2, ny2) * 0.1 - 0.05;
            // noiseValue += disturbance;
            
            // 确保值在有效范围内
            noiseValue = std::clamp(noiseValue, 0.0, 1.0);
            
            // 根据噪声值设置地形类型
            if (noiseValue < 0.2){
                tiles[y][x].terrain = TerrainType::Water;   
            } else if (noiseValue < 0.6) {
                tiles[y][x].terrain = TerrainType::Plain;       // 平原
            } else if (noiseValue < 0.9) {
                tiles[y][x].terrain = TerrainType::Forest;      // 森林
            } else {
                tiles[y][x].terrain = TerrainType::stone;
            }
        }
    }

    postProcessTerrain();
}

void GameMap::postProcessTerrain()
{

}

const MapCell* GameMap::getCell(int x, int y) const
{
    if (!isValidCoord(x, y)) return nullptr;
    return &tiles[y][x];
}

bool GameMap::isValidCoord(int x, int y) const
{
    return x >= 0 && x < mapWidth && y >= 0 && y < mapHeight;
}

TerrainType GameMap::getTerrain(int x, int y) const
{
    if (!isValidCoord(x, y)) return TerrainType::Plain;
    return tiles[y][x].terrain;
}

void GameMap::setTerrain(int x, int y, TerrainType terrain)
{
    if (!isValidCoord(x, y)) return;
    tiles[y][x].terrain = terrain;
}

bool GameMap::build(int x, int y, BuildingType type)
{
    if (!isValidCoord(x, y)) return false;
    
    MapCell &cell = tiles[y][x];
    if (!cell.canBuild(type)) return false;
    
    cell.building = type;
    cell.isOccupied = true;
    cell.buildingLevel = 1;
    cell.health = 100.0f;
    
    return true;
}

bool GameMap::removeBuilding(int x, int y)
{
    if (!isValidCoord(x, y)) return false;
    
    MapCell &cell = tiles[y][x];
    if (!cell.hasBuilding()) return false;
    
    cell.building = BuildingType::None;
    cell.isOccupied = false;
    cell.buildingLevel = 0;
    
    return true;
}

bool GameMap::upgradeBuilding(int x, int y)
{
    if (!isValidCoord(x, y)) return false;
    
    MapCell &cell = tiles[y][x];
    if (!cell.hasBuilding()) return false;
    
    cell.buildingLevel++;
    
    return true;
}

BuildingType GameMap::getBuilding(int x, int y) const
{
    if (!isValidCoord(x, y)) return BuildingType::None;
    return tiles[y][x].building;
}

bool GameMap::hasBuilding(int x, int y) const
{
    if (!isValidCoord(x, y)) return false;
    return tiles[y][x].hasBuilding();
}

float GameMap::getBuildingHealth(int x, int y) const
{
    if (!isValidCoord(x, y)) return 0.0f;
    return tiles[y][x].health;
}

void GameMap::damageBuilding(int x, int y, int damage)
{
    if (!isValidCoord(x, y)) return;
    
    MapCell &cell = tiles[y][x];
    if (!cell.hasBuilding()) return;
    
    cell.health -= damage;
    
    if (cell.health <= 0) {
        cell.health = 0;
        cell.isOccupied = false;
        cell.isDistroyed = true;
    }
    
}

Resources GameMap::getResourceProduction() const
{
    return resourceStatus;
}

// 工厂方法
GameMap* GameMap::createMap(MapConfig config)
{
    GameMap *map = new GameMap(config.height, config.width);
    map->initialize(config.type, config.level, config.seed);
    return map;
}