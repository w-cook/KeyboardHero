#include "RandomGeneration.h"

#include <cstdlib>
#include <vector>

char generateBackgroundLetter() {
  return 'A' + std::rand() % 26;
}

char generateLeftHandLetter() {
  std::vector<char> leftHandLetters = {
      'Q', 'W', 'E', 'R', 'T', 'A', 'S',
      'D', 'F', 'G', 'Z', 'X', 'C', 'V'};

  int index = std::rand() % leftHandLetters.size();
  return leftHandLetters[index];
}

char generateRightHandLetter() {
  std::vector<char> rightHandLetters = {
      'Y', 'U', 'I', 'O', 'P', 'H',
      'J', 'K', 'L', 'N', 'M', 'B'};

  int index = std::rand() % rightHandLetters.size();
  return rightHandLetters[index];
}

float generateBackgroundXPosition(int windowWidth) {
  return std::rand() % (windowWidth + 1);
}

float generateLeftHandXPosition(int windowWidth) {
  float padding = windowWidth * 0.15f;
  float minLeftX = padding;
  float maxLeftX = (windowWidth * 0.5f) - padding;

  return minLeftX +
         std::rand() % static_cast<int>(maxLeftX - minLeftX + 1);
}

float generateRightHandXPosition(int windowWidth) {
  float padding = windowWidth * 0.15f;
  float minRightX = (windowWidth * 0.5f) + padding;
  float maxRightX = windowWidth - padding;

  return minRightX +
         std::rand() % static_cast<int>(maxRightX - minRightX + 1);
}

float generateBackgroundLetterSpeed() {
  return 300 + std::rand() % 601;
}

float generateHardSpawnInterval() {
  return 0.5 + (std::rand() % 151) / 100.0;
}

float generateMediumSpawnInterval() {
  return 1.5 + (std::rand() % 151) / 100.0;
}

float generateEasySpawnInterval() {
  return 2.5 + (std::rand() % 151) / 100.0;
}