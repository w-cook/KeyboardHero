#include "AudioPlayer.h"

#include "AssetLoading.h"

// Load sound resources before constructing the players.
AudioPlayer::Buffers::Buffers() {
  loadSoundBuffers(
      titleSong,
      transitionSound,
      gameSong,
      correctSound,
      missSound);
}

AudioPlayer::AudioPlayer()
    : titleSong_(buffers_.titleSong),
      transitionSound_(buffers_.transitionSound),
      gameSong_(buffers_.gameSong),
      correctSound_(buffers_.correctSound),
      missSound_(buffers_.missSound) {}

// Menu audio
void AudioPlayer::startTitleMusic() {
  titleSong_.setLooping(true);
  titleSong_.setVolume(50);
  titleSong_.play();
}

void AudioPlayer::stopTitleMusic() {
  titleSong_.stop();
}

void AudioPlayer::playMenuTransition() {
  transitionSound_.play();
}

// Gameplay music
void AudioPlayer::startGameplayMusic() {
  gameSong_.setLooping(true);
  gameSong_.setVolume(60);
  gameSong_.play();
}

void AudioPlayer::stopGameplayMusic() {
  gameSong_.stop();
}

void AudioPlayer::toggleGameplayMusicPause() {
  if (gameSong_.getStatus() == sf::SoundSource::Status::Playing) {
    gameSong_.pause();
  } else {
    gameSong_.play();
  }
}

void AudioPlayer::toggleGameplayMusicMute() {
  gameplayMusicMuted_ = !gameplayMusicMuted_;
  gameSong_.setVolume(gameplayMusicMuted_ ? 0.f : 60.f);
}

// Gameplay sound effects
void AudioPlayer::playCorrectHit() {
  correctSound_.play();
}

void AudioPlayer::playMiss() {
  missSound_.play();
}