#pragma once

#include <SFML/Audio.hpp>

// Owns application audio resources and controls playback.
class AudioPlayer {
public:
  AudioPlayer();

  AudioPlayer(const AudioPlayer &) = delete;
  AudioPlayer &operator=(const AudioPlayer &) = delete;

  // Menu audio
  void startTitleMusic();
  void stopTitleMusic();
  void playMenuTransition();

  // Gameplay music
  void startGameplayMusic();
  void stopGameplayMusic();
  void toggleGameplayMusicPause();
  void toggleGameplayMusicMute();

  // Gameplay sound effects
  void playCorrectHit();
  void playMiss();

private:
  // Buffers must be loaded before constructing their sound players.
  struct Buffers {
    Buffers();

    sf::SoundBuffer titleSong;
    sf::SoundBuffer transitionSound;
    sf::SoundBuffer gameSong;
    sf::SoundBuffer correctSound;
    sf::SoundBuffer missSound;
  };

  Buffers buffers_;

  sf::Sound titleSong_;
  sf::Sound transitionSound_;
  sf::Sound gameSong_;
  sf::Sound correctSound_;
  sf::Sound missSound_;

  bool gameplayMusicMuted_ = false;
};