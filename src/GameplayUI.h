#pragma once

#include <SFML/Graphics.hpp>

struct GameplayState;

// Gameplay presentation initialization
void initializeGameplayUI(
    sf::Vector2u windowSize,
    GameplayState &gameState);

// Target-zone detection
bool isLetterInTargetZone(
    const sf::Text &letter,
    const sf::RectangleShape &targetBox);