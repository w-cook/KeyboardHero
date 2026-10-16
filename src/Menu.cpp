#include "Menu.h"
#include "RandomGeneration.h"

using namespace sf;
using namespace std;

void configureMenuTexts(
    Vector2u windowSize,
    Text &titleText,
    Text &menuText,
    Text &loadingText) {
  // Configure the title.
  titleText.setString("Keyboard Hero");
  titleText.setCharacterSize(72);
  titleText.setStyle(Text::Bold);
  titleText.setFillColor(Color::White);

  FloatRect titleTextSize = titleText.getLocalBounds();
  titleText.setOrigin({
      titleTextSize.size.x / 2.f,
      titleTextSize.size.y / 2.f});
  titleText.setPosition({
      windowSize.x / 2.f,
      windowSize.y / 4.f});

  // Configure the difficulty-selection menu.
  menuText.setString("1 -> Easy\n\n2 -> Medium\n\n3 -> "
                     "Hard\n\nQ -> Quit");
  menuText.setCharacterSize(36);
  menuText.setFillColor(Color::Green);

  FloatRect menuTextSize = menuText.getLocalBounds();
  menuText.setOrigin({
      menuTextSize.size.x / 2.f,
      menuTextSize.size.y / 2.f});
  menuText.setPosition({
      windowSize.x / 2.f,
      windowSize.y / 1.5f});

  // Configure the introductory loading message.
  loadingText.setString(
      "Loading...please do not press any keys\nKeyboardHero "
      "may result in KeyboardFailure");
  loadingText.setCharacterSize(36);
  loadingText.setFillColor(Color::Yellow);

  FloatRect loadingTextSize = loadingText.getLocalBounds();
  loadingText.setOrigin({
      loadingTextSize.size.x / 2.f,
      loadingTextSize.size.y / 2.f});
  loadingText.setPosition({
      windowSize.x / 2.f,
      windowSize.y - 76.f});
}

void updateAndDrawMenuBackground(
    RenderWindow &window,
    const Font &font,
    vector<BackgroundLetter> &letters,
    Clock &spawnClock,
    float spawnInterval,
    float dt) {
  // Spawn a new background letter when the interval elapses.
  if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
    Text text(font);
    char character = generateBackgroundLetter();

    text.setString(character);
    text.setCharacterSize(36);
    text.setFillColor(Color::Green);

    // Color the first and every subsequent 13th background letter red.
    if (letters.size() % 13 == 0) {
      text.setFillColor(Color::Red);
    }

    FloatRect textBounds = text.getLocalBounds();
    text.setOrigin({
        textBounds.size.x / 2.f,
        textBounds.size.y / 2.f});

    float xPosition = generateBackgroundXPosition(window.getSize().x);
    text.setPosition({
        xPosition,
        -20.f});

    float speed = generateBackgroundLetterSpeed();
    letters.push_back({
        text,
        speed});

    spawnClock.restart();
  }

  // Update and draw existing background letters.
  for (auto &letter : letters) {
    letter.text.move({
        0,
        letter.speed * dt});
    window.draw(letter.text);
  }
}

static void configureDifficulty(
    Keyboard::Scancode selection,
    float &leftSpawnInterval,
    float &leftLetterSpeed,
    float &rightSpawnInterval,
    float &rightLetterSpeed) {
  switch (selection) {
    case Keyboard::Scancode::Num1:
      // Easy
      leftSpawnInterval = generateEasySpawnInterval();
      leftLetterSpeed = 200.f;
      rightSpawnInterval = generateEasySpawnInterval();
      rightLetterSpeed = 200.f;
      break;

    case Keyboard::Scancode::Num2:
      // Medium
      leftSpawnInterval = generateMediumSpawnInterval();
      leftLetterSpeed = 300.f;
      rightSpawnInterval = generateMediumSpawnInterval();
      rightLetterSpeed = 300.f;
      break;

    case Keyboard::Scancode::Num3:
      // Hard
      leftSpawnInterval = generateHardSpawnInterval();
      leftLetterSpeed = 400.f;
      rightSpawnInterval = generateHardSpawnInterval();
      rightLetterSpeed = 400.f;
      break;

    default:
      break;
  }
}

void handleMenuKeyPress(
    Keyboard::Scancode scancode,
    MenuInputContext &context) {
  // Q -> quit
  if (scancode == Keyboard::Scancode::Q) {
    context.menuWindow.close();
    context.gameWindow.close();
    context.titleSong.stop();
  }

  // Select difficulty and transition into gameplay.
  if (scancode == Keyboard::Scancode::Num1 ||
      scancode == Keyboard::Scancode::Num2 ||
      scancode == Keyboard::Scancode::Num3) {

    context.menuWindow.close();
    context.titleSong.stop();
    context.transitionSound.play();
    context.gameWindow.display();

    configureDifficulty(
        scancode,
        context.leftSpawnInterval,
        context.leftLetterSpeed,
        context.rightSpawnInterval,
        context.rightLetterSpeed);
  }
}