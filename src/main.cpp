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

  // Create the menu and gameplay state.
  MenuState menuState(firaCodeFont);
  GameplayState gameState(firaCodeFont);

  // Create the shared application window.
  RenderWindow window(
      VideoMode({
          1920,
          1080}),
      "KeyboardHero");
  window.setVerticalSyncEnabled(true);

  // Initialize menu and gameplay presentation.
  configureMenuTexts(
      window.getSize(),
      menuState);

  initializeGameplayUI(
      window.getSize(),
      gameState);

  // Load audio resources.
  AudioPlayer audio;

  // Shared frame clock.
  Clock frameClock;

  // Begin at the main menu.
  GameScreen currentScreen = GameScreen::MainMenu;

  // Application state machine.
  while (window.isOpen() && currentScreen != GameScreen::Exit) {
    switch (currentScreen) {
      case GameScreen::MainMenu: {
        window.setTitle("KeyboardHero - Main Menu");

        audio.startTitleMusic();
        frameClock.restart();

        currentScreen = runMenu(
            menuState,
            firaCodeFont,
            frameClock,
            window,
            gameState);

        audio.stopTitleMusic();

        if (currentScreen == GameScreen::Gameplay) {
          audio.playMenuTransition();
        }

        break;
      }

      case GameScreen::Gameplay: {
        window.setTitle("KeyboardHero");

        // Prepare a fresh game using the selected difficulty.
        resetGameplaySession(gameState);
        frameClock.restart();

        audio.startGameplayMusic();

        currentScreen = runGameplay(
            window,
            gameState,
            firaCodeFont,
            frameClock,
            audio);

        audio.stopGameplayMusic();

        break;
      }

      case GameScreen::Exit:
        break;
    }
  }

  // All application shutdown paths converge here.
  window.close();

  return 0;
}