#include "GameplayFeedback.h"

#include "ApplicationState.h"
#include "AudioPlayer.h"

using namespace sf;
using namespace std;

// Internal feedback utility
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

// Scoring and feedback
void recordMiss(
    HandFeedbackState &feedback,
    AudioPlayer &audio) {
  ++feedback.missStreak;
  feedback.streak = 0;

  setFeedbackTextSize(
      feedback.missText,
      feedback.missStreak);

  feedback.messageClock.restart();
  feedback.showMissMessage = true;
  audio.playMiss();
}

void recordHit(
    HandFeedbackState &feedback,
    AudioPlayer &audio) {
  ++feedback.streak;
  feedback.missStreak = 0;

  setFeedbackTextSize(
      feedback.correctText,
      feedback.streak);

  feedback.showCorrectMessage = true;
  feedback.messageClock.restart();
  audio.playCorrectHit();
}

// Timed feedback rendering
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

void drawGameplayFeedback(
    RenderWindow &window,
    GameplayState &gameState) {
  // Draw left-hand miss feedback.
  drawTimedFeedback(
      window,
      gameState.leftFeedback.missText,
      gameState.leftFeedback.showMissMessage,
      gameState.leftFeedback.messageClock);

  // Draw right-hand miss feedback.
  drawTimedFeedback(
      window,
      gameState.rightFeedback.missText,
      gameState.rightFeedback.showMissMessage,
      gameState.rightFeedback.messageClock);

  // Draw left-hand correct-hit feedback.
  drawTimedFeedback(
      window,
      gameState.leftFeedback.correctText,
      gameState.leftFeedback.showCorrectMessage,
      gameState.leftFeedback.messageClock);

  // Draw right-hand correct-hit feedback.
  drawTimedFeedback(
      window,
      gameState.rightFeedback.correctText,
      gameState.rightFeedback.showCorrectMessage,
      gameState.rightFeedback.messageClock);
}

// Off-screen miss handling
void handleOffscreenMisses(
    float windowHeight,
    GameplayState &gameState,
    AudioPlayer &audio) {
  auto &leftLetters = gameState.leftFallingLetters;
  auto &rightLetters = gameState.rightFallingLetters;

  // Remove an off-screen left-hand letter and record a miss.
  if (!leftLetters.empty() &&
      leftLetters.front().text.getPosition().y >= windowHeight) {
    leftLetters.erase(leftLetters.begin());

    recordMiss(
        gameState.leftFeedback,
        audio);
  }

  // Remove an off-screen right-hand letter and record a miss.
  if (!rightLetters.empty() &&
      rightLetters.front().text.getPosition().y >= windowHeight) {
    rightLetters.erase(rightLetters.begin());

    recordMiss(
        gameState.rightFeedback,
        audio);
  }
}