#pragma once

#include "FallingLetters.h"
#include "GameplayFeedback.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>

// Input classification
enum class TypingHand {
  None,
  Left,
  Right
};

// Gameplay controls
void handleGameplayControls(
    sf::Keyboard::Scancode scancode,
    sf::RenderWindow &gameWindow,
    sf::Sound &gameSong,
    bool &isPaused,
    bool &isMuted);

TypingHand classifyTypingKey(sf::Keyboard::Scancode scancode);

// Per-hand typing input
void handleLeftTypingKey(
    sf::Keyboard::Scancode scancode,
    bool inTarget,
    std::vector<LeftHandLetter> &letters,
    HandFeedbackState &feedback,
    sf::Sound &missSound,
    sf::Sound &correctSound);

void handleRightTypingKey(
    sf::Keyboard::Scancode scancode,
    bool inTarget,
    std::vector<RightHandLetter> &letters,
    HandFeedbackState &feedback,
    sf::Sound &missSound,
    sf::Sound &correctSound);