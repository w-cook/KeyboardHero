#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

struct GameplayState;

// Run the gameplay loop
void runGameplay(
    sf::RenderWindow &gameWindow,
    GameplayState &gameState,
    const sf::Font &font,
    sf::Clock &frameClock,
    sf::Sound &gameSong,
    sf::Sound &missSound,
    sf::Sound &correctSound);