#include "Tile.h"

USING_NS_CC;

Tile* Tile::create(TileType typeIndex)
{
    Tile* tile = new (std::nothrow) Tile();
    if (tile && tile->initWithFile("tile.png"))
    {
        tile->autorelease();
        tile->initFromRegistry(typeIndex);
        
        return tile;
    }
    CC_SAFE_DELETE(tile);
    
    return nullptr;
}

void Tile::initFromRegistry(TileType typeIndex)
{
    _typeIndex = typeIndex;
    
    auto& desc = TileTypesRegistry::getInstance()->get(typeIndex);

    setColor(desc.color);
}



TileTypesRegistry* TileTypesRegistry::getInstance()
{
    static TileTypesRegistry registry = {};
    return &registry;
}

void TileTypesRegistry::add(const Description& description)
{
    auto it = std::find_if(_descriptions.begin(), _descriptions.end(), [&](auto& other)
    {
        return other.color == description.color;
    });
    
    if (it == _descriptions.end())
        _descriptions.push_back(description);
}

const TileTypesRegistry::Description& TileTypesRegistry::get(TileType typeIndex) const
{
    static Description fallback = {};

    if (typeIndex < (TileType)_descriptions.size())
        return _descriptions[typeIndex];

    return fallback;
}


