#pragma once

#include <SFML/Graphics.hpp>

void configureFeedbackText(
    sf::Text &text,
    const char *message,
    const sf::Color &color,
    sf::Vector2f position);

void configureGameplayHudTexts(
    sf::Vector2u windowSize,
    sf::Text &instructionsText,
    sf::Text &leftStreakText,
    sf::Text &rightStreakText,
    int leftStreak,
    int rightStreak);

sf::RectangleShape createTargetBox(sf::Vector2u windowSize);

bool isLetterInTargetZone(
    const sf::Text &letter,
    const sf::RectangleShape &targetBox);