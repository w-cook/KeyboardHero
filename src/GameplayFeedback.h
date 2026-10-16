#pragma once

#include "FallingLetters.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

class AudioPlayer;

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
    int &streak,
    int &missStreak,
    sf::Text &missText,
    bool &showMissMessage,
    sf::Clock &messageClock,
    AudioPlayer &audio);

void recordHit(
    int &streak,
    int &missStreak,
    sf::Text &correctText,
    bool &showCorrectMessage,
    sf::Clock &messageClock,
    AudioPlayer &audio);

// Timed feedback rendering
void drawTimedFeedback(
    sf::RenderWindow &window,
    sf::Text &message,
    bool &showMessage,
    const sf::Clock &messageClock);

// Off-screen miss handling
void handleOffscreenMisses(
    float windowHeight,
    std::vector<LeftHandLetter> &leftLetters,
    std::vector<RightHandLetter> &rightLetters,
    HandFeedbackState &leftFeedback,
    HandFeedbackState &rightFeedback,
    AudioPlayer &audio);