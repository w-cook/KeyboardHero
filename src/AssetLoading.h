#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>
#include <string>

std::filesystem::path assetPath(const std::string &insideAssets);

void loadSoundBuffers(
    sf::SoundBuffer &titleSongBuffer,
    sf::SoundBuffer &titleTransSoundBuffer,
    sf::SoundBuffer &gameSongBuffer,
    sf::SoundBuffer &correctSoundBuffer,
    sf::SoundBuffer &missSoundBuffer);