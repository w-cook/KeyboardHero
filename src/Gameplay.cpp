#include "Gameplay.h"

#include "ApplicationState.h"
#include "AudioPlayer.h"
#include "FallingLetters.h"
#include "GameplayFeedback.h"
#include "GameplayInput.h"
#include "GameplayUI.h"

#include <string>

using namespace sf;
using namespace std;

static void resetHandFeedback(HandFeedbackState &feedback) {
  feedback.streak = 0;
  feedback.missStreak = 0;

  feedback.showMissMessage = false;
  feedback.showCorrectMessage = false;

  feedback.missText.setCharacterSize(24);
  feedback.correctText.setCharacterSize(24);

  feedback.messageClock.restart();
}

void resetGameplaySession(GameplayState &gameState) {
  // Remove records from the previous gameplay session.
  gameState.leftFallingLetters.clear();
  gameState.rightFallingLetters.clear();

  // Reset target-zone tracking and pause state.
  gameState.leftLetterIsInTargetZone = false;
  gameState.rightLetterIsInTargetZone = false;
  gameState.isPaused = false;

  // Reset scoring and feedback.
  resetHandFeedback(gameState.leftFeedback);
  resetHandFeedback(gameState.rightFeedback);

  // Reset the streak displays.
  gameState.leftStreakText.setString("Streak: 0");
  gameState.rightStreakText.setString("Streak: 0");

  // Restart spawning for this session.
  gameState.leftSpawnClock.restart();
  gameState.rightSpawnClock.restart();
}

GameScreen runGameplay(
    RenderWindow &gameWindow,
    GameplayState &gameState,
    const Font &font,
    Clock &frameClock,
    AudioPlayer &audio) {
  while (gameWindow.isOpen()) {
    float deltaTime = frameClock.getElapsedTime().asSeconds();

    if (deltaTime >= 1.f / 60.f) {
      frameClock.restart();

      while (const auto event = gameWindow.pollEvent()) {
        // Closing the window requests application shutdown.
        if (event->is<Event::Closed>()) {
          return GameScreen::Exit;
        }

        if (auto *key = event->getIf<Event::KeyPressed>()) {
          // Escape requests a return to the main menu.
          if (key->scancode == Keyboard::Scancode::Escape) {
            return GameScreen::MainMenu;
          }

          handleGameplayControls(
              key->scancode,
              audio,
              gameState.isPaused);

          const TypingHand typingHand = classifyTypingKey(key->scancode);

          if (typingHand == TypingHand::Left) {
            handleLeftTypingKey(
                key->scancode,
                gameState,
                audio);
          }

          if (typingHand == TypingHand::Right) {
            handleRightTypingKey(
                key->scancode,
                gameState,
                audio);
          }
        }
      }

      gameWindow.clear();

      if (!gameState.isPaused) {
        gameWindow.draw(gameState.targetBox);

        // Spawn left-hand letters at the configured interval.
        if (gameState.leftSpawnClock.getElapsedTime().asSeconds() >
            gameState.leftSpawnInterval) {
          spawnLeftLetter(
              gameState.leftFallingLetters,
              font,
              gameWindow.getSize().x,
              gameState.leftLetterSpeed);

          gameState.leftSpawnClock.restart();
        }

        // Spawn right-hand letters at the configured interval.
        if (gameState.rightSpawnClock.getElapsedTime().asSeconds() >
            gameState.rightSpawnInterval) {
          spawnRightLetter(
              gameState.rightFallingLetters,
              font,
              gameWindow.getSize().x,
              gameState.rightLetterSpeed);

          gameState.rightSpawnClock.restart();
        }

        // Update and draw falling gameplay letters.
        updateAndDrawGameplayLetters(
            gameWindow,
            gameState.leftFallingLetters,
            gameState.rightFallingLetters,
            deltaTime);

        // Remove off-screen letters and record misses.
        handleOffscreenMisses(
            static_cast<float>(gameWindow.getSize().y),
            gameState,
            audio);

        // Check whether the first left-hand letter is in the target zone.
        gameState.leftLetterIsInTargetZone =
            !gameState.leftFallingLetters.empty() &&
            isLetterInTargetZone(
                gameState.leftFallingLetters.front().text,
                gameState.targetBox);

        // Check whether the first right-hand letter is in the target zone.
        gameState.rightLetterIsInTargetZone =
            !gameState.rightFallingLetters.empty() &&
            isLetterInTargetZone(
                gameState.rightFallingLetters.front().text,
                gameState.targetBox);

        // Draw active hit/miss feedback for both hands.
        drawGameplayFeedback(
            gameWindow,
            gameState);

        // Update and draw the gameplay HUD.
        gameState.leftStreakText.setString(
            "Streak: " + to_string(gameState.leftFeedback.streak));

        gameState.rightStreakText.setString(
            "Streak: " + to_string(gameState.rightFeedback.streak));

        gameWindow.draw(gameState.gameInstructionsText);
        gameWindow.draw(gameState.leftStreakText);
        gameWindow.draw(gameState.rightStreakText);
        gameWindow.display();
      }
    }
  }

  return GameScreen::Exit;
}