#pragma once

#include "FallingLetters.h"
#include "GameplayFeedback.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>

class AudioPlayer;

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
    AudioPlayer &audio,
    bool &isPaused);

TypingHand classifyTypingKey(sf::Keyboard::Scancode scancode);

// Per-hand typing input
void handleLeftTypingKey(
    sf::Keyboard::Scancode scancode,
    bool inTarget,
    std::vector<LeftHandLetter> &letters,
    HandFeedbackState &feedback,
    AudioPlayer &audio);

void handleRightTypingKey(
    sf::Keyboard::Scancode scancode,
    bool inTarget,
    std::vector<RightHandLetter> &letters,
    HandFeedbackState &feedback,
    AudioPlayer &audio);