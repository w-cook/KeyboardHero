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

void runGameplay(
    RenderWindow &gameWindow,
    GameplayState &gameState,
    const Font &font,
    Clock &frameClock,
    AudioPlayer &audio) {
  while (gameWindow.isOpen()) {
    float dt = frameClock.getElapsedTime().asSeconds();

    if (dt >= 1.f / 60.f) {
      frameClock.restart();

      while (const auto event = gameWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          gameWindow.close();
          audio.stopGameplayMusic();
        }

        if (auto *key = event->getIf<Event::KeyPressed>()) {
          handleGameplayControls(
              key->scancode,
              gameWindow,
              audio,
              gameState.isPaused);

          const TypingHand typingHand = classifyTypingKey(key->scancode);

          if (typingHand == TypingHand::Left) {
            handleLeftTypingKey(
                key->scancode,
                gameState.leftLetterIsInTargetZone,
                gameState.leftFallingLetters,
                gameState.leftFeedback,
                audio);
          }

          if (typingHand == TypingHand::Right) {
            handleRightTypingKey(
                key->scancode,
                gameState.rightLetterIsInTargetZone,
                gameState.rightFallingLetters,
                gameState.rightFeedback,
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
            dt);

        // Remove off-screen letters and record misses.
        handleOffscreenMisses(
            static_cast<float>(gameWindow.getSize().y),
            gameState.leftFallingLetters,
            gameState.rightFallingLetters,
            gameState.leftFeedback,
            gameState.rightFeedback,
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
        drawTimedFeedback(
            gameWindow,
            gameState.leftFeedback.missText,
            gameState.leftFeedback.showMissMessage,
            gameState.leftFeedback.messageClock);

        drawTimedFeedback(
            gameWindow,
            gameState.rightFeedback.missText,
            gameState.rightFeedback.showMissMessage,
            gameState.rightFeedback.messageClock);

        drawTimedFeedback(
            gameWindow,
            gameState.leftFeedback.correctText,
            gameState.leftFeedback.showCorrectMessage,
            gameState.leftFeedback.messageClock);

        drawTimedFeedback(
            gameWindow,
            gameState.rightFeedback.correctText,
            gameState.rightFeedback.showCorrectMessage,
            gameState.rightFeedback.messageClock);

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
}