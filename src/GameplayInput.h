#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

class AudioPlayer;
struct GameplayState;

// Input classification
enum class TypingHand {
  None,
  Left,
  Right
};

// Gameplay controls
void handleGameplayControls(
    sf::Keyboard::Scancode scancode,
    AudioPlayer &audio,
    bool &isPaused);

TypingHand classifyTypingKey(sf::Keyboard::Scancode scancode);

// Per-hand typing input
void handleLeftTypingKey(
    sf::Keyboard::Scancode scancode,
    GameplayState &gameState,
    AudioPlayer &audio);

void handleRightTypingKey(
    sf::Keyboard::Scancode scancode,
    GameplayState &gameState,
    AudioPlayer &audio);