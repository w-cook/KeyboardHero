#include "AssetLoading.h"
#include "FallingLetters.h"
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

// Input and feedback types
enum class TypingHand {
  None,
  Left,
  Right
};

struct HandFeedbackState {
  int &streak;
  int &missStreak;
  Text &missText;
  Text &correctText;
  bool &showMissMessage;
  bool &showCorrectMessage;
  Clock &messageClock;
};

// Scoring and feedback
static void setFeedbackTextSize(
    Text &messageText,
    int streak) {
  if (streak <= 3) {
    messageText.setCharacterSize(24);
  } else if (streak <= 6) {
    messageText.setCharacterSize(30);
  } else {
    messageText.setCharacterSize(36);
  }
}

static void recordMiss(
    int &streak,
    int &missStreak,
    Text &missText,
    bool &showMissMessage,
    Clock &messageClock,
    Sound &missSound) {
  ++missStreak;
  streak = 0;

  setFeedbackTextSize(
      missText,
      missStreak);

  messageClock.restart();
  showMissMessage = true;
  missSound.play();
}

static void recordHit(
    int &streak,
    int &missStreak,
    Text &correctText,
    bool &showCorrectMessage,
    Clock &messageClock,
    Sound &correctSound) {
  ++streak;
  missStreak = 0;

  setFeedbackTextSize(
      correctText,
      streak);

  showCorrectMessage = true;
  messageClock.restart();
  correctSound.play();
}

static void drawTimedFeedback(
    RenderWindow &window,
    Text &message,
    bool &showMessage,
    const Clock &messageClock) {
  if (showMessage) {
    window.draw(message);

    if (messageClock.getElapsedTime().asSeconds() >= 0.25f) {
      showMessage = false;
    }
  }
}

// Gameplay input
static void handleGameplayControls(
    Keyboard::Scancode scancode,
    RenderWindow &gameWindow,
    Sound &gameSong,
    bool &isPaused,
    bool &isMuted) {
  // Escape -> quit
  if (scancode == Keyboard::Scancode::Escape) {
    gameWindow.close();
    gameSong.stop();
  }

  // Tab -> pause or resume
  if (scancode == Keyboard::Scancode::Tab) {
    isPaused = !isPaused;

    if (gameSong.getStatus() == SoundSource::Status::Playing) {
      gameSong.pause();
    } else {
      gameSong.play();
    }
  }

  // Ctrl -> mute or unmute gameplay music
  if (scancode == Keyboard::Scancode::LControl ||
      scancode == Keyboard::Scancode::RControl) {
    isMuted = !isMuted;

    if (isMuted) {
      gameSong.setVolume(0);
    } else {
      gameSong.setVolume(60);
    }
  }
}

static TypingHand classifyTypingKey(Keyboard::Scancode scancode) {
  switch (scancode) {
    case Keyboard::Scancode::Q:
    case Keyboard::Scancode::W:
    case Keyboard::Scancode::E:
    case Keyboard::Scancode::R:
    case Keyboard::Scancode::T:
    case Keyboard::Scancode::A:
    case Keyboard::Scancode::S:
    case Keyboard::Scancode::D:
    case Keyboard::Scancode::F:
    case Keyboard::Scancode::G:
    case Keyboard::Scancode::Z:
    case Keyboard::Scancode::X:
    case Keyboard::Scancode::C:
    case Keyboard::Scancode::V:
      return TypingHand::Left;

    case Keyboard::Scancode::Y:
    case Keyboard::Scancode::U:
    case Keyboard::Scancode::I:
    case Keyboard::Scancode::O:
    case Keyboard::Scancode::P:
    case Keyboard::Scancode::H:
    case Keyboard::Scancode::J:
    case Keyboard::Scancode::K:
    case Keyboard::Scancode::L:
    case Keyboard::Scancode::N:
    case Keyboard::Scancode::M:
    case Keyboard::Scancode::B:
      return TypingHand::Right;

    default:
      return TypingHand::None;
  }
}

static void handleLeftTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<LeftHandLetter> &letters,
    HandFeedbackState &feedback,
    Sound &missSound,
    Sound &correctSound) {
  // A keypress outside the target zone counts as a miss.
  if (!inTarget) {
    // Remove the first letter if one is available.
    if (!letters.empty()) {
      letters.erase(letters.begin());
    }

    recordMiss(
        feedback.streak,
        feedback.missStreak,
        feedback.missText,
        feedback.showMissMessage,
        feedback.messageClock,
        missSound);
  }

  // A matching keypress inside the target zone is a hit.
  if (inTarget &&
      !letters.empty() &&
      getDescription(scancode) ==
          letters.front().text.getString()) {
    letters.erase(letters.begin());

    recordHit(
        feedback.streak,
        feedback.missStreak,
        feedback.correctText,
        feedback.showCorrectMessage,
        feedback.messageClock,
        correctSound);
  }
}

static void handleRightTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<RightHandLetter> &letters,
    HandFeedbackState &feedback,
    Sound &missSound,
    Sound &correctSound) {
  // A keypress outside the target zone counts as a miss.
  if (!inTarget) {
    // Remove the first letter if one is available.
    if (!letters.empty()) {
      letters.erase(letters.begin());
    }

    recordMiss(
        feedback.streak,
        feedback.missStreak,
        feedback.missText,
        feedback.showMissMessage,
        feedback.messageClock,
        missSound);
  }

  // A matching keypress inside the target zone is a hit.
  if (inTarget &&
      !letters.empty() &&
      getDescription(scancode) ==
          letters.front().text.getString()) {
    letters.erase(letters.begin());

    recordHit(
        feedback.streak,
        feedback.missStreak,
        feedback.correctText,
        feedback.showCorrectMessage,
        feedback.messageClock,
        correctSound);
  }
}

// Off-screen miss handling
static void handleOffscreenMisses(
    float windowHeight,
    vector<LeftHandLetter> &leftLetters,
    vector<RightHandLetter> &rightLetters,
    HandFeedbackState &leftFeedback,
    HandFeedbackState &rightFeedback,
    Sound &missSound) {
  // Remove an off-screen left-hand letter and record a miss.
  if (!leftLetters.empty() &&
      leftLetters.front().text.getPosition().y >=
          windowHeight) {
    leftLetters.erase(leftLetters.begin());

    recordMiss(
        leftFeedback.streak,
        leftFeedback.missStreak,
        leftFeedback.missText,
        leftFeedback.showMissMessage,
        leftFeedback.messageClock,
        missSound);
  }

  // Remove an off-screen right-hand letter and record a miss.
  if (!rightLetters.empty() &&
      rightLetters.front().text.getPosition().y >=
          windowHeight) {
    rightLetters.erase(rightLetters.begin());

    recordMiss(
        rightFeedback.streak,
        rightFeedback.missStreak,
        rightFeedback.missText,
        rightFeedback.showMissMessage,
        rightFeedback.messageClock,
        missSound);
  }
}

// Application entry point
int main() {
  // Load the shared font.
  auto fontPath = assetPath("Fonts/FiraCodeNerdFontMono-Light.ttf");
  Font firaCodeFont;
  if (!firaCodeFont.openFromFile(fontPath)) {
    cerr << "Error loading firaCodeFont" << endl;
    return -1;
  }

  // Application-owned letter collections.
  vector<BackgroundLetter> menuBackgroundLetters;
  vector<LeftHandLetter> leftFallingLetters;
  vector<RightHandLetter> rightFallingLetters;

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

  // Create and configure the main-menu text.
  Text titleText(firaCodeFont);
  Text menuText(firaCodeFont);
  Text loadingText(firaCodeFont);

  configureMenuTexts(
      menuWindow.getSize(),
      titleText,
      menuText,
      loadingText);

  bool showLoadingMessage = true;

  // Create the gameplay instructions.
  Text gameInstructionsText(firaCodeFont);

  // Gameplay feedback positions.
  const Vector2u gameSize = gameWindow.getSize();

  const Vector2f leftFeedbackPosition{
      gameSize.x / 9.5f,
      gameSize.y / 2.f};

  const Vector2f rightFeedbackPosition{
      gameSize.x - (gameSize.x / 10.f),
      gameSize.y / 2.f};

  // Left-hand miss feedback.
  Text missLeftText(firaCodeFont);
  configureFeedbackText(
      missLeftText,
      "Miss",
      Color::Red,
      leftFeedbackPosition);
  bool showLeftMissMessage = false;

  // Right-hand miss feedback.
  Text missRightText(firaCodeFont);
  configureFeedbackText(
      missRightText,
      "Miss",
      Color::Red,
      rightFeedbackPosition);
  bool showRightMissMessage = false;

  // Left-hand correct-hit feedback.
  Text correctLeftText(firaCodeFont);
  configureFeedbackText(
      correctLeftText,
      "Correct!",
      Color::Cyan,
      leftFeedbackPosition);
  bool showLeftCorrectMessage = false;

  // Right-hand correct-hit feedback.
  Text correctRightText(firaCodeFont);
  configureFeedbackText(
      correctRightText,
      "Correct!",
      Color::Cyan,
      rightFeedbackPosition);
  bool showRightCorrectMessage = false;

  // Initialize streak counters.
  int leftStreak = 0;
  int rightStreak = 0;
  int leftMissStreak = 0;
  int rightMissStreak = 0;

  // Create gameplay HUD text.
  Text leftStreakText(firaCodeFont);
  Text rightStreakText(firaCodeFont);

  configureGameplayHudTexts(
      gameWindow.getSize(),
      gameInstructionsText,
      leftStreakText,
      rightStreakText,
      leftStreak,
      rightStreak);

  // Create the gameplay target bar.
  RectangleShape targetBox = createTargetBox(gameWindow.getSize());

  bool leftLetterIsInTargetZone = false;
  bool rightLetterIsInTargetZone = false;

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
  Clock backgroundSpawnClock;
  Clock leftSpawnClock;
  Clock rightSpawnClock;
  Clock leftMessageClock;
  Clock rightMessageClock;
  Clock loadingClock;
  float backgroundSpawnInterval = 0.2f;

  HandFeedbackState leftFeedback{
      leftStreak,
      leftMissStreak,
      missLeftText,
      correctLeftText,
      showLeftMissMessage,
      showLeftCorrectMessage,
      leftMessageClock};
  HandFeedbackState rightFeedback{
      rightStreak,
      rightMissStreak,
      missRightText,
      correctRightText,
      showRightMissMessage,
      showRightCorrectMessage,
      rightMessageClock};

  // Difficulty-dependent letter settings.
  float leftLetterSpeed;
  float rightLetterSpeed;
  float leftSpawnInterval;
  float rightSpawnInterval;

  MenuInputContext menuInput{
      menuWindow,
      gameWindow,
      titleSong,
      titleTransSound,
      leftSpawnInterval,
      leftLetterSpeed,
      rightSpawnInterval,
      rightLetterSpeed};

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

      menuWindow.draw(titleText);
      menuWindow.draw(menuText);

      // Update and draw the animated menu background.
      updateAndDrawMenuBackground(
          menuWindow,
          firaCodeFont,
          menuBackgroundLetters,
          backgroundSpawnClock,
          backgroundSpawnInterval,
          dt);

      if (showLoadingMessage) {
        menuWindow.draw(loadingText);
        if (loadingClock.getElapsedTime().asSeconds() > 3.f) {
          showLoadingMessage = false;
          loadingClock.stop();
        }
      }

      menuWindow.display();
    }
  }

  // Start gameplay music.
  gameSong.setLooping(true);
  gameSong.setVolume(60);
  gameSong.play();
  bool isPaused = false;
  bool isMuted = false;

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
              isPaused,
              isMuted);

          const TypingHand typingHand = classifyTypingKey(key->scancode);

          if (typingHand == TypingHand::Left) {
            handleLeftTypingKey(
                key->scancode,
                leftLetterIsInTargetZone,
                leftFallingLetters,
                leftFeedback,
                missSound,
                correctSound);
          }

          if (typingHand == TypingHand::Right) {
            handleRightTypingKey(
                key->scancode,
                rightLetterIsInTargetZone,
                rightFallingLetters,
                rightFeedback,
                missSound,
                correctSound);
          }
        }
      }

      gameWindow.clear();

      if (!isPaused) {
        gameWindow.draw(targetBox);

        // Spawn left-hand letters at the configured interval.
        if (leftSpawnClock.getElapsedTime().asSeconds() > leftSpawnInterval) {
          spawnLeftLetter(
              leftFallingLetters,
              firaCodeFont,
              gameWindow.getSize().x,
              leftLetterSpeed);

          leftSpawnClock.restart();
        }

        // Spawn right-hand letters at the configured interval.
        if (rightSpawnClock.getElapsedTime().asSeconds() > rightSpawnInterval) {
          spawnRightLetter(
              rightFallingLetters,
              firaCodeFont,
              gameWindow.getSize().x,
              rightLetterSpeed);

          rightSpawnClock.restart();
        }

        // Update and draw falling gameplay letters.
        updateAndDrawGameplayLetters(
            gameWindow,
            leftFallingLetters,
            rightFallingLetters,
            dt);

        // Remove off-screen letters and record misses.
        handleOffscreenMisses(
            static_cast<float>(gameWindow.getSize().y),
            leftFallingLetters,
            rightFallingLetters,
            leftFeedback,
            rightFeedback,
            missSound);

        // Check whether the first left-hand letter is in the target zone.
        leftLetterIsInTargetZone =
            !leftFallingLetters.empty() &&
            isLetterInTargetZone(
                leftFallingLetters.front().text,
                targetBox);

        // Check whether the first right-hand letter is in the target zone.
        rightLetterIsInTargetZone =
            !rightFallingLetters.empty() &&
            isLetterInTargetZone(
                rightFallingLetters.front().text,
                targetBox);

        // Draw active hit/miss feedback for both hands.
        drawTimedFeedback(
            gameWindow,
            missLeftText,
            showLeftMissMessage,
            leftMessageClock);

        drawTimedFeedback(
            gameWindow,
            missRightText,
            showRightMissMessage,
            rightMessageClock);

        drawTimedFeedback(
            gameWindow,
            correctLeftText,
            showLeftCorrectMessage,
            leftMessageClock);

        drawTimedFeedback(
            gameWindow,
            correctRightText,
            showRightCorrectMessage,
            rightMessageClock);

        leftStreakText.setString("Streak: " + to_string(leftStreak));
        rightStreakText.setString("Streak: " + to_string(rightStreak));
        gameWindow.draw(gameInstructionsText);
        gameWindow.draw(leftStreakText);
        gameWindow.draw(rightStreakText);
        gameWindow.display();
      }
    }
  }

  return 0;
}
