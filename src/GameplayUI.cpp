#include "GameplayUI.h"

#include <string>

using namespace sf;
using namespace std;

void configureFeedbackText(
    Text &text,
    const char *message,
    const Color &color,
    Vector2f position) {
  text.setString(message);
  text.setCharacterSize(24);
  text.setFillColor(color);

  FloatRect textSize = text.getLocalBounds();
  text.setOrigin({
      textSize.size.x / 2.f,
      textSize.size.y / 2.f});

  text.setPosition(position);
}

void configureGameplayHudTexts(
    Vector2u windowSize,
    Text &instructionsText,
    Text &leftStreakText,
    Text &rightStreakText,
    int leftStreak,
    int rightStreak) {
  // Configure gameplay instructions.
  instructionsText.setString(
      "TAB -> Pause\n\nESC -> Quit\n\nCTL -> Mute Song");
  instructionsText.setCharacterSize(36);
  instructionsText.setFillColor(Color::White);

  FloatRect instructionsSize = instructionsText.getLocalBounds();
  instructionsText.setOrigin({
      instructionsSize.size.x / 2.f,
      instructionsSize.size.y / 2.f});
  instructionsText.setPosition({
      windowSize.x / 2.f,
      windowSize.y / 2.f});

  // Configure left-hand streak counter.
  leftStreakText.setString("Streak: " + to_string(leftStreak));
  leftStreakText.setCharacterSize(36);
  leftStreakText.setFillColor(Color::White);

  FloatRect leftStreakSize = leftStreakText.getLocalBounds();
  leftStreakText.setOrigin({
      leftStreakSize.size.x / 2.f,
      leftStreakSize.size.y / 2.f});
  leftStreakText.setPosition({
      windowSize.x / 4.f,
      windowSize.y - (windowSize.y / 20.f)});

  // Configure right-hand streak counter.
  rightStreakText.setString("Streak: " + to_string(rightStreak));
  rightStreakText.setCharacterSize(36);
  rightStreakText.setFillColor(Color::White);

  FloatRect rightStreakSize = rightStreakText.getLocalBounds();
  rightStreakText.setOrigin({
      rightStreakSize.size.x / 2.f,
      rightStreakSize.size.y / 2.f});
  rightStreakText.setPosition({
      windowSize.x - (windowSize.x / 4.f),
      windowSize.y - (windowSize.y / 20.f)});
}

RectangleShape createTargetBox(Vector2u windowSize) {
  RectangleShape targetBox(
      {static_cast<float>(windowSize.x),
       72.f});

  targetBox.setOrigin({
      targetBox.getSize().x / 2.f,
      targetBox.getSize().y / 2.f});

  targetBox.setPosition({
      windowSize.x / 2.f,
      windowSize.y - (windowSize.y / 8.5f)});

  targetBox.setOutlineColor(Color::Green);
  targetBox.setOutlineThickness(1.f);
  targetBox.setFillColor(Color::Transparent);

  return targetBox;
}

bool isLetterInTargetZone(
    const Text &letter,
    const RectangleShape &targetBox) {
  const float letterY = letter.getPosition().y;
  const float targetY = targetBox.getPosition().y;

  return letterY >= (targetY - 54.f) &&
         letterY <= (targetY + 36.f);
}