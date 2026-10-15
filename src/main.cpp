#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace sf;
using namespace std;

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

// path functions
static std::filesystem::path getExecutableDir() {
#if defined(__APPLE__)
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::string buf(size, '\0');
  if (_NSGetExecutablePath(buf.data(), &size) != 0) {
    return std::filesystem::current_path(); // fallback
  }
  return std::filesystem::weakly_canonical(buf).parent_path();
#else
  return std::filesystem::current_path();
#endif
}

static std::filesystem::path assetPath(const std::string &insideAssets) {
  // Common case: assets folder sits next to the executable
  auto p = getExecutableDir() / "assets" / insideAssets;
  if (std::filesystem::exists(p))
    return p;

  // macOS .app bundle case: .../MyGame.app/Contents/Resources/assets/...
  auto p2 = std::filesystem::weakly_canonical(
      getExecutableDir() / ".." / "Resources" / "assets" / insideAssets);
  if (std::filesystem::exists(p2))
    return p2;

  // last resort: working directory
  return std::filesystem::current_path() / "assets" / insideAssets;
}

// random letter functions
char random_letter_generator() { return 'A' + rand() % 26; }

char random_left_letter_generator() {
  vector<char> leftHandLetters = {'Q', 'W', 'E', 'R', 'T', 'A', 'S',
                                  'D', 'F', 'G', 'Z', 'X', 'C', 'V'};
  int index = rand() % leftHandLetters.size();
  return leftHandLetters[index];
}

char random_right_letter_generator() {
  vector<char> rightHandLetters = {'Y', 'U', 'I', 'O', 'P', 'H',
                                   'J', 'K', 'L', 'N', 'M', 'B'};
  int index = rand() % rightHandLetters.size();
  return rightHandLetters[index];
}

// random x value functions
float random_X_value(int windowSize) { return rand() % (windowSize + 1); }

float random_left_X_value(int windowSize) {
  float padding = windowSize * 0.15f;
  float minLeftX = padding;
  float maxLeftX = (windowSize * 0.5f) - padding;
  return minLeftX + rand() % static_cast<int>(maxLeftX - minLeftX + 1);
}

float random_right_X_value(int windowSize) {
  float padding = windowSize * 0.15f;
  float minRightX = (windowSize * 0.5f) + padding;
  float maxRightX = windowSize - padding;
  return minRightX + rand() % static_cast<int>(maxRightX - minRightX + 1);
}

float random_speed() { return 300 + rand() % 601; }

float random_spawn_float_h() {
  return 0.5 + (rand() % 151) / 100.0;
} // 0.5 - 2.0 step 0.01
float random_spawn_float_m() {
  return 1.5 + (rand() % 151) / 100.0;
} // 1.5 - 3.0 step 0.01
float random_spawn_float_e() {
  return 2.5 + (rand() % 151) / 100.0;
} // 2.5 - 4.0 step 0.01

struct randomLetter {
  Text randomLetterText;
  float randomLetterSpeed;
};
vector<randomLetter> randomLetters;

struct randomLeftLetter {
  Text randomLeftLetterText;
  float speed;
  float center = randomLeftLetterText.getLocalBounds().getCenter().y;
};
vector<randomLeftLetter> randomLeftLetters;

struct randomRightLetter {
  Text randomRightLetterText;
  float speed;
  float center = randomRightLetterText.getLocalBounds().getCenter().y;
};
vector<randomRightLetter> randomRightLetters;

bool isPaused = false;

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
  titleText.setOrigin(
      {titleTextSize.size.x / 2.f, titleTextSize.size.y / 2.f});
  titleText.setPosition(
      {windowSize.x / 2.f, windowSize.y / 4.f});

  // Configure the difficulty-selection menu.
  menuText.setString("1 -> Easy\n\n2 -> Medium\n\n3 -> "
                     "Hard\n\nQ -> Quit");
  menuText.setCharacterSize(36);
  menuText.setFillColor(Color::Green);

  FloatRect menuTextSize = menuText.getLocalBounds();
  menuText.setOrigin(
      {menuTextSize.size.x / 2.f, menuTextSize.size.y / 2.f});
  menuText.setPosition(
      {windowSize.x / 2.f, windowSize.y / 1.5f});

  // Configure the introductory loading message.
  loadingText.setString(
      "Loading...please do not press any keys\nKeyboardHero "
      "may result in KeyboardFailure");
  loadingText.setCharacterSize(36);
  loadingText.setFillColor(Color::Yellow);

  FloatRect loadingTextSize = loadingText.getLocalBounds();
  loadingText.setOrigin(
      {loadingTextSize.size.x / 2.f, loadingTextSize.size.y / 2.f});
  loadingText.setPosition(
      {windowSize.x / 2.f, windowSize.y - 76.f});
}

static void configureFeedbackText(
    Text &text,
    const char *message,
    const Color &color,
    Vector2f position) {

  text.setString(message);
  text.setCharacterSize(24);
  text.setFillColor(color);

  FloatRect textSize = text.getLocalBounds();
  text.setOrigin(
      {textSize.size.x / 2.f, textSize.size.y / 2.f});

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
  instructionsText.setOrigin(
      {instructionsSize.size.x / 2.f,
       instructionsSize.size.y / 2.f});
  instructionsText.setPosition(
      {windowSize.x / 2.f, windowSize.y / 2.f});

  // Configure left-hand streak counter.
  leftStreakText.setString("Streak: " + to_string(leftStreak));
  leftStreakText.setCharacterSize(36);
  leftStreakText.setFillColor(Color::White);

  FloatRect leftStreakSize = leftStreakText.getLocalBounds();
  leftStreakText.setOrigin(
      {leftStreakSize.size.x / 2.f,
       leftStreakSize.size.y / 2.f});
  leftStreakText.setPosition(
      {windowSize.x / 4.f,
       windowSize.y - (windowSize.y / 20.f)});

  // Configure right-hand streak counter.
  rightStreakText.setString("Streak: " + to_string(rightStreak));
  rightStreakText.setCharacterSize(36);
  rightStreakText.setFillColor(Color::White);

  FloatRect rightStreakSize = rightStreakText.getLocalBounds();
  rightStreakText.setOrigin(
      {rightStreakSize.size.x / 2.f,
       rightStreakSize.size.y / 2.f});
  rightStreakText.setPosition(
      {windowSize.x - (windowSize.x / 4.f),
       windowSize.y - (windowSize.y / 20.f)});
}

static RectangleShape createTargetBox(Vector2u windowSize) {
  RectangleShape targetBox(
      {static_cast<float>(windowSize.x), 72.f});

  targetBox.setOrigin(
      {targetBox.getSize().x / 2.f,
       targetBox.getSize().y / 2.f});

  targetBox.setPosition(
      {windowSize.x / 2.f,
       windowSize.y - (windowSize.y / 8.5f)});

  targetBox.setOutlineColor(Color::Green);
  targetBox.setOutlineThickness(1.f);
  targetBox.setFillColor(Color::Transparent);

  return targetBox;
}

static void updateAndDrawMenuBackground(
    RenderWindow &window,
    const Font &font,
    vector<randomLetter> &letters,
    Clock &spawnClock,
    float spawnInterval,
    float dt) {
  // Spawn a new background letter when the interval elapses.
  if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
    Text randomLetterText(font);
    char random_letter = random_letter_generator();

    randomLetterText.setString(random_letter);
    randomLetterText.setCharacterSize(36);
    randomLetterText.setFillColor(Color::Green);

    // Every 13th letter drops red.
    if (letters.size() % 13 == 0) {
      randomLetterText.setFillColor(Color::Red);
    }

    FloatRect randomLetterTextSize = randomLetterText.getLocalBounds();
    randomLetterText.setOrigin(
        {randomLetterTextSize.size.x / 2.f,
         randomLetterTextSize.size.y / 2.f});

    float randomXvalue = random_X_value(window.getSize().x);
    randomLetterText.setPosition({randomXvalue, -20.f});

    float randomSpeed = random_speed();
    letters.push_back({randomLetterText, randomSpeed});

    spawnClock.restart();
  }

  // Update and draw existing background letters.
  for (auto &letter : letters) {
    letter.randomLetterText.move({0, letter.randomLetterSpeed * dt});
    window.draw(letter.randomLetterText);
  }
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

static void configureDifficulty(
    Keyboard::Scancode selection,
    float &leftSpawnInterval,
    float &leftLetterSpeed,
    float &rightSpawnInterval,
    float &rightLetterSpeed) {

  switch (selection) {
  case Keyboard::Scancode::Num1:
    // Easy
    leftSpawnInterval = random_spawn_float_e();
    leftLetterSpeed = 200.f;
    rightSpawnInterval = random_spawn_float_e();
    rightLetterSpeed = 200.f;
    break;

  case Keyboard::Scancode::Num2:
    // Medium
    leftSpawnInterval = random_spawn_float_m();
    leftLetterSpeed = 300.f;
    rightSpawnInterval = random_spawn_float_m();
    rightLetterSpeed = 300.f;
    break;

  case Keyboard::Scancode::Num3:
    // Hard
    leftSpawnInterval = random_spawn_float_h();
    leftLetterSpeed = 400.f;
    rightSpawnInterval = random_spawn_float_h();
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

enum class TypingHand {
  None,
  Left,
  Right
};

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

static void spawnLeftLetter(
    vector<randomLeftLetter> &letters,
    const Font &font,
    int windowWidth,
    float speed) {

  Text letterText(font);
  char letter = random_left_letter_generator();

  letterText.setString(letter);
  letterText.setCharacterSize(36);
  letterText.setFillColor(Color::Green);

  FloatRect textSize = letterText.getLocalBounds();
  letterText.setOrigin(
      {textSize.size.x / 2.f, textSize.size.y / 2.f});

  float x = random_left_X_value(windowWidth);
  letterText.setPosition({x, -20.f});

  letters.push_back({letterText, speed});
}

static void spawnRightLetter(
    vector<randomRightLetter> &letters,
    const Font &font,
    int windowWidth,
    float speed) {

  Text letterText(font);
  char letter = random_right_letter_generator();

  letterText.setString(letter);
  letterText.setCharacterSize(36);
  letterText.setFillColor(Color::Green);

  FloatRect textSize = letterText.getLocalBounds();
  letterText.setOrigin(
      {textSize.size.x / 2.f, textSize.size.y / 2.f});

  float x = random_right_X_value(windowWidth);
  letterText.setPosition({x, -20.f});

  letters.push_back({letterText, speed});
}

static void updateAndDrawGameplayLetters(
    RenderWindow &window,
    vector<randomLeftLetter> &leftLetters,
    vector<randomRightLetter> &rightLetters,
    float dt) {

  // Move and draw left-hand letters.
  for (auto &letter : leftLetters) {
    letter.randomLeftLetterText.move({0, letter.speed * dt});
    window.draw(letter.randomLeftLetterText);
  }

  // Move and draw right-hand letters.
  for (auto &letter : rightLetters) {
    letter.randomRightLetterText.move({0, letter.speed * dt});
    window.draw(letter.randomRightLetterText);
  }
}

static void setFeedbackTextSize(Text &messageText, int streak) {
  if (streak <= 3) {
    messageText.setCharacterSize(24);
  } else if (streak <= 6) {
    messageText.setCharacterSize(30);
  } else {
    messageText.setCharacterSize(36);
  }
}

static bool isLetterInTargetZone(
    const Text &letter,
    const RectangleShape &targetBox) {

  const float letterY = letter.getPosition().y;
  const float targetY = targetBox.getPosition().y;

  return letterY >= (targetY - 54.f) &&
         letterY <= (targetY + 36.f);
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

  setFeedbackTextSize(missText, missStreak);

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

  setFeedbackTextSize(correctText, streak);

  showCorrectMessage = true;
  messageClock.restart();
  correctSound.play();
}

struct HandFeedbackState {
  int &streak;
  int &missStreak;
  Text &missText;
  Text &correctText;
  bool &showMissMessage;
  bool &showCorrectMessage;
  Clock &messageClock;
};

static void handleLeftTypingKey(
    Keyboard::Scancode scancode,
    bool inTarget,
    vector<randomLeftLetter> &letters,
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
          letters.front().randomLeftLetterText.getString()) {

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
    vector<randomRightLetter> &letters,
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
          letters.front().randomRightLetterText.getString()) {

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

static void handleOffscreenMisses(
    float windowHeight,
    vector<randomLeftLetter> &leftLetters,
    vector<randomRightLetter> &rightLetters,
    HandFeedbackState &leftFeedback,
    HandFeedbackState &rightFeedback,
    Sound &missSound) {

  // Remove an off-screen left-hand letter and record a miss.
  if (!leftLetters.empty() &&
      leftLetters.front().randomLeftLetterText.getPosition().y >=
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
      rightLetters.front().randomRightLetterText.getPosition().y >=
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

int main() {

  // settings
  ContextSettings settings;
  settings.antiAliasingLevel = 4;

  // load font - FiraCodeNerdFontMono-Light
  auto Fira_Path = assetPath("Fonts/FiraCodeNerdFontMono-Light.ttf");
  Font firacode;
  if (!firacode.openFromFile(Fira_Path)) {
    cerr << "Error loading firacode" << endl;
    return -1;
  }

  // create game window
  RenderWindow gameWindow(VideoMode({1920, 1080}), "KeyboardHero");
  gameWindow.setVerticalSyncEnabled(true);

  // create menu window
  RenderWindow window(VideoMode({1920, 1080}), "Main Menu");
  window.setVerticalSyncEnabled(true);

  // Create and configure the main-menu text.
  Text titleText(firacode);
  Text menuText(firacode);
  Text loadingText(firacode);

  configureMenuTexts(
      window.getSize(), titleText, menuText, loadingText);

  bool showLoadingMessage = true;

  // create game menu text
  Text gamemenuText(firacode);

  // Gameplay feedback positions.
  const Vector2u gameSize = gameWindow.getSize();

  const Vector2f leftFeedbackPosition{
      gameSize.x / 9.5f,
      gameSize.y / 2.f};

  const Vector2f rightFeedbackPosition{
      gameSize.x - (gameSize.x / 10.f),
      gameSize.y / 2.f};

  // Left-hand miss feedback.
  Text missLeftText(firacode);
  configureFeedbackText(
      missLeftText, "Miss", Color::Red, leftFeedbackPosition);
  bool showLMissMessage = false;

  // Right-hand miss feedback.
  Text missRightText(firacode);
  configureFeedbackText(
      missRightText, "Miss", Color::Red, rightFeedbackPosition);
  bool showRMissMessage = false;

  // Left-hand correct-hit feedback.
  Text correctLeftText(firacode);
  configureFeedbackText(
      correctLeftText, "Correct!", Color::Cyan, leftFeedbackPosition);
  bool showLCorrectMessage = false;

  // Right-hand correct-hit feedback.
  Text correctRightText(firacode);
  configureFeedbackText(
      correctRightText, "Correct!", Color::Cyan, rightFeedbackPosition);
  bool showRCorrectMessage = false;

  // counters
  int leftStreak = 0;
  int rightStreak = 0;
  int leftMissStreak = 0;
  int rightMissStreak = 0;
  
  // Create gameplay HUD text.
  Text leftStreakText(firacode);
  Text rightStreakText(firacode);

  configureGameplayHudTexts(
      gameWindow.getSize(),
      gamemenuText,
      leftStreakText,
      rightStreakText,
      leftStreak,
      rightStreak);
  
  // Create the gameplay target bar.
  RectangleShape targetBox = createTargetBox(gameWindow.getSize());

  bool inTargetL = false;
  bool inTargetR = false;

  // sound buffers
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

  // sound players
  Sound titleSong(titleSongBuffer);
  Sound titleTransSound(titleTransSoundBuffer);
  Sound gameSong(gameSongBuffer);
  Sound correctSound(correctSoundBuffer);
  Sound missSound(missSoundBuffer);

  // initialize clocks
  Clock gameClock;
  Clock spawnClock;
  Clock leftSpawnClock;
  Clock rightSpawnClock;
  Clock leftMessageClock;
  Clock rightMessageClock;
  Clock loadingClock;
  float bgSpawnInterval = 0.2f;

  HandFeedbackState leftFeedback{
      leftStreak,
      leftMissStreak,
      missLeftText,
      correctLeftText,
      showLMissMessage,
      showLCorrectMessage,
      leftMessageClock};
  HandFeedbackState rightFeedback{
      rightStreak,
      rightMissStreak,
      missRightText,
      correctRightText,
      showRMissMessage,
      showRCorrectMessage,
      rightMessageClock};

  // left and right letter attribute declarations
  float leftLetterSpeed;
  float rightLetterSpeed;
  float leftSpawnInterval;
  float rightSpawnInterval;

  MenuInputContext menuInput{
      window,
      gameWindow,
      titleSong,
      titleTransSound,
      leftSpawnInterval,
      leftLetterSpeed,
      rightSpawnInterval,
      rightLetterSpeed};

  // start title song
  titleSong.setLooping(true);
  titleSong.setVolume(50);
  titleSong.play();
  bool isLooping = titleSong.isLooping();

  // start menu window loop //
  while (window.isOpen()) {

    // start delta time
    float dt = gameClock.getElapsedTime().asSeconds();
    if (dt >= 1.f / 60.f) {

      gameClock.restart();

      // start event handling
      while (const auto event = window.pollEvent()) {
        if (event->is<Event::Closed>()) {
          window.close();
          titleSong.stop();
        }

        // keyboard input - key pressed //
        if (auto *key = event->getIf<sf::Event::KeyPressed>()) {

          handleMenuKeyPress(key->scancode, menuInput);
        } // END key pressed handling
      } // END menu loop event handling

      // start drawing - menu window
      window.clear();

      // draw menu titling text
      window.draw(titleText);
      window.draw(menuText);

      // Update and draw the animated menu background.
      updateAndDrawMenuBackground(
          window, firacode, randomLetters, spawnClock, bgSpawnInterval, dt);

      // show loading message
      if (showLoadingMessage) {
        window.draw(loadingText);
        if (loadingClock.getElapsedTime().asSeconds() > 3.f) {
          showLoadingMessage = false;
          loadingClock.stop();
        }
      }

      window.display();
    } // END delta time
  } // END main menu window loop

  // start game song
  gameSong.setLooping(true);
  gameSong.setVolume(60);
  gameSong.play();
  bool isMuted = false;

  // start game window (KeyboardHero)loop //
  while (gameWindow.isOpen()) {

    // start delta time
    float dt = gameClock.getElapsedTime().asSeconds();
    if (dt >= 1.f / 60.f) {

      gameClock.restart();

      // start event handling
      while (const auto event = gameWindow.pollEvent()) {
        if (event->is<Event::Closed>()) {
          gameWindow.close();
          gameSong.stop();
        }

        // keyboard input - key pressed //
        if (auto *key = event->getIf<sf::Event::KeyPressed>()) {

          handleGameplayControls(
              key->scancode, gameWindow, gameSong, isPaused, isMuted);

          const TypingHand typingHand = classifyTypingKey(key->scancode);

          if (typingHand == TypingHand::Left) {

            handleLeftTypingKey(
                key->scancode,
                inTargetL,
                randomLeftLetters,
                leftFeedback,
                missSound,
                correctSound);
          }

          if (typingHand == TypingHand::Right) {

            handleRightTypingKey(
                key->scancode,
                inTargetR,
                randomRightLetters,
                rightFeedback,
                missSound,
                correctSound);
          }

        } // END key pressed handling

      } // END game window loop event handling

      // start drawing - game window
      gameWindow.clear();

      if (!isPaused) {

        gameWindow.draw(targetBox);
       
        // Spawn left-hand letters at the configured interval.
        if (leftSpawnClock.getElapsedTime().asSeconds() > leftSpawnInterval) {
          spawnLeftLetter(
              randomLeftLetters,
              firacode,
              gameWindow.getSize().x,
              leftLetterSpeed);

          leftSpawnClock.restart();
        }

        // Spawn right-hand letters at the configured interval.
        if (rightSpawnClock.getElapsedTime().asSeconds() > rightSpawnInterval) {
          spawnRightLetter(
              randomRightLetters,
              firacode,
              gameWindow.getSize().x,
              rightLetterSpeed);

          rightSpawnClock.restart();
        }

        // Update and draw falling gameplay letters.
        updateAndDrawGameplayLetters(
            gameWindow, randomLeftLetters, randomRightLetters, dt);
   
        // Remove off-screen letters and record misses.
        handleOffscreenMisses(
            static_cast<float>(gameWindow.getSize().y),
            randomLeftLetters,
            randomRightLetters,
            leftFeedback,
            rightFeedback,
            missSound);
 
        // Check whether the first left-hand letter is in the target zone.
        inTargetL = !randomLeftLetters.empty() &&
                    isLetterInTargetZone(
                        randomLeftLetters.front().randomLeftLetterText,
                        targetBox);

        // Check whether the first right-hand letter is in the target zone.
        inTargetR = !randomRightLetters.empty() &&
                    isLetterInTargetZone(
                        randomRightLetters.front().randomRightLetterText,
                        targetBox);
        
        // Draw active hit/miss feedback for both hands.
        drawTimedFeedback(
            gameWindow, missLeftText, showLMissMessage, leftMessageClock);

        drawTimedFeedback(
            gameWindow, missRightText, showRMissMessage, rightMessageClock);

        drawTimedFeedback(
            gameWindow, correctLeftText, showLCorrectMessage, leftMessageClock);

        drawTimedFeedback(
            gameWindow, correctRightText, showRCorrectMessage, rightMessageClock);

        leftStreakText.setString("Streak: " + to_string(leftStreak));
        rightStreakText.setString("Streak: " + to_string(rightStreak));
        gameWindow.draw(gamemenuText);
        gameWindow.draw(leftStreakText);
        gameWindow.draw(rightStreakText);
        gameWindow.display();
      } // END !isPaused
    } // END delta time
  } // END game window loop

  return 0;
}
