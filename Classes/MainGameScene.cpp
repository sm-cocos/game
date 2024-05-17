#include "MainGameScene.h"

#include "2d/CCLabel.h"
#include "2d/CCMenu.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"

USING_NS_CC;

namespace
{
    constexpr int TILES_X = 16;
    constexpr int TILES_Y = 10;
    constexpr float BOARD_PADDING = 0.05f;
    constexpr int COLORS = 3;

    constexpr int MAX_TILES = 32;
    constexpr int MAX_COLORS = 6;

    constexpr int OVERLAY_Z_ORDER = 1;
    constexpr int UI_Z_ORDER = 2;

    const Color4B OVERLAY_COLOR = Color4B{0, 0, 0, 125};

    const std::string FONT_PATH = "fonts/Roboto-Regular.ttf";

    constexpr int UI_FONT_SIZE_SMALL = 32;
    constexpr int UI_FONT_SIZE = 64;
    constexpr int UI_FONT_BIG = 80;
    constexpr int UI_SPACING = UI_FONT_SIZE * 2;
}


Scene* MainGameScene::createScene()
{
    return create();
}

bool MainGameScene::init()
{
    if (!Scene::init())
        return false;

    _boardTilesX = TILES_X;
    _boardTilesY = TILES_Y;
    _boardColors = COLORS;

    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto background = Sprite::create("background.png");
    background->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    background->setContentSize(visibleSize);
    addChild(background);

    _scoreLabel = Label::createWithTTF("Score: " + std::to_string(_scoreValue), FONT_PATH, UI_FONT_SIZE_SMALL);
    _scoreLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
    _scoreLabel->setPosition(UI_FONT_SIZE, visibleSize.height - UI_FONT_SIZE_SMALL);
    addChild(_scoreLabel, UI_Z_ORDER);

    resetBoard();
    onGameStateChange(MainGameState::GamePaused);
    
    scheduleUpdate();

    return true;
}

void MainGameScene::update(float delta)
{
    if (_state == MainGameState::GameActive && !_board->hasValidMoves())
    {
        onGameStateChange(MainGameState::GameOver);
    }
}

void MainGameScene::resetBoard()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Size boardSize = visibleSize;
    // modify board size assuming the tile is a square
    float targetAspect = (float)_boardTilesX / (float)_boardTilesY;
    if (targetAspect > 1.0f)
        boardSize = Size{boardSize.width, boardSize.width / targetAspect};
    else
        boardSize = Size{boardSize.height * targetAspect, boardSize.height};
    
    boardSize = boardSize * (1.0f - BOARD_PADDING);

    if (_board)
        _board->removeFromParent();
    _board = Board::create(boardSize, _boardTilesX, _boardTilesY, _boardColors);
    _board->setOnTileRemoveCallback([this](const Board::TilesRemoveCallbackData& tilesData)
    {
        onBoardRemoveTiles(tilesData);
    });
    addChild(_board);
    
    updateScore(0);
}

void MainGameScene::onBoardRemoveTiles(const Board::TilesRemoveCallbackData& tilesData)
{
    // some complex scoring system is possible (e.g. based on type of the tile),
    // but here it is simple count-based scoring function

    // pretty random quadratic function
    updateScore(_scoreValue + 10 * tilesData.count * tilesData.count);
}

void MainGameScene::updateScore(int newScore)
{
    _scoreValue = newScore;
    _scoreLabel->setString("Score: " + std::to_string(newScore));
}

void MainGameScene::onGameStateChange(MainGameState newState)
{
    _state = newState;
    resetStateDependentData();

    switch (newState)
    {
    case MainGameState::GameActive:
        onGameActive();
        break;
    case MainGameState::GamePaused:
        onGamePause();
        break;
    case MainGameState::GameOver:
        onGameOver();
        break;
    case MainGameState::Settings:
        onSettings();
        break;
    default:
        log("unexpected game state");
        break;
    }
}

void MainGameScene::onGameActive()
{
    _board->unlockBoard();
    _board->setVisible(true);
    _scoreLabel->setVisible(true);

    createGameActiveUI();
}

void MainGameScene::onGamePause()
{
    _board->lockBoard();
    _board->setVisible(true);
    _scoreLabel->setVisible(false);

    createGamePauseUI();
}

void MainGameScene::onGameOver()
{
    _board->setVisible(true);
    // instead of the main score, we display new label for 'final score'
    _scoreLabel->setVisible(false);
    
    createGameOverUI();
}

void MainGameScene::onSettings()
{
    _board->setVisible(false);
    _scoreLabel->setVisible(false);
    
    createSettingsUI();
}

void MainGameScene::createGameActiveUI()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = visibleSize * 0.5f;
            
    auto pauseButton = MenuItemLabel::create(
        Label::createWithTTF("Pause Game", FONT_PATH, UI_FONT_SIZE_SMALL),
        [this](Ref*)
        {
            onGameStateChange(MainGameState::GamePaused);
        });
    auto menu = Menu::create(pauseButton, nullptr);
    menu->setPosition(center.x, visibleSize.height - UI_FONT_SIZE_SMALL);
    _stateDependentData->addChild(menu, UI_Z_ORDER);
}

void MainGameScene::createGamePauseUI()
{
    auto startButton = MenuItemLabel::create(
        Label::createWithTTF("Start Game", FONT_PATH, UI_FONT_SIZE),
        [this](Ref*)
        {
            onGameStateChange(MainGameState::GameActive);
        });
    auto newGameButton = MenuItemLabel::create(
        Label::createWithTTF("New Game", FONT_PATH, UI_FONT_SIZE),
        [this](Ref*)
        {
            resetBoard();
        });
    auto toSettingsButton = MenuItemLabel::create(
        Label::createWithTTF("Settings", FONT_PATH, UI_FONT_SIZE),
        [this](Ref*)
        {
            onGameStateChange(MainGameState::Settings);
        });
    newGameButton->setPosition(0.0f, -UI_SPACING);
    toSettingsButton->setPosition(0.0f, -UI_SPACING * 2);
    auto menu = Menu::create(startButton, newGameButton, toSettingsButton, nullptr);
    _stateDependentData->addChild(menu, UI_Z_ORDER);

    auto overlay = LayerColor::create(OVERLAY_COLOR);
    _stateDependentData->addChild(overlay, OVERLAY_Z_ORDER);
}

void MainGameScene::createGameOverUI()
{
    auto newGameButton = MenuItemLabel::create(
        Label::createWithTTF("New Game", FONT_PATH, UI_FONT_SIZE),
        [this](Ref*)
        {
            resetBoard();
            onGameStateChange(MainGameState::GamePaused);
        });
    newGameButton->setPosition(0.0f, -UI_SPACING);
    auto menu = Menu::create(newGameButton, nullptr);
    _stateDependentData->addChild(menu, UI_Z_ORDER);

    Size boardSize = _board->getContentSize();
    auto gameOver = Label::createWithSystemFont("Game Over", FONT_PATH, UI_FONT_BIG);
    gameOver->setPosition(_board->getPosition() + boardSize * 0.5f);
    _stateDependentData->addChild(gameOver, UI_Z_ORDER);

    auto finalScore = Label::createWithTTF("Final Score: " + std::to_string(_scoreValue), FONT_PATH, UI_FONT_BIG);
    finalScore->setPosition(gameOver->getPosition() + Vec2{0, UI_SPACING});
    _stateDependentData->addChild(finalScore, UI_Z_ORDER);
    
    auto overlay = LayerColor::create(OVERLAY_COLOR);
    _stateDependentData->addChild(overlay, OVERLAY_Z_ORDER);
}

void MainGameScene::createSettingsUI()
{
    // this creates a control that looks like that: `'control_name'   '<-' 'value' '->'`
    auto createControl = [this](const std::string& controlName, const Vec2& position, auto initialValue,
        const auto& onDecrease, const auto& onIncrease)
    {
        auto control = Node::create();
        auto controlMenu = Menu::create();
        auto infoLabel = Label::createWithTTF(controlName, FONT_PATH, UI_FONT_SIZE);
        auto contentLabel = Label::createWithTTF(std::to_string(initialValue), FONT_PATH, UI_FONT_SIZE);
        auto decreaseLabel = MenuItemLabel::create(
            Label::createWithTTF("<", FONT_PATH, UI_FONT_SIZE),
            [=](Ref*) mutable
            {
                contentLabel->setString(std::to_string(onDecrease()));
            });
        auto increaseLabel = MenuItemLabel::create(
            Label::createWithTTF(">", FONT_PATH, UI_FONT_SIZE),
            [=](Ref*) mutable
            {
                contentLabel->setString(std::to_string(onIncrease()));
            });

        controlMenu->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        controlMenu->setPosition(Vec2::ZERO);
        infoLabel->setPosition({-UI_SPACING * 3, 0});
        decreaseLabel->setPosition({UI_SPACING, 0});
        contentLabel->setPosition({UI_SPACING * 2, 0});
        increaseLabel->setPosition({UI_SPACING * 3, 0});
    
        controlMenu->addChild(decreaseLabel);
        controlMenu->addChild(increaseLabel);

        control->addChild(controlMenu);
        control->addChild(infoLabel);
        control->addChild(contentLabel);

        control->setPosition(position - control->getContentSize() * 0.5f);

        return control;
    };

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 center = visibleSize * 0.5f;
    
    auto widthControl = createControl("Width", center + Vec2{0, UI_SPACING * 1}, _boardTilesX,
        [this](){ _boardIsDirty = true; _boardTilesX = std::max(_boardTilesX - 1, 1); return _boardTilesX; },
        [this](){ _boardIsDirty = true; _boardTilesX = std::min(_boardTilesX + 1, MAX_TILES); return _boardTilesX; });
    
    auto heightControl = createControl("Height", center + Vec2{0, UI_SPACING * 0}, _boardTilesY,
        [this](){ _boardIsDirty = true; _boardTilesY = std::max(_boardTilesY - 1, 1); return _boardTilesY; },
        [this](){ _boardIsDirty = true; _boardTilesY = std::min(_boardTilesY + 1, MAX_TILES); return _boardTilesY; });
    
    auto colorsControl = createControl("Colors", center - Vec2{0, UI_SPACING * 1}, _boardColors,
        [this](){ _boardIsDirty = true; _boardColors = std::max(_boardColors - 1, 1); return _boardColors; },
        [this](){ _boardIsDirty = true; _boardColors = std::min(_boardColors + 1, MAX_COLORS); return _boardColors; });
    
    auto backButton = MenuItemLabel::create(
        Label::createWithTTF("Back", FONT_PATH, UI_FONT_SIZE),
        [this](Ref*)
        {
            if (_boardIsDirty)
            {
                _boardIsDirty = false;
                resetBoard();
            }
            onGameStateChange(MainGameState::GamePaused);
        });
    backButton->setPosition(0.0f, -UI_SPACING * 2.0f);
    auto menu = Menu::create(backButton, nullptr);
    auto overlay = LayerColor::create(OVERLAY_COLOR);

    _stateDependentData->addChild(overlay, OVERLAY_Z_ORDER);
    _stateDependentData->addChild(widthControl, UI_Z_ORDER);
    _stateDependentData->addChild(heightControl, UI_Z_ORDER);
    _stateDependentData->addChild(colorsControl, UI_Z_ORDER);
    _stateDependentData->addChild(menu, UI_Z_ORDER);
}

void MainGameScene::resetStateDependentData()
{
    if (_stateDependentData)
    {
        _stateDependentData->removeAllChildren();
    }
    else
    {
        Size visibleSize = Director::getInstance()->getVisibleSize();
        _stateDependentData = Node::create();
        _stateDependentData->setContentSize(visibleSize);

        addChild(_stateDependentData, OVERLAY_Z_ORDER);
    }
}
