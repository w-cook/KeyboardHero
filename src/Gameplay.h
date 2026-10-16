#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

class AudioPlayer;
struct GameplayState;

// Run the gameplay loop
void runGameplay(
    sf::RenderWindow &gameWindow,
    GameplayState &gameState,
    const sf::Font &font,
    sf::Clock &frameClock,
    AudioPlayer &audio);