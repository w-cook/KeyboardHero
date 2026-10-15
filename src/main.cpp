#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace sf;
using namespace std;

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

// Letter types
struct BackgroundLetter {
  Text text;
  float speed;
};

struct LeftHandLetter {
  Text text;
  float speed;
};

struct RightHandLetter {
  Text text;
  float speed;
};

// Input and feedback types
enum class TypingHand {
  None,
  Left,
  Right
};

struct MenuInputContext {
  RenderWindow &menuWindow;
  RenderWindow &gameWindow;
  Sound &titleSong;
  Sound &transitionSound;

  float &leftSpawnInterval;
  float &leftLetterSpeed;
  float &rightSpawnInterval;
  float &rightLetterSpeed;
};

struct HandFeedbackState {
  int &streak;
  int &missStreak;
  Text &missText;
  Text &correctText;
  bool &showMissMessage;
  bool &showCorrectMessage;
  Clock &messageClock;
};

// Global state (ownership to be addressed in Phase 2)
vector<BackgroundLetter> menuBackgroundLetters;
vector<LeftHandLetter> leftFallingLetters;
vector<RightHandLetter> rightFallingLetters;

bool isPaused = false;

// Asset utilities
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

static std::filesystem::path assetPath(const std::string &insideAssets) {
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

static void loadSoundBuffers(
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

// Random generation
char generateBackgroundLetter() { return 'A' + rand() % 26; }

char generateLeftHandLetter() {
  vector<char> leftHandLetters = {'Q', 'W', 'E', 'R', 'T', 'A', 'S',
                                  'D', 'F', 'G', 'Z', 'X', 'C', 'V'};
  int index = rand() % leftHandLetters.size();
  return leftHandLetters[index];
}

char generateRightHandLetter() {
  vector<char> rightHandLetters = {'Y', 'U', 'I', 'O', 'P', 'H',
                                   'J', 'K', 'L', 'N', 'M', 'B'};
  int index = rand() % rightHandLetters.size();
  return rightHandLetters[index];
}

float generateBackgroundXPosition(int windowWidth) { return rand() % (windowWidth + 1); }

float generateLeftHandXPosition(int windowWidth) {
  float padding = windowWidth * 0.15f;
  float minLeftX = padding;
  float maxLeftX = (windowWidth * 0.5f) - padding;
  return minLeftX + rand() % static_cast<int>(maxLeftX - minLeftX + 1);
}

float generateRightHandXPosition(int windowWidth) {
  float padding = windowWidth * 0.15f;
  float minRightX = (windowWidth * 0.5f) + padding;
  float maxRightX = windowWidth - padding;
  return minRightX + rand() % static_cast<int>(maxRightX - minRightX + 1);
}

float generateBackgroundLetterSpeed() { return 300 + rand() % 601; }

float generateHardSpawnInterval() {
  return 0.5 + (rand() % 151) / 100.0;
} // 0.5 - 2.0 step 0.01
float generateMediumSpawnInterval() {
  return 1.5 + (rand() % 151) / 100.0;
} // 1.5 - 3.0 step 0.01
float generateEasySpawnInterval() {
  return 2.5 + (rand() % 151) / 100.0;
} // 2.5 - 4.0 step 0.01

// Main menu
static void configureMenuTexts(
    Vector2u windowSize,
    Text &titleText,
    Text &menuText,
    Text &loadingText) {
  // Configure the title.
  titleText.setString("Keyboard Hero");
  titleText.setCharacterSize(72);
  titleText.setStyle(Text::Bold);
  titleText.setFillColor(Color::White);

  FloatRect titleTextSize = titleText.getLocalBounds();
  titleText.setOrigin({
      titleTextSize.size.x / 2.f,
      titleTextSize.size.y / 2.f});
  titleText.setPosition({
      windowSize.x / 2.f,
      windowSize.y / 4.f});

  // Configure the difficulty-selection menu.
  menuText.setString("1 -> Easy\n\n2 -> Medium\n\n3 -> "
                     "Hard\n\nQ -> Quit");
  menuText.setCharacterSize(36);
  menuText.setFillColor(Color::Green);

  FloatRect menuTextSize = menuText.getLocalBounds();
  menuText.setOrigin({
      menuTextSize.size.x / 2.f,
      menuTextSize.size.y / 2.f});
  menuText.setPosition({
      windowSize.x / 2.f,
      windowSize.y / 1.5f});

  // Configure the introductory loading message.
  loadingText.setString(
      "Loading...please do not press any keys\nKeyboardHero "
      "may result in KeyboardFailure");
  loadingText.setCharacterSize(36);
  loadingText.setFillColor(Color::Yellow);

  FloatRect loadingTextSize = loadingText.getLocalBounds();
  loadingText.setOrigin({
      loadingTextSize.size.x / 2.f,
      loadingTextSize.size.y / 2.f});
  loadingText.setPosition({
      windowSize.x / 2.f,
      windowSize.y - 76.f});
}

static void updateAndDrawMenuBackground(
    RenderWindow &window,
    const Font &font,
    vector<BackgroundLetter> &letters,
    Clock &spawnClock,
    float spawnInterval,
    float dt) {
  // Spawn a new background letter when the interval elapses.
  if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
    Text text(font);
    char character = generateBackgroundLetter();

    text.setString(character);
    text.setCharacterSize(36);
    text.setFillColor(Color::Green);

    // Color the first and every subsequent 13th background letter red.
    if (letters.size() % 13 == 0) {
      text.setFillColor(Color::Red);
    }

    FloatRect textBounds = text.getLocalBounds();
    text.setOrigin({
        textBounds.size.x / 2.f,
        textBounds.size.y / 2.f});

    float xPosition = generateBackgroundXPosition(window.getSize().x);
    text.setPosition({
        xPosition,
        -20.f});

    float speed = generateBackgroundLetterSpeed();
    letters.push_back({
        text,
        speed});

    spawnClock.restart();
  }

  // Update and draw existing background letters.
  for (auto &letter : letters) {
    letter.text.move({
        0,
        letter.speed * dt});
    window.draw(letter.text);
  }
}

static void configureDifficulty(
    Keyboard::Scancode selection,
    float &leftSpawnInterval,
    float &leftLetterSpeed,
    float &rightSpawnInterval,
    float &rightLetterSpeed) {
  switch (selection) {
    case Keyboard::Scancode::Num1:
      // Easy
      leftSpawnInterval = generateEasySpawnInterval();
      leftLetterSpeed = 200.f;
      rightSpawnInterval = generateEasySpawnInterval();
      rightLetterSpeed = 200.f;
      break;

    case Keyboard::Scancode::Num2:
      // Medium
      leftSpawnInterval = generateMediumSpawnInterval();
      leftLetterSpeed = 300.f;
      rightSpawnInterval = generateMediumSpawnInterval();
      rightLetterSpeed = 300.f;
      break;

    case Keyboard::Scancode::Num3:
      // Hard
      leftSpawnInterval = generateHardSpawnInterval();
      leftLetterSpeed = 400.f;
      rightSpawnInterval = generateHardSpawnInterval();
      rightLetterSpeed = 400.f;
      break;

    default:
      break;
  }
}

static void handleMenuKeyPress(
    Keyboard::Scancode scancode,
    MenuInputContext &context) {
  // Q -> quit
  if (scancode == Keyboard::Scancode::Q) {
    context.menuWindow.close();
    context.gameWindow.close();
    context.titleSong.stop();
  }

  // Select difficulty and transition into gameplay.
  if (scancode == Keyboard::Scancode::Num1 ||
      scancode == Keyboard::Scancode::Num2 ||
      scancode == Keyboard::Scancode::Num3) {

    context.menuWindow.close();
    context.titleSong.stop();
    context.transitionSound.play();
    context.gameWindow.display();

    configureDifficulty(
        scancode,
        context.leftSpawnInterval,
        context.leftLetterSpeed,
        context.rightSpawnInterval,
        context.rightLetterSpeed);
  }
}

// Gameplay UI and targeting
static void configureFeedbackText(
    Text &text,
    const char *message,
    const Color &color,
    Vector2f position) {
  text.setString(message);
  text.setCharacterSize(24);
  text.setFillColor(color);

  FloatRect textSize = text.getLocalBounds();
  text.setOrigin({
      textSize.size.x / 2.f,
      textSize.size.y / 2.f});

  text.setPosition(position);
}

static void configureGameplayHudTexts(
    Vector2u windowSize,
    Text &instructionsText,
    Text &leftStreakText,
    Text &rightStreakText,
    int leftStreak,
    int rightStreak) {
  // Configure gameplay instructions.
  instructionsText.setString(
      "TAB -> Pause\n\nESC -> Quit\n\nCTL -> Mute Song");
  instructionsText.setCharacterSize(36);
  instructionsText.setFillColor(Color::White);

  FloatRect instructionsSize = instructionsText.getLocalBounds();
  instructionsText.setOrigin({
      instructionsSize.size.x / 2.f,
      instructionsSize.size.y / 2.f});
  instructionsText.setPosition({
      windowSize.x / 2.f,
      windowSize.y / 2.f});

  // Configure left-hand streak counter.
  leftStreakText.setString("Streak: " + to_string(leftStreak));
  leftStreakText.setCharacterSize(36);
  leftStreakText.setFillColor(Color::White);

  FloatRect leftStreakSize = leftStreakText.getLocalBounds();
  leftStreakText.setOrigin({
      leftStreakSize.size.x / 2.f,
      leftStreakSize.size.y / 2.f});
  leftStreakText.setPosition({
      windowSize.x / 4.f,
      windowSize.y - (windowSize.y / 20.f)});

  // Configure right-hand streak counter.
  rightStreakText.setString("Streak: " + to_string(rightStreak));
  rightStreakText.setCharacterSize(36);
  rightStreakText.setFillColor(Color::White);

  FloatRect rightStreakSize = rightStreakText.getLocalBounds();
  rightStreakText.setOrigin({
      rightStreakSize.size.x / 2.f,
      rightStreakSize.size.y / 2.f});
  rightStreakText.setPosition({
      windowSize.x - (windowSize.x / 4.f),
      windowSize.y - (windowSize.y / 20.f)});
}

static RectangleShape createTargetBox(Vector2u windowSize) {
  RectangleShape targetBox(
      {static_cast<float>(windowSize.x),
       72.f});

  targetBox.setOrigin({
      targetBox.getSize().x / 2.f,
      targetBox.getSize().y / 2.f});

  targetBox.setPosition({
      windowSize.x / 2.f,
      windowSize.y - (windowSize.y / 8.5f)});

  targetBox.setOutlineColor(Color::Green);
  targetBox.setOutlineThickness(1.f);
  targetBox.setFillColor(Color::Transparent);

  return targetBox;
}

static bool isLetterInTargetZone(
    const Text &letter,
    const RectangleShape &targetBox) {
  const float letterY = letter.getPosition().y;
  const float targetY = targetBox.getPosition().y;

  return letterY >= (targetY - 54.f) &&
         letterY <= (targetY + 36.f);
}

// Falling letters
static void spawnLeftLetter(
    vector<LeftHandLetter> &letters,
    const Font &font,
    int windowWidth,
    float speed) {
  Text letterText(font);
  char letter = generateLeftHandLetter();

  letterText.setString(letter);
  letterText.setCharacterSize(36);
  letterText.setFillColor(Color::Green);

  FloatRect textSize = letterText.getLocalBounds();
  letterText.setOrigin({
      textSize.size.x / 2.f,
      textSize.size.y / 2.f});

  float x = generateLeftHandXPosition(windowWidth);
  letterText.setPosition({
      x,
      -20.f});

  letters.push_back({
      letterText,
      speed});
}

static void spawnRightLetter(
    vector<RightHandLetter> &letters,
    const Font &font,
    int windowWidth,
    float speed) {
  Text letterText(font);
  char letter = generateRightHandLetter();

  letterText.setString(letter);
  letterText.setCharacterSize(36);
  letterText.setFillColor(Color::Green);

  FloatRect textSize = letterText.getLocalBounds();
  letterText.setOrigin({
      textSize.size.x / 2.f,
      textSize.size.y / 2.f});

  float x = generateRightHandXPosition(windowWidth);
  letterText.setPosition({
      x,
      -20.f});

  letters.push_back({
      letterText,
      speed});
}

static void updateAndDrawGameplayLetters(
    RenderWindow &window,
    vector<LeftHandLetter> &leftLetters,
    vector<RightHandLetter> &rightLetters,
    float dt) {
  // Move and draw left-hand letters.
  for (auto &letter : leftLetters) {
    letter.text.move({
        0,
        letter.speed * dt});
    window.draw(letter.text);
  }

  // Move and draw right-hand letters.
  for (auto &letter : rightLetters) {
    letter.text.move({
        0,
        letter.speed * dt});
    window.draw(letter.text);
  }
}

// Scoring and feedback
static void setFeedbackTextSize(
    Text &messageText,
    int streak) {
  if (streak <= 3) {
    messageText.setCharacterSize(24);
  } else if (streak <= 6) {
    messageText.setCharacterSize(30);
  } else {
    messageText.setCharacterSize(36);
  }
}

static void recordMiss(
    int &streak,
    int &missStreak,
    Text &missText,
    bool &showMissMessage,
    Clock &messageClock,
    Sound &missSound) {
  ++missStreak;
  streak = 0;

  setFeedbackTextSize(
      missText,
      missStreak);

  messageClock.restart();
  showMissMessage = true;
  missSound.play();
}

static void recordHit(
    int &streak,
    int &missStreak,
    Text &correctText,
    bool &showCorrectMessage,
    Clock &messageClock,
    Sound &correctSound) {
  ++streak;
  missStreak = 0;

  setFeedbackTextSize(
      correctText,
      streak);

  showCorrectMessage = true;
  messageClock.restart();
  correctSound.play();
}

static void drawTimedFeedback(
    RenderWindow &window,
    Text &message,
    bool &showMessage,
    const Clock &messageClock) {
  if (showMessage) {
    window.draw(message);

    if (messageClock.getElapsedTime().asSeconds() >= 0.25f) {
      showMessage = false;
    }
  }
}

// Gameplay input
static void handleGameplayControls(
    Keyboard::Scancode scancode,
    RenderWindow &gameWindow,
    Sound &gameSong,
    bool &isPaused,
    bool &isMuted) {
  // Escape -> quit
  if (scancode == Keyboard::Scancode::Escape) {
    gameWindow.close();
    gameSong.stop();
  }

  // Tab -> pause or resume
  if (scancode == Keyboard::Scancode::Tab) {
    isPaused = !isPaused;

    if (gameSong.getStatus() == SoundSource::Status::Playing) {
      gameSong.pause();
    } else {
      gameSong.play();
    }
  }

  // Ctrl -> mute or unmute gameplay music
  if (scancode == Keyboard::Scancode::LControl ||
      scancode == Keyboard::Scancode::RControl) {
    isMuted = !isMuted;

    if (isMuted) {
      gameSong.setVolume(0);
    } else {
      gameSong.setVolume(60);
    }
  }
}

static TypingHand classifyTypingKey(Keyboard::Scancode scancode) {
  switch (scancode) {
    case Keyboard::Scancode::Q:
    case Keyboard::Scancode::W:
    case Keyboard::Scancode::E:
    case Keyboard::Scancode::R:
    case Keyboard::Scancode::T:
    case Keyboard::Scancode::A:
    case Keyboard::Scancode::S:
    case Keyboard::Scancode::D:
    case Keyboard::Scancode::F:
    case Keyboard::Scancode::G:
    case Keyboard::Scancode::Z:
    case Keyboard::Scancode::X:
    case Keyboard::Scancode::C:
    case Keyboard::Scancode::V:
      return TypingHand::Left;

    case Keyboard::Scancode::Y:
    case Keyboard::Scancode::U:
    case Keyboard::Scancode::I:
    case Keyboard::Scancode::O:
    case Keyboard::Scancode::P:
    case Keyboard::Scancode::H:
    case Keyboard::Scancode::J:
    case Keyboard::Scancode::K:
    case Keyboard::Scancode::L:
    case Keyboard::Scancode::N:
    case Keyboard::Scancode::M:
    case Keyboard::Scancode::B:
      return TypingHand::Right;

    default:
      return TypingHand::None;
  }
}

static void handleLeftTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<LeftHandLetter> &letters,
    HandFeedbackState &feedback,
    Sound &missSound,
    Sound &correctSound) {
  // A keypress outside the target zone counts as a miss.
  if (!inTarget) {
    // Remove the first letter if one is available.
    if (!letters.empty()) {
      letters.erase(letters.begin());
    }

    recordMiss(
        feedback.streak,
        feedback.missStreak,
        feedback.missText,
        feedback.showMissMessage,
        feedback.messageClock,
        missSound);
  }

  // A matching keypress inside the target zone is a hit.
  if (inTarget &&
      !letters.empty() &&
      getDescription(scancode) ==
          letters.front().text.getString()) {
    letters.erase(letters.begin());

    recordHit(
        feedback.streak,
        feedback.missStreak,
        feedback.correctText,
        feedback.showCorrectMessage,
        feedback.messageClock,
        correctSound);
  }
}

static void handleRightTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<RightHandLetter> &letters,
    HandFeedbackState &feedback,
    Sound &missSound,
    Sound &correctSound) {
  // A keypress outside the target zone counts as a miss.
  if (!inTarget) {
    // Remove the first letter if one is available.
    if (!letters.empty()) {
      letters.erase(letters.begin());
    }

    recordMiss(
        feedback.streak,
        feedback.missStreak,
        feedback.missText,
        feedback.showMissMessage,
        feedback.messageClock,
        missSound);
  }

  // A matching keypress inside the target zone is a hit.
  if (inTarget &&
      !letters.empty() &&
      getDescription(scancode) ==
          letters.front().text.getString()) {
    letters.erase(letters.begin());

    recordHit(
        feedback.streak,
        feedback.missStreak,
        feedback.correctText,
        feedback.showCorrectMessage,
        feedback.messageClock,
        correctSound);
  }
}

// Off-screen miss handling
static void handleOffscreenMisses(
    float windowHeight,
    vector<LeftHandLetter> &leftLetters,
    vector<RightHandLetter> &rightLetters,
    HandFeedbackState &leftFeedback,
    HandFeedbackState &rightFeedback,
    Sound &missSound) {
  // Remove an off-screen left-hand letter and record a miss.
  if (!leftLetters.empty() &&
      leftLetters.front().text.getPosition().y >=
          windowHeight) {
    leftLetters.erase(leftLetters.begin());

    recordMiss(
        leftFeedback.streak,
        leftFeedback.missStreak,
        leftFeedback.missText,
        leftFeedback.showMissMessage,
        leftFeedback.messageClock,
        missSound);
  }

  // Remove an off-screen right-hand letter and record a miss.
  if (!rightLetters.empty() &&
      rightLetters.front().text.getPosition().y >=
          windowHeight) {
    rightLetters.erase(rightLetters.begin());

    recordMiss(
        rightFeedback.streak,
        rightFeedback.missStreak,
        rightFeedback.missText,
        rightFeedback.showMissMessage,
        rightFeedback.messageClock,
        missSound);
  }
}

// Application entry point
int main() {
  // Load the shared font.
  auto fontPath = assetPath("Fonts/FiraCodeNerdFontMono-Light.ttf");
  Font firaCodeFont;
  if (!firaCodeFont.openFromFile(fontPath)) {
    cerr << "Error loading firaCodeFont" << endl;
    return -1;
  }

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

  // Create and configure the main-menu text.
  Text titleText(firaCodeFont);
  Text menuText(firaCodeFont);
  Text loadingText(firaCodeFont);

  configureMenuTexts(
      menuWindow.getSize(),
      titleText,
      menuText,
      loadingText);

  bool showLoadingMessage = true;

  // Create the gameplay instructions.
  Text gameInstructionsText(firaCodeFont);

  // Gameplay feedback positions.
  const Vector2u gameSize = gameWindow.getSize();

  const Vector2f leftFeedbackPosition{
      gameSize.x / 9.5f,
      gameSize.y / 2.f};

  const Vector2f rightFeedbackPosition{
      gameSize.x - (gameSize.x / 10.f),
      gameSize.y / 2.f};

  // Left-hand miss feedback.
  Text missLeftText(firaCodeFont);
  configureFeedbackText(
      missLeftText,
      "Miss",
      Color::Red,
      leftFeedbackPosition);
  bool showLeftMissMessage = false;

  // Right-hand miss feedback.
  Text missRightText(firaCodeFont);
  configureFeedbackText(
      missRightText,
      "Miss",
      Color::Red,
      rightFeedbackPosition);
  bool showRightMissMessage = false;

  // Left-hand correct-hit feedback.
  Text correctLeftText(firaCodeFont);
  configureFeedbackText(
      correctLeftText,
      "Correct!",
      Color::Cyan,
      leftFeedbackPosition);
  bool showLeftCorrectMessage = false;

  // Right-hand correct-hit feedback.
  Text correctRightText(firaCodeFont);
  configureFeedbackText(
      correctRightText,
      "Correct!",
      Color::Cyan,
      rightFeedbackPosition);
  bool showRightCorrectMessage = false;

  // Initialize streak counters.
  int leftStreak = 0;
  int rightStreak = 0;
  int leftMissStreak = 0;
  int rightMissStreak = 0;

  // Create gameplay HUD text.
  Text leftStreakText(firaCodeFont);
  Text rightStreakText(firaCodeFont);

  configureGameplayHudTexts(
      gameWindow.getSize(),
      gameInstructionsText,
      leftStreakText,
      rightStreakText,
      leftStreak,
      rightStreak);

  // Create the gameplay target bar.
  RectangleShape targetBox = createTargetBox(gameWindow.getSize());

  bool leftLetterIsInTargetZone = false;
  bool rightLetterIsInTargetZone = false;

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
  Clock backgroundSpawnClock;
  Clock leftSpawnClock;
  Clock rightSpawnClock;
  Clock leftMessageClock;
  Clock rightMessageClock;
  Clock loadingClock;
  float backgroundSpawnInterval = 0.2f;

  HandFeedbackState leftFeedback{
      leftStreak,
      leftMissStreak,
      missLeftText,
      correctLeftText,
      showLeftMissMessage,
      showLeftCorrectMessage,
      leftMessageClock};
  HandFeedbackState rightFeedback{
      rightStreak,
      rightMissStreak,
      missRightText,
      correctRightText,
      showRightMissMessage,
      showRightCorrectMessage,
      rightMessageClock};

  // Difficulty-dependent letter settings.
  float leftLetterSpeed;
  float rightLetterSpeed;
  float leftSpawnInterval;
  float rightSpawnInterval;

  MenuInputContext menuInput{
      menuWindow,
      gameWindow,
      titleSong,
      titleTransSound,
      leftSpawnInterval,
      leftLetterSpeed,
      rightSpawnInterval,
      rightLetterSpeed};

  // Start the title music.
  titleSong.setLooping(true);
  titleSong.setVolume(50);
  titleSong.play();

  // Main-menu loop.
  while (menuWindow.isOpen()) {
    float dt = gameClock.getElapsedTime().asSeconds();
    if (dt >= 1.f / 60.f) {
      gameClock.restart();

      while (const auto event = menuWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          menuWindow.close();
          titleSong.stop();
        }

        if (auto *key = event->getIf<sf::Event::KeyPressed>()) {
          handleMenuKeyPress(
              key->scancode,
              menuInput);
        }
      }

      menuWindow.clear();

      menuWindow.draw(titleText);
      menuWindow.draw(menuText);

      // Update and draw the animated menu background.
      updateAndDrawMenuBackground(
          menuWindow,
          firaCodeFont,
          menuBackgroundLetters,
          backgroundSpawnClock,
          backgroundSpawnInterval,
          dt);

      if (showLoadingMessage) {
        menuWindow.draw(loadingText);
        if (loadingClock.getElapsedTime().asSeconds() > 3.f) {
          showLoadingMessage = false;
          loadingClock.stop();
        }
      }

      menuWindow.display();
    }
  }

  // Start gameplay music.
  gameSong.setLooping(true);
  gameSong.setVolume(60);
  gameSong.play();
  bool isMuted = false;

  // Gameplay loop.
  while (gameWindow.isOpen()) {
    float dt = gameClock.getElapsedTime().asSeconds();
    if (dt >= 1.f / 60.f) {
      gameClock.restart();

      while (const auto event = gameWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          gameWindow.close();
          gameSong.stop();
        }

        if (auto *key = event->getIf<sf::Event::KeyPressed>()) {
          handleGameplayControls(
              key->scancode,
              gameWindow,
              gameSong,
              isPaused,
              isMuted);

          const TypingHand typingHand = classifyTypingKey(key->scancode);

          if (typingHand == TypingHand::Left) {
            handleLeftTypingKey(
                key->scancode,
                leftLetterIsInTargetZone,
                leftFallingLetters,
                leftFeedback,
                missSound,
                correctSound);
          }

          if (typingHand == TypingHand::Right) {
            handleRightTypingKey(
                key->scancode,
                rightLetterIsInTargetZone,
                rightFallingLetters,
                rightFeedback,
                missSound,
                correctSound);
          }
        }
      }

      gameWindow.clear();

      if (!isPaused) {
        gameWindow.draw(targetBox);

        // Spawn left-hand letters at the configured interval.
        if (leftSpawnClock.getElapsedTime().asSeconds() > leftSpawnInterval) {
          spawnLeftLetter(
              leftFallingLetters,
              firaCodeFont,
              gameWindow.getSize().x,
              leftLetterSpeed);

          leftSpawnClock.restart();
        }

        // Spawn right-hand letters at the configured interval.
        if (rightSpawnClock.getElapsedTime().asSeconds() > rightSpawnInterval) {
          spawnRightLetter(
              rightFallingLetters,
              firaCodeFont,
              gameWindow.getSize().x,
              rightLetterSpeed);

          rightSpawnClock.restart();
        }

        // Update and draw falling gameplay letters.
        updateAndDrawGameplayLetters(
            gameWindow,
            leftFallingLetters,
            rightFallingLetters,
            dt);

        // Remove off-screen letters and record misses.
        handleOffscreenMisses(
            static_cast<float>(gameWindow.getSize().y),
            leftFallingLetters,
            rightFallingLetters,
            leftFeedback,
            rightFeedback,
            missSound);

        // Check whether the first left-hand letter is in the target zone.
        leftLetterIsInTargetZone =
            !leftFallingLetters.empty() &&
            isLetterInTargetZone(
                leftFallingLetters.front().text,
                targetBox);

        // Check whether the first right-hand letter is in the target zone.
        rightLetterIsInTargetZone =
            !rightFallingLetters.empty() &&
            isLetterInTargetZone(
                rightFallingLetters.front().text,
                targetBox);

        // Draw active hit/miss feedback for both hands.
        drawTimedFeedback(
            gameWindow,
            missLeftText,
            showLeftMissMessage,
            leftMessageClock);

        drawTimedFeedback(
            gameWindow,
            missRightText,
            showRightMissMessage,
            rightMessageClock);

        drawTimedFeedback(
            gameWindow,
            correctLeftText,
            showLeftCorrectMessage,
            leftMessageClock);

        drawTimedFeedback(
            gameWindow,
            correctRightText,
            showRightCorrectMessage,
            rightMessageClock);

        leftStreakText.setString("Streak: " + to_string(leftStreak));
        rightStreakText.setString("Streak: " + to_string(rightStreak));
        gameWindow.draw(gameInstructionsText);
        gameWindow.draw(leftStreakText);
        gameWindow.draw(rightStreakText);
        gameWindow.display();
      }
    }
  }

  return 0;
}
