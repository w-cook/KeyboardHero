#pragma once

#include "FallingLetters.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

class AudioPlayer;
struct GameplayState;

// Per-hand scoring and feedback state
struct HandFeedbackState {
  int streak = 0;
  int missStreak = 0;

  sf::Text missText;
  sf::Text correctText;

  bool showMissMessage = false;
  bool showCorrectMessage = false;

  sf::Clock messageClock;

  explicit HandFeedbackState(const sf::Font &font)
      : missText(font),
        correctText(font) {}
};

// Scoring and feedback
void recordMiss(
    HandFeedbackState &feedback,
    AudioPlayer &audio);

void recordHit(
    HandFeedbackState &feedback,
    AudioPlayer &audio);

// Render active feedback messages
void drawGameplayFeedback(
    sf::RenderWindow &window,
    GameplayState &gameState);

// Off-screen miss handling
void handleOffscreenMisses(
    float windowHeight,
    GameplayState &gameState,
    AudioPlayer &audio);