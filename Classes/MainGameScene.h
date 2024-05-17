#pragma once

#include "Board.h"
#include "2d/CCScene.h"

namespace cocos2d
{
    class Label;
}

enum class MainGameState
{
    GameActive, GamePaused, GameOver, Settings
};

class MainGameScene final : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    bool init() override;

    void update(float delta) override;

    CREATE_FUNC(MainGameScene)
private:
    void resetBoard();

    void onBoardRemoveTiles(const Board::TilesRemoveCallbackData& tilesData);
    void updateScore(int newScore);
    void onGameStateChange(MainGameState newState);

    void onGameActive();
    void onGamePause();
    void onGameOver();
    void onSettings();

    void createGameActiveUI();
    void createGamePauseUI();
    void createGameOverUI();
    void createSettingsUI();

    void resetStateDependentData();
private:
    Board* _board{nullptr};
    cocos2d::Node* _stateDependentData{nullptr};

    int _scoreValue{0};
    cocos2d::Label* _scoreLabel{nullptr};

    int _boardTilesX{0};
    int _boardTilesY{0};
    int _boardColors{0};
    bool _boardIsDirty{false};
    
    MainGameState _state{MainGameState::GamePaused};
};
