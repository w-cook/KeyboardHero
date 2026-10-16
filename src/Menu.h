#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>

// Menu types
struct BackgroundLetter {
  sf::Text text;
  float speed;
};

struct MenuInputContext {
  sf::RenderWindow &menuWindow;
  sf::RenderWindow &gameWindow;
  sf::Sound &titleSong;
  sf::Sound &transitionSound;

  float &leftSpawnInterval;
  float &leftLetterSpeed;
  float &rightSpawnInterval;
  float &rightLetterSpeed;
};

// Menu configuration
void configureMenuTexts(
    sf::Vector2u windowSize,
    sf::Text &titleText,
    sf::Text &menuText,
    sf::Text &loadingText);

// Menu animation
void updateAndDrawMenuBackground(
    sf::RenderWindow &window,
    const sf::Font &font,
    std::vector<BackgroundLetter> &letters,
    sf::Clock &spawnClock,
    float spawnInterval,
    float dt);

// Menu input
void handleMenuKeyPress(
    sf::Keyboard::Scancode scancode,
    MenuInputContext &context);