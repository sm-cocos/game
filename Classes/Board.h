#pragma once

#include <unordered_set>

#include "Tile.h"
#include "2d/CCLayer.h"

namespace cocos2d
{
    class Sprite;
}

class Tile;

class Board final : public cocos2d::Layer
{
    static constexpr float ONE_BLOCK_FALL_TIME = 0.15f;
    static constexpr float EASE_IN_RATE = 2.0f;
    static constexpr int MIN_ISLAND_SIZE = 3;
public:
    struct TilesRemoveCallbackData
    {
        int count{};
        TileType tileType{};
    };
    using onTilesRemoveCallback = std::function<void(const TilesRemoveCallbackData&)>;
public:
    static Board* create(const cocos2d::Size& size, int tilesX, int tilesY, int colorCount);

    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) override;

    void setOnTileRemoveCallback(onTilesRemoveCallback&& callback) { _tileRemoveCallback = callback; }

    bool hasValidMoves() const { return _hasValidMoves; }
        
    void lockBoard() { _isLocked = true; }
    void unlockBoard() { _isLocked = false; }
private:
    bool initWithSizeAndTileInfo(const cocos2d::Size& size, int tilesX, int tilesY, int colorCount);
    void initTileRegistry();
    void randomizeTiles();

    cocos2d::Color3B getColorFromPalette(float t);

    int getTileIndex(int x, int y) const { return x + y * _tilesX; }

    int getTileIndexFromPosition(const cocos2d::Vec2& position) const;
    cocos2d::Vec2 getRelativeTilePositionFrom2dIndex(int x, int y) const;
    void addDirectNeighboursOf(int tileIndex, std::vector<int>& neighbours) const;
    std::vector<int> getTileIsland(int tileIndex) const;
    std::vector<int> getTileIsland(int tileIndex, std::unordered_set<int>& visited) const;
    void removeTileIsland(int tileIndex);
    void fillInTheGaps();

    bool isLocked() const { return _isLocked || _isFillGapsLocked; }
    bool updateHasLegalMoves() const;
private:
    cocos2d::Size _size{};
    cocos2d::Size _tileSize{};

    int _tilesX{0};
    int _tilesY{0};

    int _colorCount{0};

    std::vector<Tile*> _tilesFlat;
    cocos2d::Sprite* _backbone{nullptr};

    onTilesRemoveCallback _tileRemoveCallback{[](const TilesRemoveCallbackData&){}};
    
    // is set to true while gaps are being filled, blocks input
    bool _isFillGapsLocked{false};
    bool _isLocked{true};

    bool _hasValidMoves{true};
};
