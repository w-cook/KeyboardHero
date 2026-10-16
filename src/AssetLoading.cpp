#include "AssetLoading.h"

#include <cstdint>
#include <iostream>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

using namespace sf;
using namespace std;

// Asset path resolution
static std::filesystem::path getExecutableDir() {
#if defined(__APPLE__)
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::string executablePathBuffer(size, '\0');

  if (_NSGetExecutablePath(executablePathBuffer.data(), &size) != 0) {
    return std::filesystem::current_path(); // fallback
  }

  return std::filesystem::weakly_canonical(executablePathBuffer).parent_path();
#else
  return std::filesystem::current_path();
#endif
}

std::filesystem::path assetPath(const std::string &insideAssets) {
  // First check for assets next to the executable.
  auto adjacentAssetPath = getExecutableDir() / "assets" / insideAssets;
  if (std::filesystem::exists(adjacentAssetPath))
    return adjacentAssetPath;

  // Check the macOS application bundle's Resources directory.
  auto bundleAssetPath = std::filesystem::weakly_canonical(
      getExecutableDir() / ".." / "Resources" / "assets" / insideAssets);

  if (std::filesystem::exists(bundleAssetPath))
    return bundleAssetPath;

  // Fall back to the working directory.
  return std::filesystem::current_path() / "assets" / insideAssets;
}

// Audio resource loading
void loadSoundBuffers(
    SoundBuffer &titleSongBuffer,
    SoundBuffer &titleTransSoundBuffer,
    SoundBuffer &gameSongBuffer,
    SoundBuffer &correctSoundBuffer,
    SoundBuffer &missSoundBuffer) {
  if (!titleSongBuffer.loadFromFile(
          assetPath("Audio/StartMenu-Song.wav").string())) {
    cerr << "Failed to load title song" << endl;
  }

  if (!titleTransSoundBuffer.loadFromFile(
          assetPath("Audio/StartMenuTransition-Sound.wav").string())) {
    cerr << "Failed to load title transition sound" << endl;
  }

  if (!gameSongBuffer.loadFromFile(
          assetPath("Audio/gameSong1.wav").string())) {
    cerr << "Failed to load game song" << endl;
  }

  if (!correctSoundBuffer.loadFromFile(
          assetPath("Audio/correctHit.wav").string())) {
    cerr << "Failed to load correct hit sound" << endl;
  }

  if (!missSoundBuffer.loadFromFile(
          assetPath("Audio/missSound.wav").string())) {
    cerr << "Failed to load miss sound" << endl;
  }
}