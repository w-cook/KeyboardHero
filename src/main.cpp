#include "ApplicationState.h"
#include "AssetLoading.h"
#include "FallingLetters.h"
#include "GameplayFeedback.h"
#include "GameplayInput.h"
#include "GameplayUI.h"
#include "Menu.h"
#include "RandomGeneration.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

using namespace sf;
using namespace std;

// Application entry point
int main() {
  // Load the shared font.
  auto fontPath = assetPath("Fonts/FiraCodeNerdFontMono-Light.ttf");
  Font firaCodeFont;
  if (!firaCodeFont.openFromFile(fontPath)) {
    cerr << "Error loading firaCodeFont" << endl;
    return -1;
  }

  MenuState menuState(firaCodeFont);
  GameplayState gameState(firaCodeFont);

  // Create the gameplay window.
  RenderWindow gameWindow(
      VideoMode({
          1920,
          1080}),
      "KeyboardHero");
  gameWindow.setVerticalSyncEnabled(true);

  // Create the main-menu window.
  RenderWindow menuWindow(
      VideoMode({
          1920,
          1080}),
      "Main Menu");
  menuWindow.setVerticalSyncEnabled(true);

  configureMenuTexts(
      menuWindow.getSize(),
      menuState.titleText,
      menuState.menuText,
      menuState.loadingText);

  // Gameplay feedback positions.
  const Vector2u gameSize = gameWindow.getSize();

  const Vector2f leftFeedbackPosition{
      gameSize.x / 9.5f,
      gameSize.y / 2.f};

  const Vector2f rightFeedbackPosition{
      gameSize.x - (gameSize.x / 10.f),
      gameSize.y / 2.f};

  // Left-hand miss feedback.
  configureFeedbackText(
      gameState.leftFeedback.missText,
      "Miss",
      Color::Red,
      leftFeedbackPosition);

  // Right-hand miss feedback.
  configureFeedbackText(
      gameState.rightFeedback.missText,
      "Miss",
      Color::Red,
      rightFeedbackPosition);

  // Left-hand correct-hit feedback.
  configureFeedbackText(
      gameState.leftFeedback.correctText,
      "Correct!",
      Color::Cyan,
      leftFeedbackPosition);

  // Right-hand correct-hit feedback.
  configureFeedbackText(
      gameState.rightFeedback.correctText,
      "Correct!",
      Color::Cyan,
      rightFeedbackPosition);

  configureGameplayHudTexts(
      gameWindow.getSize(),
      gameState.gameInstructionsText,
      gameState.leftStreakText,
      gameState.rightStreakText,
      gameState.leftFeedback.streak,
      gameState.rightFeedback.streak);

  // Create the gameplay target bar.
  gameState.targetBox = createTargetBox(gameWindow.getSize());

  // Load audio resources.
  SoundBuffer titleSongBuffer;
  SoundBuffer titleTransSoundBuffer;
  SoundBuffer gameSongBuffer;
  SoundBuffer correctSoundBuffer;
  SoundBuffer missSoundBuffer;

  loadSoundBuffers(
      titleSongBuffer,
      titleTransSoundBuffer,
      gameSongBuffer,
      correctSoundBuffer,
      missSoundBuffer);

  // Create sound players after their buffers.
  Sound titleSong(titleSongBuffer);
  Sound titleTransSound(titleTransSoundBuffer);
  Sound gameSong(gameSongBuffer);
  Sound correctSound(correctSoundBuffer);
  Sound missSound(missSoundBuffer);

  // Initialize clocks.
  Clock gameClock;
  gameState.leftSpawnClock.restart();
  gameState.rightSpawnClock.restart();

  MenuInputContext menuInput{
      menuWindow,
      gameWindow,
      titleSong,
      titleTransSound,
      gameState.leftSpawnInterval,
      gameState.leftLetterSpeed,
      gameState.rightSpawnInterval,
      gameState.rightLetterSpeed};

  // Start the title music.
  titleSong.setLooping(true);
  titleSong.setVolume(50);
  titleSong.play();

  // Main-menu loop.
  while (menuWindow.isOpen()) {
    float dt = gameClock.getElapsedTime().asSeconds();
    if (dt >= 1.f / 60.f) {
      gameClock.restart();

      while (const auto event = menuWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          menuWindow.close();
          titleSong.stop();
        }

        if (auto *key = event->getIf<sf::Event::KeyPressed>()) {
          handleMenuKeyPress(
              key->scancode,
              menuInput);
        }
      }

      menuWindow.clear();

      menuWindow.draw(menuState.titleText);
      menuWindow.draw(menuState.menuText);

      // Update and draw the animated menu background.
      updateAndDrawMenuBackground(
          menuWindow,
          firaCodeFont,
          menuState.backgroundLetters,
          menuState.backgroundSpawnClock,
          menuState.backgroundSpawnInterval,
          dt);

      if (menuState.showLoadingMessage) {
        menuWindow.draw(menuState.loadingText);
        if (menuState.loadingClock.getElapsedTime().asSeconds() > 3.f) {
          menuState.showLoadingMessage = false;
          menuState.loadingClock.stop();
        }
      }

      menuWindow.display();
    }
  }

  // Start gameplay music.
  gameSong.setLooping(true);
  gameSong.setVolume(60);
  gameSong.play();

  // Gameplay loop.
  while (gameWindow.isOpen()) {
    float dt = gameClock.getElapsedTime().asSeconds();
    if (dt >= 1.f / 60.f) {
      gameClock.restart();

      while (const auto event = gameWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          gameWindow.close();
          gameSong.stop();
        }

        if (auto *key = event->getIf<sf::Event::KeyPressed>()) {
          handleGameplayControls(
              key->scancode,
              gameWindow,
              gameSong,
              gameState.isPaused,
              gameState.isMuted);

          const TypingHand typingHand = classifyTypingKey(key->scancode);

          if (typingHand == TypingHand::Left) {
            handleLeftTypingKey(
                key->scancode,
                gameState.leftLetterIsInTargetZone,
                gameState.leftFallingLetters,
                gameState.leftFeedback,
                missSound,
                correctSound);
          }

          if (typingHand == TypingHand::Right) {
            handleRightTypingKey(
                key->scancode,
                gameState.rightLetterIsInTargetZone,
                gameState.rightFallingLetters,
                gameState.rightFeedback,
                missSound,
                correctSound);
          }
        }
      }

      gameWindow.clear();

      if (!gameState.isPaused) {
        gameWindow.draw(gameState.targetBox);

        // Spawn left-hand letters at the configured interval.
        if (gameState.leftSpawnClock.getElapsedTime().asSeconds() > gameState.leftSpawnInterval) {
          spawnLeftLetter(
              gameState.leftFallingLetters,
              firaCodeFont,
              gameWindow.getSize().x,
              gameState.leftLetterSpeed);

          gameState.leftSpawnClock.restart();
        }

        // Spawn right-hand letters at the configured interval.
        if (gameState.rightSpawnClock.getElapsedTime().asSeconds() > gameState.rightSpawnInterval) {
          spawnRightLetter(
              gameState.rightFallingLetters,
              firaCodeFont,
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
            missSound);

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

        gameState.leftStreakText.setString("Streak: " + to_string(gameState.leftFeedback.streak));
        gameState.rightStreakText.setString("Streak: " + to_string(gameState.rightFeedback.streak));
        gameWindow.draw(gameState.gameInstructionsText);
        gameWindow.draw(gameState.leftStreakText);
        gameWindow.draw(gameState.rightStreakText);
        gameWindow.display();
      }
    }
  }

  return 0;
}
