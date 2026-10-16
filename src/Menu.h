#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>

class AudioPlayer;
struct GameplayState;
struct MenuState;

// Menu types
struct BackgroundLetter {
  sf::Text text;
  float speed;
};

struct MenuInputContext {
  sf::RenderWindow &menuWindow;
  sf::RenderWindow &gameWindow;
  AudioPlayer &audio;
  GameplayState &gameState;
};

// Menu configuration
void configureMenuTexts(
    sf::Vector2u windowSize,
    MenuState &menuState);

// Menu animation
void updateAndDrawMenuBackground(
    sf::RenderWindow &window,
    const sf::Font &font,
    MenuState &menuState,
    float dt);

// Menu input
void handleMenuKeyPress(
    sf::Keyboard::Scancode scancode,
    MenuInputContext &context);

// Run the main-menu loop
void runMenu(
    MenuState &menuState,
    const sf::Font &font,
    sf::Clock &frameClock,
    MenuInputContext &context);