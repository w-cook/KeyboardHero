#include "ApplicationState.h"
#include "AssetLoading.h"
#include "Gameplay.h"
#include "GameplayUI.h"
#include "Menu.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <iostream>

using namespace sf;
using namespace std;

// Application entry point
int main() {
  // Load the shared font.
  auto fontPath = assetPath("Fonts/FiraCodeNerdFontMono-Light.ttf");
  Font firaCodeFont;
  if (!firaCodeFont.openFromFile(fontPath)) {
    cerr << "Error loading firaCodeFont" << endl;
    return -1;
  }

  MenuState menuState(firaCodeFont);
  GameplayState gameState(firaCodeFont);

  // Create the gameplay window.
  RenderWindow gameWindow(
      VideoMode({
          1920,
          1080}),
      "KeyboardHero");
  gameWindow.setVerticalSyncEnabled(true);

  // Create the main-menu window.
  RenderWindow menuWindow(
      VideoMode({
          1920,
          1080}),
      "Main Menu");
  menuWindow.setVerticalSyncEnabled(true);

  configureMenuTexts(
      menuWindow.getSize(),
      menuState.titleText,
      menuState.menuText,
      menuState.loadingText);

  // Initialize gameplay presentation.
  initializeGameplayUI(
      gameWindow.getSize(),
      gameState);

  // Load audio resources.
  SoundBuffer titleSongBuffer;
  SoundBuffer titleTransSoundBuffer;
  SoundBuffer gameSongBuffer;
  SoundBuffer correctSoundBuffer;
  SoundBuffer missSoundBuffer;

  loadSoundBuffers(
      titleSongBuffer,
      titleTransSoundBuffer,
      gameSongBuffer,
      correctSoundBuffer,
      missSoundBuffer);

  // Create sound players after their buffers.
  Sound titleSong(titleSongBuffer);
  Sound titleTransSound(titleTransSoundBuffer);
  Sound gameSong(gameSongBuffer);
  Sound correctSound(correctSoundBuffer);
  Sound missSound(missSoundBuffer);

  // Initialize clocks.
  Clock gameClock;
  gameState.leftSpawnClock.restart();
  gameState.rightSpawnClock.restart();

  MenuInputContext menuInput{
      menuWindow,
      gameWindow,
      titleSong,
      titleTransSound,
      gameState.leftSpawnInterval,
      gameState.leftLetterSpeed,
      gameState.rightSpawnInterval,
      gameState.rightLetterSpeed};

  // Start the title music.
  titleSong.setLooping(true);
  titleSong.setVolume(50);
  titleSong.play();

  // Run the main menu.
  runMenu(
      menuState,
      firaCodeFont,
      gameClock,
      menuInput);

  // Start gameplay music.
  gameSong.setLooping(true);
  gameSong.setVolume(60);
  gameSong.play();

  // Run gameplay.
  runGameplay(
      gameWindow,
      gameState,
      firaCodeFont,
      gameClock,
      gameSong,
      missSound,
      correctSound);

  return 0;
}
