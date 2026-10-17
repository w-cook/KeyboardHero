#pragma once

#include "FallingLetters.h"
#include "GameplayFeedback.h"
#include "Menu.h"

#include <SFML/System/Clock.hpp>
#include <cstddef>
#include <vector>

// Application navigation states
enum class GameScreen {
    MainMenu,
    Gameplay,
    Exit
};

// Main-menu session state
struct MenuState {
  std::vector<BackgroundLetter> backgroundLetters;

  sf::Clock backgroundSpawnClock;
  sf::Clock loadingClock;

  std::size_t backgroundLettersSpawned = 0;
  float backgroundSpawnInterval = 0.2f;
  bool showLoadingMessage = true;

  // Menu Presentation
  sf::Text titleText;
  sf::Text menuText;
  sf::Text loadingText;

  explicit MenuState(const sf::Font &font)
      : titleText(font),
        menuText(font),
        loadingText(font) {}
};

// Gameplay session state
struct GameplayState {
  std::vector<LeftHandLetter> leftFallingLetters;
  std::vector<RightHandLetter> rightFallingLetters;

  sf::Clock leftSpawnClock;
  sf::Clock rightSpawnClock;

  // Difficulty-dependent letter settings
  float leftLetterSpeed = 0.f;
  float rightLetterSpeed = 0.f;
  float leftSpawnInterval = 0.f;
  float rightSpawnInterval = 0.f;

  bool leftLetterIsInTargetZone = false;
  bool rightLetterIsInTargetZone = false;

  bool isPaused = false;

  HandFeedbackState leftFeedback;
  HandFeedbackState rightFeedback;

  // Gameplay presentation
  sf::Text gameInstructionsText;
  sf::Text leftStreakText;
  sf::Text rightStreakText;

  sf::RectangleShape targetBox;

  explicit GameplayState(const sf::Font &font)
    : leftFeedback(font),
      rightFeedback(font),
      gameInstructionsText(font),
      leftStreakText(font),
      rightStreakText(font) {}
};