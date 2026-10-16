#include "FallingLetters.h"
#include "RandomGeneration.h"

using namespace sf;
using namespace std;

// Letter spawning
void spawnLeftLetter(
    vector<LeftHandLetter> &letters,
    const Font &font,
    int windowWidth,
    float speed) {
  Text letterText(font);
  char letter = generateLeftHandLetter();

  letterText.setString(letter);
  letterText.setCharacterSize(36);
  letterText.setFillColor(Color::Green);

  FloatRect textSize = letterText.getLocalBounds();
  letterText.setOrigin({
      textSize.size.x / 2.f,
      textSize.size.y / 2.f});

  float x = generateLeftHandXPosition(windowWidth);
  letterText.setPosition({
      x,
      -20.f});

  letters.push_back({
      letterText,
      speed});
}

void spawnRightLetter(
    vector<RightHandLetter> &letters,
    const Font &font,
    int windowWidth,
    float speed) {
  Text letterText(font);
  char letter = generateRightHandLetter();

  letterText.setString(letter);
  letterText.setCharacterSize(36);
  letterText.setFillColor(Color::Green);

  FloatRect textSize = letterText.getLocalBounds();
  letterText.setOrigin({
      textSize.size.x / 2.f,
      textSize.size.y / 2.f});

  float x = generateRightHandXPosition(windowWidth);
  letterText.setPosition({
      x,
      -20.f});

  letters.push_back({
      letterText,
      speed});
}

// Letter movement and rendering
void updateAndDrawGameplayLetters(
    RenderWindow &window,
    vector<LeftHandLetter> &leftLetters,
    vector<RightHandLetter> &rightLetters,
    float dt) {
  // Move and draw left-hand letters.
  for (auto &letter : leftLetters) {
    letter.text.move({
        0,
        letter.speed * dt});
    window.draw(letter.text);
  }

  // Move and draw right-hand letters.
  for (auto &letter : rightLetters) {
    letter.text.move({
        0,
        letter.speed * dt});
    window.draw(letter.text);
  }
}