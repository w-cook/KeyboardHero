#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <vector>

class AudioPlayer;
struct GameplayState;
struct MenuState;
enum class GameScreen;

// Menu types
struct BackgroundLetter {
  sf::Text text;
  float speed;
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
    float deltaTime);

// Run the main-menu loop and return the requested screen
GameScreen runMenu(
    MenuState &menuState,
    const sf::Font &font,
    sf::Clock &frameClock,
    sf::RenderWindow &window,
    GameplayState &gameState);