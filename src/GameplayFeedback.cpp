#include "GameplayFeedback.h"

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
    int &streak,
    int &missStreak,
    Text &missText,
    bool &showMissMessage,
    Clock &messageClock,
    AudioPlayer &audio) {
  ++missStreak;
  streak = 0;

  setFeedbackTextSize(
      missText,
      missStreak);

  messageClock.restart();
  showMissMessage = true;
  audio.playMiss();
}

void recordHit(
    int &streak,
    int &missStreak,
    Text &correctText,
    bool &showCorrectMessage,
    Clock &messageClock,
    AudioPlayer &audio) {
  ++streak;
  missStreak = 0;

  setFeedbackTextSize(
      correctText,
      streak);

  showCorrectMessage = true;
  messageClock.restart();
  audio.playCorrectHit();
}

// Timed feedback rendering
void drawTimedFeedback(
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

// Off-screen miss handling
void handleOffscreenMisses(
    float windowHeight,
    vector<LeftHandLetter> &leftLetters,
    vector<RightHandLetter> &rightLetters,
    HandFeedbackState &leftFeedback,
    HandFeedbackState &rightFeedback,
    AudioPlayer &audio) {
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
        audio);
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
        audio);
  }
}