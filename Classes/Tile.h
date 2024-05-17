#pragma once
#include "2d/CCSprite.h"

using TileType = int;

class Tile final : public cocos2d::Sprite
{
public:
    static Tile* create(TileType typeIndex);
    TileType getTileTypeIndex() const { return _typeIndex; }
private:
    void initFromRegistry(TileType typeIndex);
private:
    TileType _typeIndex{0};
};


class TileTypesRegistry
{
public:
    struct Description
    {
        cocos2d::Color3B color{cocos2d::Color3B::WHITE};
    };
public:
    static TileTypesRegistry* getInstance();
    void add(const Description& description);
    const Description& get(TileType typeIndex) const;
private:
    std::vector<Description> _descriptions;
};