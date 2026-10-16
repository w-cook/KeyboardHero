#include "ApplicationState.h"
#include "AssetLoading.h"
#include "AudioPlayer.h"
#include "Gameplay.h"
#include "GameplayUI.h"
#include "Menu.h"

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
  AudioPlayer audio;

  // Initialize clocks.
  Clock gameClock;
  gameState.leftSpawnClock.restart();
  gameState.rightSpawnClock.restart();

  MenuInputContext menuInput{
      menuWindow,
      gameWindow,
      audio,
      gameState.leftSpawnInterval,
      gameState.leftLetterSpeed,
      gameState.rightSpawnInterval,
      gameState.rightLetterSpeed};

  // Start the title music.
  audio.startTitleMusic();

  // Run the main menu.
  runMenu(
      menuState,
      firaCodeFont,
      gameClock,
      menuInput);

  // Start gameplay music.
  audio.startGameplayMusic();

  // Run gameplay.
  runGameplay(
      gameWindow,
      gameState,
      firaCodeFont,
      gameClock,
      audio);

  return 0;
}
