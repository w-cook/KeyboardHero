#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

class AudioPlayer;
struct GameplayState;
enum class GameScreen;

// Prepare a fresh gameplay session while preserving difficulty settings
void resetGameplaySession(GameplayState &gameState);

// Run the gameplay loop and return the requested screen
GameScreen runGameplay(
    sf::RenderWindow &gameWindow,
    GameplayState &gameState,
    const sf::Font &font,
    sf::Clock &frameClock,
    AudioPlayer &audio);