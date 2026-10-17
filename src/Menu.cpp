#include "Menu.h"

#include "ApplicationState.h"
#include "RandomGeneration.h"

#include <algorithm>
#include <optional>

using namespace sf;
using namespace std;

void configureMenuTexts(
    Vector2u windowSize,
    MenuState &menuState) {
  Text &titleText = menuState.titleText;
  Text &menuText = menuState.menuText;

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
}

void updateAndDrawMenuBackground(
    RenderWindow &window,
    const Font &font,
    MenuState &menuState,
    float deltaTime) {
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
    if (menuState.backgroundLettersSpawned % 13 == 0) {
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
    ++menuState.backgroundLettersSpawned;

    spawnClock.restart();
  }

  // Update and draw existing background letters.
  for (auto &letter : letters) {
    letter.text.move({
        0,
        letter.speed * deltaTime});
    window.draw(letter.text);
  }

  // Remove letters that have completely moved below the window.
  const float windowHeight =
      static_cast<float>(window.getSize().y);

  letters.erase(
      std::remove_if(
          letters.begin(),
          letters.end(),
          [windowHeight](const BackgroundLetter &letter) {
            return letter.text.getGlobalBounds().position.y >
                   windowHeight;
          }),
      letters.end());
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


static std::optional<GameScreen> handleMenuKeyPress(
    Keyboard::Scancode scancode,
    GameplayState &gameState) {
    // Q requests application shutdown.
    if (scancode == Keyboard::Scancode::Q) {
        return GameScreen::Exit;
    }

    // Difficulty selection requests gameplay.
    if (scancode == Keyboard::Scancode::Num1 ||
        scancode == Keyboard::Scancode::Num2 ||
        scancode == Keyboard::Scancode::Num3) {

        configureDifficulty(scancode, gameState);
        return GameScreen::Gameplay;
    }

    // Other keys do not request a transition.
    return std::nullopt;
}

GameScreen runMenu(
    MenuState &menuState,
    const Font &font,
    Clock &frameClock,
    RenderWindow &window,
    GameplayState &gameState) {
    while (window.isOpen()) {
        float deltaTime = frameClock.getElapsedTime().asSeconds();

        if (deltaTime >= 1.f / 60.f) {
            frameClock.restart();

            while (const auto event = window.pollEvent()) {
                if (event->is<Event::Closed>()) {
                    return GameScreen::Exit;
                }

                if (auto *key = event->getIf<Event::KeyPressed>()) {
                    auto nextScreen = handleMenuKeyPress(
                        key->scancode,
                        gameState);

                    if (nextScreen.has_value()) {
                        return *nextScreen;
                    }
                }
            }

            window.clear();

            window.draw(menuState.titleText);
            window.draw(menuState.menuText);

            updateAndDrawMenuBackground(
                window,
                font,
                menuState,
                deltaTime);

            window.display();
        }
    }

    return GameScreen::Exit;
}