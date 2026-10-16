#include "GameplayInput.h"

#include "AudioPlayer.h"

using namespace sf;
using namespace std;

// Gameplay input
void handleGameplayControls(
    Keyboard::Scancode scancode,
    RenderWindow &gameWindow,
    AudioPlayer &audio,
    bool &isPaused) {
  // Escape -> quit
  if (scancode == Keyboard::Scancode::Escape) {
    gameWindow.close();
    audio.stopGameplayMusic();
  }

  // Tab -> pause or resume
  if (scancode == Keyboard::Scancode::Tab) {
    isPaused = !isPaused;
    audio.toggleGameplayMusicPause();
  }

  // Ctrl -> mute or unmute gameplay music
  if (scancode == Keyboard::Scancode::LControl ||
      scancode == Keyboard::Scancode::RControl) {
    audio.toggleGameplayMusicMute();
  }
}

TypingHand classifyTypingKey(Keyboard::Scancode scancode) {
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

void handleLeftTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<LeftHandLetter> &letters,
    HandFeedbackState &feedback,
    AudioPlayer &audio) {
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
        audio);
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
        audio);
  }
}

void handleRightTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<RightHandLetter> &letters,
    HandFeedbackState &feedback,
    AudioPlayer &audio) {
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
        audio);
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
        audio);
  }
}