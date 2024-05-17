#include "Board.h"

#include <array>

#include "Tile.h"
#include "2d/CCActionEase.h"
#include "2d/CCActionInstant.h"
#include "2d/CCActionInterval.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerTouch.h"

USING_NS_CC;

Board* Board::create(const Size& size, int tilesX, int tilesY, int colorCount)
{
    Board* board = new (std::nothrow) Board();
    if (board && board->initWithSizeAndTileInfo(size, tilesX, tilesY, colorCount))
    {
        board->autorelease();
        
        return board;
    }
    CC_SAFE_DELETE(board);
    
    return nullptr;
}

bool Board::onTouchBegan(Touch* touch, Event* event)
{
    if (isLocked() || !hasValidMoves())
        return false;
    
    if (!event->getCurrentTarget()->getBoundingBox().containsPoint(touch->getLocation()))
        return false;

    int tileIndex = getTileIndexFromPosition(touch->getLocation());
    if (_tilesFlat[tileIndex])
        removeTileIsland(tileIndex);
    
    return true;
}

bool Board::initWithSizeAndTileInfo(const Size& size, int tilesX, int tilesY, int colorCount)
{
    if (!Layer::init())
        return false;

    _size = size;
    _tileSize = Vec2{size.width / (float)tilesX, size.height / (float)tilesY};
    _tilesX = tilesX;
    _tilesY = tilesY;
    _colorCount = colorCount;
    _tilesFlat.resize(tilesX * tilesY, nullptr);

    auto director = Director::getInstance();
    Size visibleSize = director->getVisibleSize();
    Vec2 origin = director->getVisibleOrigin();
    Vec2 center = (origin + visibleSize * 0.5f);

    _backbone = Sprite::create("background.png");
    _backbone->setPosition(center);
    _backbone->setContentSize(size);
    _backbone->setColor(Color3B{200, 200, 200});
    addChild(_backbone);
    
    initTileRegistry();
    while(!updateHasLegalMoves())
        randomizeTiles();

    for (auto* tile : _tilesFlat)
        _backbone->addChild(tile);

    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = [this](Touch* touch, Event* event)
    {
        return onTouchBegan(touch, event);
    };
    director->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, _backbone);
    
    return true;
}

void Board::initTileRegistry()
{
    using Description = TileTypesRegistry::Description;
    for (int i = 0; i < _colorCount; i++)
    {
        float t = (float)i / (float)_colorCount;
        Description description = {};
        description.color = getColorFromPalette(t);

        TileTypesRegistry::getInstance()->add(description);
    }
}

void Board::randomizeTiles()
{
    Size contentSize = _backbone->getContentSize();

    Size tileSize = Size{contentSize.width / (float)_tilesX, contentSize.height / (float)_tilesY};

    for (int y = 0; y < _tilesY; y++)
    {
        for (int x = 0 ; x < _tilesX; x++)
        {
            int tileIndex = x + y * _tilesX;

            TileType type = (TileType)RandomHelper::random_int(0, _colorCount - 1);
            Tile* tile = Tile::create(type);
            tile->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

            Vec2 position = Vec2{(float)x * tileSize.width, (float)y * tileSize.height};
            tile->setPosition(position);
            tile->setContentSize(tileSize);

            _tilesFlat[tileIndex] = tile;
        }
    }
}

Color3B Board::getColorFromPalette(float t)
{
    // see https://iquilezles.org/articles/palettes/

    static const Vec3 A = {0.5f, 0.5f, 0.5f};
    static const Vec3 B = {0.5f, 0.5f, 0.5f};
    static const Vec3 C = {1.0f, 1.0f, 1.0f};
    static const Vec3 D = {0.0f, 0.33f, 0.67f};

    Vec3 color = A + Vec3{
        B.x * std::cos(6.18f * (C.x * t + D.x)),
        B.y * std::cos(6.18f * (C.y * t + D.y)),
        B.z * std::cos(6.18f * (C.z * t + D.z))};

    return Color3B{
        (uint8_t)(255 * color.x),
        (uint8_t)(255 * color.y), 
        (uint8_t)(255 * color.z)};
}

int Board::getTileIndexFromPosition(const Vec2& position) const
{
    Size contentSize = _backbone->getContentSize();
    Vec2 origin = _backbone->getPosition();
    Vec2 bottomLeft = origin - contentSize * 0.5f;

    Vec2 delta = position - bottomLeft;
    int tileX = (int)(delta.x / _tileSize.width);
    int tileY = (int)(delta.y / _tileSize.height);

    return getTileIndex(tileX, tileY);
}

Vec2 Board::getRelativeTilePositionFrom2dIndex(int x, int y) const
{
    return Vec2{(float)x * _tileSize.width, (float)y * _tileSize.height};
}


void Board::addDirectNeighboursOf(int tileIndex, std::vector<int>& neighbours) const
{
    TileType tileType = _tilesFlat[tileIndex]->getTileTypeIndex();
    
    int tileX = tileIndex % _tilesX;
    int tileY = tileIndex / _tilesX;

    for (int y : std::array<int, 2>{-1, 1})
    {
        int neighbourY = tileY + y;
        if (neighbourY < 0 || neighbourY >= _tilesY)
            continue;
        
        int neighbourIndex = getTileIndex(tileX, neighbourY);
        if (_tilesFlat[neighbourIndex] && _tilesFlat[neighbourIndex]->getTileTypeIndex() == tileType)
            neighbours.push_back(neighbourIndex);
    }
    for (int x : std::array<int, 2>{-1, 1})
    {
        int neighbourX = tileX + x;
        if (neighbourX < 0 || neighbourX >= _tilesX)
            continue;
        
        int neighbourIndex = getTileIndex(neighbourX, tileY);
        if (_tilesFlat[neighbourIndex] && _tilesFlat[neighbourIndex]->getTileTypeIndex() == tileType)
            neighbours.push_back(neighbourIndex);
    }
}

std::vector<int> Board::getTileIsland(int tileIndex) const
{
    std::unordered_set<int> visited = {};
    
    return getTileIsland(tileIndex, visited);
}

std::vector<int> Board::getTileIsland(int tileIndex, std::unordered_set<int>& visited) const
{
    std::vector<int> islandIndices = {};

    // find island by dfs
    auto dfs = [this, &visited](int start, std::vector<int>& indices)
    {
        std::vector<int> toVisit = {start};
        
        while (!toVisit.empty())
        {
            int current = toVisit.back(); toVisit.pop_back();
            if (visited.find(current) == visited.end())
            {
                indices.push_back(current);
                
                addDirectNeighboursOf(current, toVisit);
                visited.emplace(current);
            }
        }
    };

    dfs(tileIndex, islandIndices);

    return islandIndices;
}

void Board::removeTileIsland(int tileIndex)
{
    std::vector<int> islandIndices = getTileIsland(tileIndex);
    
    if (islandIndices.size() >= MIN_ISLAND_SIZE)
    {
        // first call callback while tiles are still valid
        _tileRemoveCallback({
            (int)islandIndices.size(),
            _tilesFlat[tileIndex]->getTileTypeIndex()});
        
        for (int index : islandIndices)
        {
            _tilesFlat[index]->removeFromParent();
            _tilesFlat[index] = nullptr;
        }

        fillInTheGaps();
        _hasValidMoves = updateHasLegalMoves();
    }    
}

void Board::fillInTheGaps()
{    
    struct GapInfo
    {
        int top{0};
        int bottom{0};
    };
    // each column may have multiple gaps
    std::vector<std::vector<GapInfo>> gaps;
    gaps.resize(_tilesX);

    // locate gaps for each column
    for (int y = 0; y < _tilesY; y++)
    {
        for (int x = 0; x < _tilesX; x++)
        {
            int tileIndex = getTileIndex(x, y);
            if (!_tilesFlat[tileIndex])
            {
                // if new gap is found
                if (gaps[x].empty() || gaps[x].back().top != y - 1)
                    gaps[x].push_back({y, y});
                else
                    gaps[x].back().top++;
            }
        }
    }

    // we will block the further modification of grid, while ANY tile is in falling state
    int maxFallHeight = 0;
    
    for (int x = 0; x < _tilesX; x++)
    {
        int droppedCount = 0;
        for (int gapIndex = 0; gapIndex < (int)gaps[x].size(); gapIndex++)
        {
            auto& gap = gaps[x][gapIndex];
            int gapHeight = gap.top - gap.bottom + 1;
            if (gapHeight < 0 || gapHeight == _tilesY)
                continue;

            // drop all the tiles above the gap
            int lastToDrop = gapIndex + 1 < (int)gaps[x].size() ?
                gaps[x][gapIndex + 1].bottom :
                _tilesY;
            
            for (int y = gap.top + 1; y < lastToDrop; y++)
            {
                int tileIndex = getTileIndex(x, y);
                
                int targetY = gaps[x].front().bottom + droppedCount;
                int targetIndex = getTileIndex(x, targetY);
                Vec2 targetPosition = getRelativeTilePositionFrom2dIndex(x, targetY);

                int fallHeight = y - targetY;
                maxFallHeight = std::max(maxFallHeight, fallHeight);
                auto moveAction = EaseIn::create(
                    MoveTo::create(ONE_BLOCK_FALL_TIME * (float)fallHeight, targetPosition),
                    EASE_IN_RATE);

                std::swap(_tilesFlat[tileIndex], _tilesFlat[targetIndex]);
                _tilesFlat[targetIndex]->runAction(moveAction);
                droppedCount++;
            }
        }
    }

    auto block = CallFunc::create([this](){ _isFillGapsLocked = true; });
    auto unblock = CallFunc::create([this](){ _isFillGapsLocked = false; });
    auto delay = DelayTime::create((float)maxFallHeight * ONE_BLOCK_FALL_TIME);
    auto blockModificationSequence = Sequence::create(block, delay, unblock, nullptr);
    runAction(blockModificationSequence);
}

bool Board::updateHasLegalMoves() const
{
    std::unordered_set<int> visited = {};

    for (int i = 0; i < (int)_tilesFlat.size(); i++)
        if (_tilesFlat[i] && visited.find(i) == visited.end())
            if (getTileIsland(i, visited).size() >= MIN_ISLAND_SIZE)
                return true;

    return false;
}
