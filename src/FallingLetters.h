#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Gameplay letter types
struct LeftHandLetter {
  sf::Text text;
  float speed;
};

struct RightHandLetter {
  sf::Text text;
  float speed;
};

// Letter spawning
void spawnLeftLetter(
    std::vector<LeftHandLetter> &letters,
    const sf::Font &font,
    int windowWidth,
    float speed);

void spawnRightLetter(
    std::vector<RightHandLetter> &letters,
    const sf::Font &font,
    int windowWidth,
    float speed);

// Letter movement and rendering
void updateAndDrawGameplayLetters(
    sf::RenderWindow &window,
    std::vector<LeftHandLetter> &leftLetters,
    std::vector<RightHandLetter> &rightLetters,
    float deltaTime);