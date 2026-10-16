#include "Menu.h"

#include "ApplicationState.h"
#include "AudioPlayer.h"
#include "RandomGeneration.h"

using namespace sf;
using namespace std;

void configureMenuTexts(
    Vector2u windowSize,
    MenuState &menuState) {
  Text &titleText = menuState.titleText;
  Text &menuText = menuState.menuText;
  Text &loadingText = menuState.loadingText;

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
    MenuState &menuState,
    float dt) {
  auto &letters = menuState.backgroundLetters;
  auto &spawnClock = menuState.backgroundSpawnClock;
  const float spawnInterval = menuState.backgroundSpawnInterval;

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
    GameplayState &gameState) {
  switch (selection) {
    case Keyboard::Scancode::Num1:
      // Easy
      gameState.leftSpawnInterval = generateEasySpawnInterval();
      gameState.leftLetterSpeed = 200.f;
      gameState.rightSpawnInterval = generateEasySpawnInterval();
      gameState.rightLetterSpeed = 200.f;
      break;

    case Keyboard::Scancode::Num2:
      // Medium
      gameState.leftSpawnInterval = generateMediumSpawnInterval();
      gameState.leftLetterSpeed = 300.f;
      gameState.rightSpawnInterval = generateMediumSpawnInterval();
      gameState.rightLetterSpeed = 300.f;
      break;

    case Keyboard::Scancode::Num3:
      // Hard
      gameState.leftSpawnInterval = generateHardSpawnInterval();
      gameState.leftLetterSpeed = 400.f;
      gameState.rightSpawnInterval = generateHardSpawnInterval();
      gameState.rightLetterSpeed = 400.f;
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
    context.audio.stopTitleMusic();
  }

  // Select difficulty and transition into gameplay.
  if (scancode == Keyboard::Scancode::Num1 ||
      scancode == Keyboard::Scancode::Num2 ||
      scancode == Keyboard::Scancode::Num3) {

    context.menuWindow.close();
    context.audio.stopTitleMusic();
    context.audio.playMenuTransition();
    context.gameWindow.display();

    configureDifficulty(
        scancode,
        context.gameState);
  }
}

void runMenu(
    MenuState &menuState,
    const Font &font,
    Clock &frameClock,
    MenuInputContext &context) {
  RenderWindow &menuWindow = context.menuWindow;

  while (menuWindow.isOpen()) {
    float dt = frameClock.getElapsedTime().asSeconds();

    if (dt >= 1.f / 60.f) {
      frameClock.restart();

      while (const auto event = menuWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          menuWindow.close();
          context.audio.stopTitleMusic();
        }

        if (auto *key = event->getIf<Event::KeyPressed>()) {
          handleMenuKeyPress(
              key->scancode,
              context);
        }
      }

      menuWindow.clear();

      menuWindow.draw(menuState.titleText);
      menuWindow.draw(menuState.menuText);

      // Update and draw the animated menu background.
      updateAndDrawMenuBackground(
          menuWindow,
          font,
          menuState,
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
}