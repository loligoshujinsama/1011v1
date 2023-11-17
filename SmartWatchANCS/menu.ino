#define	BLACK           0x00
#define	BLUE            0xE0
#define	RED             0x03
#define	DGREEN          0x0C
#define WHITE           0xFF

typedef struct
{
  const uint8_t amtLines;
  const char* const * strings;
  void (*selectionHandler)(uint8_t);
} menu_info;

unsigned long timerDuration = 25 * 60 * 1000; // 25 minutes in milliseconds
boolean pomodoroActive = false; // Flag to indicate if Pomodoro timer is active
bool pomodoroPaused = false;
unsigned long elapsedTimeWhenPaused = 0; 
unsigned long pausedTime = 0; 

/* Global games variables */
bool slotsActive = false;
bool ballActive = false;
bool cointossActive = false;
bool dicesimulActive = false;
bool ccActive = false;
bool spsActive = false;
/* SPS Init */
enum Choice { SCISSORS, PAPER, STONE };
Choice playerChoice, computerChoice;
int number1 = 0;
int number2 = 0;
int number3 = 0;
int credits = 10;
bool slotRunning = false;
bool win = false;
/* * * * * * * * * * * * * */

uint8_t menuHistory[5];
uint8_t menuHistoryIndex = 0;
uint8_t currentMenu = 0;
uint8_t currentMenuLine = 0;
uint8_t lastMenuLine = -1;
uint8_t currentSelectionLine = 0;
uint8_t lastSelectionLine = -1;
long int rando = 0;  //Setting up the random number holder

void newMenu(int8_t newIndex) {
  currentMenuLine = 0;
  lastMenuLine = -1;
  currentSelectionLine = 0;
  lastSelectionLine = -1;
  if (newIndex >= 0) {
    menuHistory[menuHistoryIndex++] = currentMenu;
    currentMenu = newIndex;
  } else {
    if (currentDisplayState == displayStateMenu) {
      menuHistoryIndex--;
      currentMenu = menuHistory[menuHistoryIndex];
    }
  }
  if (menuHistoryIndex) {
    currentDisplayState = displayStateMenu;
    if (menu_debug_print)SerialMonitorInterface.print("New menu index ");
    if (menu_debug_print)SerialMonitorInterface.println(currentMenu);
  } else {
    if (menu_debug_print)SerialMonitorInterface.print("New menu index ");
    if (menu_debug_print)SerialMonitorInterface.println("home");
    currentDisplayState = displayStateHome;
    initHomeScreen();
  }
}

static const char PROGMEM mainMenuStrings0[] = "Set date/time";
static const char PROGMEM mainMenuStrings1[] = "Set auto off";
static const char PROGMEM mainMenuStrings2[] = "Set brightness";
static const char PROGMEM mainMenuStrings3[] = "Others"; // ADD more options here

static const char* const PROGMEM mainMenuStrings[] =
{
  mainMenuStrings0,
  mainMenuStrings1,
  mainMenuStrings2,
  mainMenuStrings3, // EDIT accordingly
};

const menu_info mainMenuInfo =
{
  4, // EDIT accordingly
  mainMenuStrings,
  mainMenu,
};

static const char PROGMEM dateTimeMenuStrings0[] = "Set Year";
static const char PROGMEM dateTimeMenuStrings1[] = "Set Month";
static const char PROGMEM dateTimeMenuStrings2[] = "Set Day";
static const char PROGMEM dateTimeMenuStrings3[] = "Set Hour";
static const char PROGMEM dateTimeMenuStrings4[] = "Set Minute";
static const char PROGMEM dateTimeMenuStrings5[] = "Set Second";

static const char* const PROGMEM dateTimeMenuStrings[] =
{
  dateTimeMenuStrings0,
  dateTimeMenuStrings1,
  dateTimeMenuStrings2,
  dateTimeMenuStrings3,
  dateTimeMenuStrings4,
  dateTimeMenuStrings5,
};

const menu_info dateTimeMenuInfo =
{
  6,
  dateTimeMenuStrings,
  dateTimeMenu,
};

//const menu_info menuList[] = {mainMenuInfo, dateTimeMenuInfo};
#define mainMenuIndex 0
#define dateTimeMenuIndex 1

int currentVal = 0;
int digits[4];
int currentDigit = 0;
int maxDigit = 4;
int *originalVal;
void (*editIntCallBack)() = NULL;

uint8_t editInt(uint8_t button, int *inVal, char *intName, void (*cb)()) {
  if (menu_debug_print)SerialMonitorInterface.println("editInt");
  if (!button) {
    if (menu_debug_print)SerialMonitorInterface.println("editIntInit");
    editIntCallBack = cb;
    currentDisplayState = displayStateEditor;
    editorHandler = editInt;
    currentDigit = 0;
    originalVal = inVal;
    currentVal = *originalVal;
    digits[3] = currentVal % 10; currentVal /= 10;
    digits[2] = currentVal % 10; currentVal /= 10;
    digits[1] = currentVal % 10; currentVal /= 10;
    digits[0] = currentVal % 10;
    currentVal = *originalVal;
    display.clearWindow(0, 12, 96, 64);
    display.setFont(font10pt);
    display.fontColor(defaultFontColor, defaultFontBG);
    display.setCursor(0, menuTextY[0]);
    display.print(F("< back/undo"));
    display.setCursor(90, menuTextY[0]);
    display.print('^');
    display.setCursor(10, menuTextY[1]);
    display.print(intName);
    display.setCursor(0, menuTextY[3]);
    display.print(F("< next/save"));
    display.setCursor(90, menuTextY[3]);
    display.print('v');
  } else if (button == upButton) {
    if (digits[currentDigit] < 9)
      digits[currentDigit]++;
  } else if (button == downButton) {
    if (digits[currentDigit] > 0)
      digits[currentDigit]--;
  } else if (button == selectButton) {
    if (currentDigit < maxDigit - 1) {
      currentDigit++;
    } else {
      //save
      int newValue = (digits[3]) + (digits[2] * 10) + (digits[1] * 100) + (digits[0] * 1000);
      *originalVal = newValue;
      viewMenu(backButton);
      if (editIntCallBack) {
        editIntCallBack();
        editIntCallBack = NULL;
      }
      return 1;
    }
  } else if (button == backButton) {
    if (currentDigit > 0) {
      currentDigit--;
    } else {
      if (menu_debug_print)SerialMonitorInterface.println(F("back"));
      viewMenu(backButton);
      return 0;
    }
  }
  display.setCursor(10, menuTextY[2]);
  for (uint8_t i = 0; i < 4; i++) {
    if (i != currentDigit)display.fontColor(inactiveFontColor, defaultFontBG);
    display.print(digits[i]);
    if (i != currentDigit)display.fontColor(defaultFontColor, defaultFontBG);
  }
  display.print(F("   "));
  return 0;
}

static const char PROGMEM gamesMenuStrings0[] = "Slot machine";
static const char PROGMEM gamesMenuStrings1[] = "8ball";
static const char PROGMEM gamesMenuStrings2[] = "Pomodoro Timer";
static const char PROGMEM gamesMenuStrings3[] = "Coin toss";
static const char PROGMEM gamesMenuStrings4[] = "Dice simul.";
static const char PROGMEM gamesMenuStrings5[] = "Scissors P S";
static const char PROGMEM gamesMenuStrings6[] = "Scuffed BlackJ";
static const char PROGMEM gamesMenuStrings7[] = "Credit Miner";


static const char* const PROGMEM gamesMenuStrings[] =
{
  gamesMenuStrings0,
  gamesMenuStrings1,
  gamesMenuStrings2,
  gamesMenuStrings3,
  gamesMenuStrings4,
  gamesMenuStrings5,
  gamesMenuStrings6,
  gamesMenuStrings7,
};

const menu_info gamesMenuInfo =
{
  8,
  gamesMenuStrings,
  gamesMenu,
};
#define gamesMenuIndex 2 // Adjust the index according to your menuList array

void updatePomodoroTimer() {
  if (!pomodoroPaused) { // Only update if Pomodoro is not paused
    unsigned long elapsedTime = millis() - startTime; // Calculate elapsed time

    if (elapsedTime >= timerDuration) {
      display.clearWindow(0, 25, 96, 39);
      display.setFont(thinPixel7_10ptFontInfo);
      display.setCursor(10, 25);
      display.fontColor(TS_8b_Red, TS_8b_Black);
      display.print("Time's up!");
      while (true) {
        // Loop indefinitely after timer is done
      }
    }

    unsigned long remainingTime = timerDuration - elapsedTime;
    unsigned long minutes = remainingTime / (60 * 1000);
    unsigned long seconds = (remainingTime / 1000) % 60;

    display.setFont(thinPixel7_10ptFontInfo);
    int width = display.getPrintWidth("Pomodoro Timer");
    display.setFlip(true);

    display.setCursor(0, 10);
    display.fontColor(TS_8b_White, TS_8b_Black);
    display.print("<");
    display.setCursor(0, 55);
    display.fontColor(TS_8b_White, TS_8b_Black);
    display.print("< Pause");
    display.setCursor(55, 55);
    display.print("Resume >");

    display.setCursor(48 - (width / 2), 10);
    display.fontColor(TS_8b_Green, TS_8b_Black);
    display.print("Pomodoro Timer");

    char timeString[10];
    sprintf(timeString, "%02lu:%02lu", minutes, seconds);

    display.clearWindow(0, 25, 96, 30);
    display.setFont(liberationSans_22ptFontInfo);
    display.setCursor(10, 25);
    display.fontColor(TS_8b_Blue, TS_8b_Black);
    display.print(timeString);
    
  }
}


void initializePomodoroTimer() {
  startTime = millis(); // Record the start time
}

void pausePomodoro() {
  if (!pomodoroPaused) {
    pausedTime = millis();
    pomodoroPaused = true;
  }
}

void resumePomodoro() {
  if (pomodoroPaused) {
    startTime = millis() - (pausedTime - startTime);
    pomodoroPaused = false;
  }
}

uint8_t getButtonPress() {
  if (digitalRead(display.getButtons(TSButtonLowerLeft)) == LOW) {
    delay(50);
    return upButton;
  } else if (digitalRead(display.getButtons(TSButtonLowerLeft)) == LOW) {
    delay(50);
    return downButton;
  }
  return 0;
}

void gamesMenu(uint8_t selection) {
  if (menu_debug_print) SerialMonitorInterface.print("gamesMenu ");
  if (menu_debug_print) SerialMonitorInterface.println(selection);

  if (selection == 0) {
    slotsActive = true;
    display.clearWindow(0, 12, 96, 64);
    display.setFont(font10pt);
    display.setCursor(15, menuTextY[0]);
    while (slotsActive) {
      slots();
    if (display.getButtons(TSButtonUpperLeft)) {
        if (slotsActive) {
          slotsActive = false;
        }
        display.clearWindow(0, 12, 96, 64);
        newMenu(gamesMenuIndex);
      }
    }
  } else if (selection == 1) {
      ballActive = true;
      display.clearWindow(0, 12, 96, 64);
      display.setFont(font10pt);
      display.setCursor(10, menuTextY[0]);
      while (ballActive) {
        rando = random(1, 9); //You need a random range 1 more than the number of statements you have
        rando = round(rando); //Round it to a whole number
        
        display.drawRect(0,23,96,64,TSRectangleFilled,DGREEN);
        display.setFont(font10pt);
        display.setCursor(0, 13);
        display.fontColor(TS_8b_White, TS_8b_Black);
        display.print("< Back");

        display.setFont(liberationSans_16ptFontInfo);
        display.fontColor(WHITE,DGREEN);
                        
        // Check for button presses
        if (display.getButtons(TSButtonUpperLeft)) {
          if (ballActive) {
            ballActive = false;
          }
          display.clearWindow(0, 10, 96, 64);
          newMenu(gamesMenuIndex);
        }

        accel.read(); //Read and manipulate accelerometer data
        int z = accel.Z;
        SerialMonitorInterface.println(z); //printing out Z values for debugging purposes

        // Use serial monitor to debug the values
        if (z < 40) {display.clearWindow(0,23,96,64); display.drawRect(0,23,96,64,TSRectangleFilled,DGREEN);
        delay(1000);}
        if (z > 170) {
          switch(rando) { //A switch case for each statement
            case 1: 
              accel.read();
              while (accel.Z > 150){
                display.setCursor(30,32);
                display.print("Yes"); //Here's the message that gets printed
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64); //It's important to clear the window after each graphic
            case 2:
              accel.read();
              while (accel.Z > 150){
                display.setCursor(37,32);
                display.print("No");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            case 3:
              accel.read();
              while (accel.Z > 150){
                display.setFont(liberationSans_10ptFontInfo);
                display.setCursor(17,30);
                display.print("Kane says");
                display.setCursor(37,42);
                display.print("OK");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            case 4:
              accel.read();
              while (accel.Z > 150){
                display.setFont(liberationSans_10ptFontInfo);
                display.setCursor(25,30);
                display.print("Are you");
                display.setCursor(30,42);
                display.print("dumb?");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            case 5:
              accel.read();
              while (accel.Z > 150){
                display.setFont(liberationSans_10ptFontInfo);
                display.setCursor(16,30);
                display.print("Steven say");
                display.setCursor(36,42);
                display.print("can");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            case 6:
              accel.read();
              while (accel.Z > 150){
                display.setFont(liberationSans_10ptFontInfo);
                display.setCursor(15,30);
                display.print("Can't predict");
                display.setCursor(37,43);
                display.print("now");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            case 7:
              accel.read();
              while (accel.Z > 150){
                display.setFont(liberationSans_10ptFontInfo);
                display.setCursor(20,30);
                display.print("Very good");
                display.setCursor(28,43);
                display.print("chance");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            case 8:
              accel.read();
              while (accel.Z > 150){
                display.setFont(liberationSans_10ptFontInfo);
                display.setCursor(33,30);
                display.print("Not a");
                display.setCursor(24,42);
                display.print("good idea");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,23,96,64);
            break;
          }
        }
      }
  } else if (selection == 2) {
    if (!pomodoroPaused) {
      startTime = millis(); // Update the existing startTime variable
    }
    pomodoroActive = true; // Set Pomodoro active flag
    display.clearWindow(0, 10, 96, 64);
    display.setFont(font10pt);
    display.setCursor(12, menuTextY[0]);

    // Use a loop to handle the Pomodoro Timer
    while (pomodoroActive) {
      if (!pomodoroPaused) {
        updatePomodoroTimer();
      }
      // Check for button presses
      if (display.getButtons(TSButtonLowerLeft)) {
        pausePomodoro();
      } else if (display.getButtons(TSButtonLowerRight)) {
        resumePomodoro();
      } else if (display.getButtons(TSButtonUpperLeft)) {
        if (pomodoroActive) {
          pomodoroActive = false;
        }
        display.clearWindow(0, 10, 96, 64);
        newMenu(gamesMenuIndex);
      }
    }
  } else if (selection == 3) {
    cointossActive = true;
    display.clearWindow(0, 12, 96, 64);
    display.setFont(font10pt);
    display.setCursor(15, menuTextY[0]);
    while (cointossActive) {
      coinToss();
    if (display.getButtons(TSButtonUpperLeft)) {
        if (cointossActive) {
          cointossActive = false;
        }
        display.clearWindow(0, 12, 96, 64);
        newMenu(gamesMenuIndex);
      }
    }
  } else if (selection == 4) {
    dicesimulActive = true;
    display.clearWindow(0, 12, 96, 64);
    display.setFont(font10pt);
    display.setCursor(15, menuTextY[0]);
    int xPos = 25;
    int yPos = 35;
    int xSpeed = 1;
    
    while (dicesimulActive) {
      xPos += xSpeed;
      if (xPos <= 0 || xPos >= 96) {
        xSpeed = -xSpeed; // Reverse the direction
    }
      rando = random(1,7);
      display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
      accel.read(); //Read and manipulate accelerometer data
      
      int a = 1;
      if (a > 0) {
          switch(rando) { //A switch case for each statement
            case 1: 
              accel.read();
              if (accel.Z > 250)  {
                display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
                
                display.drawPixel(xPos, yPos, TS_8b_Red); 
                display.setCursor(0, 12);
                display.print("<");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
                
              display.clearWindow(0,12,96,64); //It's important to clear the window after each graphic
              break;
            case 2:
              accel.read();
              if (accel.Z > 250) {
                display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
                
                display.drawPixel(xPos - 4, yPos - 4, TS_8b_Red); // Top-left dot
                display.drawPixel(xPos + 4, yPos + 4, TS_8b_Red); // Bottom-right dot
                display.setCursor(0, 12);
                display.print("<");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,12,96,64);
              break;
            case 3:
              accel.read();
              if (accel.Z > 250)  {
                display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
                // Draw the dice face with dots near two corners resembling the number 3
                display.drawPixel(xPos - 4, yPos - 4, TS_8b_Red); // Top-left dot
                display.drawPixel(xPos, yPos, TS_8b_Red);         // Center dot
                display.drawPixel(xPos + 4, yPos + 4, TS_8b_Red); // Bottom-right dot
                display.setCursor(0, 12);
                display.print("<");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,12,96,64);
              break;
            case 4:
              accel.read();
              if (accel.Z > 250) {
                display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
            
                // Draw the dice face with dots near two corners resembling the number 4
                display.drawPixel(xPos - 4, yPos - 4, TS_8b_Red); // Top-left dot
                display.drawPixel(xPos + 4, yPos - 4, TS_8b_Red); // Top-right dot
                display.drawPixel(xPos - 4, yPos + 4, TS_8b_Red); // Bottom-left dot
                display.drawPixel(xPos + 4, yPos + 4, TS_8b_Red); // Bottom-right dot
                display.setCursor(0, 12);
                display.print("<");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,12,96,64);
              break;
            case 5:
              accel.read();
              if (accel.Z > 250){
                display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
                display.drawPixel(xPos - 3, yPos - 3, TS_8b_Red); // Top-left dot
                display.drawPixel(xPos + 3, yPos - 3, TS_8b_Red); // Top-right dot
                display.drawPixel(xPos, yPos, TS_8b_Red);         // Center dot
                display.drawPixel(xPos - 3, yPos + 3, TS_8b_Red); // Bottom-left dot
                display.drawPixel(xPos + 3, yPos + 3, TS_8b_Red); // Bottom-right dot
                display.setCursor(0, 12);
                display.print("<");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,12,96,64);
              break;
            case 6:
              accel.read();
              if (accel.Z > 250){
                display.drawRect(xPos - 10, yPos - 10, 20, 20, TSRectangleNoFill, TS_8b_White);
                // Draw the dice face with dots near two corners resembling the number 6
                display.drawPixel(xPos - 3, yPos - 3, TS_8b_Red); // Top-left dot
                display.drawPixel(xPos + 3, yPos - 3, TS_8b_Red); // Top-right dot
                display.drawPixel(xPos - 3, yPos, TS_8b_Red);         // Middle left dot
                display.drawPixel(xPos + 3, yPos, TS_8b_Red); // Middle right dot
                display.drawPixel(xPos - 3, yPos + 3, TS_8b_Red); // Bottom-left dot
                display.drawPixel(xPos + 3, yPos + 3, TS_8b_Red); // Bottom-right dot
                display.setCursor(0, 12);
                display.print("<");
                SerialMonitorInterface.println(rando);
                delay(2000);
                accel.read();}
              display.clearWindow(0,12,96,64);
            break;
          }
      }
      if (display.getButtons(TSButtonUpperLeft)) {
        while (dicesimulActive) {
          dicesimulActive = false;
        }
        display.clearWindow(0, 12, 96, 64);
        newMenu(gamesMenuIndex);
      }
    }
  } else if (selection == 7) {
    display.clearWindow(0, 10, 96, 64);
    display.setFont(font10pt);
    display.setCursor(15, menuTextY[0]);
    ccActive = true;
    while (ccActive) {
      checkcredit();
      if (display.getButtons(TSButtonUpperLeft)) {
        ccActive = false;
        display.clearWindow(0, 10, 96, 64);
        newMenu(gamesMenuIndex);
      }
    }
  } else if (selection == 5) {
    display.clearWindow(0, 12, 96, 64);
    display.setFont(font10pt);
    display.setCursor(15, menuTextY[0]);
    spsActive = true;
    bool choiceMade = false;
    int x;
    spsDisplay();
    delay(2000);
    while (spsActive) {
        if (credits > 0) {
            if (!choiceMade) { 
                if (display.getButtons(TSButtonLowerLeft)) {
                    x = 0;
                    choiceMade = true;
                } else if (display.getButtons(TSButtonUpperRight)) {
                    x = 1;
                    choiceMade = true;
                } else if (display.getButtons(TSButtonLowerRight)) {
                    x = 2;
                    choiceMade = true;
                } else if (display.getButtons(TSButtonUpperLeft)) {
                    display.clearWindow(0, 12, 96, 64);
                    newMenu(gamesMenuIndex);
                    spsActive = false;
                }
            }

            if (choiceMade) {
                sps(x);
                // Reset the flag after processing the choice
                choiceMade = false;
            }
        } else {
            broke();
        }
    }
  } 
  pomodoroPaused = false; // Reset the pause flag when starting a new Pomodoro
}

void drawFilledCircle(int x, int y, int radius, uint16_t color) {
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            if (i * i + j * j <= radius * radius) {
                display.drawPixel(x + i, y + j, color);
            }
        }
    }
}

void coinToss() {
    int xPos = 25;
    int yPos = 35;
    static int yyPos = 16;
    static int yDir = 1;
    static bool tossing = false; // Track if tossing animation is in progress
    int result = 5; // Initialize result with a value that won't match 0 or 1

    // Check if the flip button is pressed to start tossing animation
    if (display.getButtons(TSButtonLowerLeft)) {
        tossing = true; // Start tossing animation
    }

    if (tossing) {
        // Clear the whole screen before starting the animation
        display.clearWindow(0, 12, 96, 64);

        // Draw the moving circle
        drawFilledCircle(48, yyPos, 4, TS_8b_Yellow);

        // Move the circle
        yyPos += yDir;

        // Check if the coin has reached the bottom or top
        if (yyPos >= 30 || yyPos <= 16) {
            yDir *= -1;

            // Check if the coin has reached the bottom
            if (yyPos >= 30) {
                result = random(2);
                tossing = false; // Stop tossing animation
            }
        }

        // Display the result after the flipping animation
        display.setFont(liberationSans_16ptFontInfo);
        display.setCursor(xPos, yPos);

        if (result == 0) {
            display.fontColor(TS_8b_Red, TS_8b_Black);
            display.print("Heads");
            
        } else if (result == 1) {
            display.fontColor(TS_8b_Blue, TS_8b_Black);
            display.print("Tails");
        }

        if (result == 0 || result == 1) {
            display.setFont(font10pt);
            display.fontColor(TS_8b_White, TS_8b_Black);

            display.setCursor(0, 12);
            display.print("<");
            display.setCursor(0, 55);
            display.fontColor(TS_8b_White, TS_8b_Black);
            display.print("< flip");
            display.setCursor(20, 12);
            display.print("Coin Toss");
            display.setCursor(20, 20);
        }

        delay(100); // Adjust delay for smoother animation
       
    }
}

void slots() {
  int xPos = 25;
  //25,51,96,39
  display.clearWindow(xPos, 51, 96, 5); // Updates the number portion
  if (credits > 0) {
    if (slotRunning) {
      number1 = random(10);
      number2 = random(10);
      number3 = random(10);
    }
    display.setFont(font10pt);
    display.fontColor(TS_8b_White, TS_8b_Black);

    display.setCursor(0, 12);
    display.print("<");
    display.setCursor(20, 12);
    display.print("$lot Machine");
    display.setCursor(20, 20);
    display.print("Credits: ");
    display.print(credits);
    display.setCursor(0, 55);
    display.fontColor(TS_8b_White, TS_8b_Black);
    display.print("< roll");
    display.setCursor(58, 55);
    display.print("stop >");

    display.setFont(liberationSans_16ptFontInfo);
    display.setCursor(xPos, 35);
    display.fontColor(TS_8b_Red, TS_8b_Black);
    display.print(number1);
    display.setCursor(xPos + 20, 35);
    display.fontColor(TS_8b_Blue, TS_8b_Black);
    display.print(number2);
    display.setCursor(xPos + 40, 35);
    display.fontColor(TS_8b_Green, TS_8b_Black);
    display.print(number3);
  

    int winXPos = (96 - display.getPrintWidth("WIN")) / 2;
    if (!slotRunning && number1 == number2 && number2 == number3) {
      display.setFont(liberationSans_16ptFontInfo);
      display.fontColor(TS_8b_White, TS_8b_Black);
      display.setCursor(winXPos, 35); 
      display.print("WIN");
      if (!win) {
        credits+=10;
        win = true;
      }
    }

    if (display.getButtons(TSButtonLowerRight)) {
      // Pauses the rolling state
      slotRunning = false;
    }

    if (!slotRunning && display.getButtons(TSButtonLowerLeft)) {
      credits--;
      win=false; // reset flag win when continue
      slotRunning = true;
      display.clearWindow(winXPos, 20, 96, 33);
    }

    delay(100); // Adjust the delay as needed
  } else {
    const unsigned char clownFace[] PROGMEM = {
      B00000000,
      B01111110,
      B10000001,
      B10100101,
      B10000001,
      B01111110,
    };
    display.clearWindow(0, 10, 96, 64);
    // Draw the clown face pixel by pixel
    int startX = 10;
    int startY = 10;
    int pixelSize = 3;  // Adjust the size of each pixel
    for (int y = 0; y < 6; y++) {
      for (int x = 0; x < 8; x++) {
        if (bitRead(clownFace[y], 7 - x)) {
          display.drawPixel(startX + x * pixelSize, startY + y * pixelSize, 0xFF);
          // Adjust the size of each pixel
          for (int i = 1; i < pixelSize; i++) {
            display.drawPixel(startX + x * pixelSize + i, startY + y * pixelSize, 0xFF);
          }
        }
      }
    }
  }
}

void checkcredit() {
  display.setFont(font10pt);
  int width = display.getPrintWidth("CreditMiner 2.0");
  display.setCursor(0, 12);
  display.print("<");
  display.setCursor(17, 12);
  display.print("CreditMiner 2.0");
  display.setCursor((width/2),30);
  display.setFont(liberationSans_16ptFontInfo);
  display.print(credits);
  movetoadd();
}

void movetoadd() {
  accel.read(); //Read and manipulate accelerometer data
  int z = accel.Z;
  SerialMonitorInterface.println(z); //printing out Z values for debugging purposes
  if (z>300) {
    delay(500); // Since it is captured in batches, need to bottleneck a little
    credits++;
  }
}


// Add the gamesMenuInfo to the menuList
const menu_info menuList[] = {mainMenuInfo, dateTimeMenuInfo, gamesMenuInfo}; 

// Invoking of functions in main menu 
void mainMenu(uint8_t selection) {
  if (menu_debug_print)SerialMonitorInterface.println("mainMenuHandler");
  if (selection == 0) {
    newMenu(dateTimeMenuIndex);
  }
  if (selection == 1) {
    char buffer[20];
    strcpy_P(buffer, (PGM_P)pgm_read_word(&(menuList[mainMenuIndex].strings[selection])));
    editInt(0, &sleepTimeout, buffer, NULL);
  }
  if (selection == 2) {
    char buffer[20];
    strcpy_P(buffer, (PGM_P)pgm_read_word(&(menuList[mainMenuIndex].strings[selection])));
    editInt(0, &brightness, buffer, NULL);
  }
  if (selection == 3) {
    newMenu(gamesMenuIndex);
  }
}

uint8_t dateTimeSelection = 0;
int dateTimeVariable = 0;

void saveChangeCallback() {
#if defined (ARDUINO_ARCH_AVR)
  int timeData[] = {year(), month(), day(), hour(), minute(), second()};
  timeData[dateTimeSelection] = dateTimeVariable;
  setTime(timeData[3], timeData[4], timeData[5], timeData[2], timeData[1], timeData[0]);
#elif defined(ARDUINO_ARCH_SAMD)
  int timeData[] = {RTCZ.getYear(), RTCZ.getMonth(), RTCZ.getDay(), RTCZ.getHours(), RTCZ.getMinutes(), RTCZ.getSeconds()};
  timeData[dateTimeSelection] = dateTimeVariable;
  RTCZ.setTime(timeData[3], timeData[4], timeData[5]);
  RTCZ.setDate(timeData[2], timeData[1], timeData[0] - 2000);
#endif
  if (menu_debug_print)SerialMonitorInterface.print("set time ");
  if (menu_debug_print)SerialMonitorInterface.println(dateTimeVariable);
}


void dateTimeMenu(uint8_t selection) {
  if (menu_debug_print)SerialMonitorInterface.print("dateTimeMenu ");
  if (menu_debug_print)SerialMonitorInterface.println(selection);
  if (selection >= 0 && selection < 6) {
#if defined (ARDUINO_ARCH_AVR)
    int timeData[] = {year(), month(), day(), hour(), minute(), second()};
#elif defined(ARDUINO_ARCH_SAMD)
    int timeData[] = {RTCZ.getYear(), RTCZ.getMonth(), RTCZ.getDay(), RTCZ.getHours(), RTCZ.getMinutes(), RTCZ.getSeconds()};
#endif
    dateTimeVariable = timeData[selection];
    dateTimeSelection = selection;
    char buffer[20];
    strcpy_P(buffer, (PGM_P)pgm_read_word(&(menuList[dateTimeMenuIndex].strings[selection])));
    editInt(0, &dateTimeVariable, buffer, saveChangeCallback);
  }
}

void viewMenu(uint8_t button) {
  if (menu_debug_print)SerialMonitorInterface.print("viewMenu ");
  if (menu_debug_print)SerialMonitorInterface.println(button);
  if (!button) {
    newMenu(mainMenuIndex);
    display.clearWindow(0, 12, 96, 64);
    pomodoroActive = false; // Set Pomodoro active flag to false
  } else {
    if (button == upButton) {
      if (currentSelectionLine > 0) {
        currentSelectionLine--;
      } else if (currentMenuLine > 0) {
        currentMenuLine--;
      }
    } else if (button == downButton) {
      if (currentSelectionLine < menuList[currentMenu].amtLines - 1 && currentSelectionLine < 3) {
        currentSelectionLine++;
      } else if (currentSelectionLine + currentMenuLine < menuList[currentMenu].amtLines - 1) {
        currentMenuLine++;
      }
    } else if (button == selectButton) {
      if (menu_debug_print)SerialMonitorInterface.print("select ");
      if (menu_debug_print)SerialMonitorInterface.println(currentMenuLine + currentSelectionLine);
      menuList[currentMenu].selectionHandler(currentMenuLine + currentSelectionLine);
    } else if (button == backButton) {
      newMenu(-1);
      if (!menuHistoryIndex)
        return;
    }
  }
  display.setFont(font10pt);
  if (lastMenuLine != currentMenuLine || lastSelectionLine != currentSelectionLine) {
    if (menu_debug_print)SerialMonitorInterface.println("drawing menu ");
    if (menu_debug_print)SerialMonitorInterface.println(currentMenu);
    for (int i = 0; i < 4; i++) {
      display.setCursor(7, menuTextY[i]);
      if (i == currentSelectionLine) {
        display.fontColor(defaultFontColor, inactiveFontBG);
      } else {
        display.fontColor(inactiveFontColor, inactiveFontBG);
      }
      if (currentMenuLine + i < menuList[currentMenu].amtLines) {
        char buffer[20];
        strcpy_P(buffer, (PGM_P)pgm_read_word(&(menuList[currentMenu].strings[currentMenuLine + i])));
        display.print(buffer);
      }
      for (uint8_t i = 0; i < 25; i++)display.write(' ');
      if (i == 0) {
        display.fontColor(defaultFontColor, inactiveFontBG);
        display.setCursor(0, menuTextY[0]);
        display.print('<');
        display.setCursor(90, menuTextY[0]);
        display.print('^');
      }
      if (i == 3) {
        display.fontColor(defaultFontColor, inactiveFontBG);
        display.setCursor(0, menuTextY[3]);
        display.print('>');
        display.setCursor(90, menuTextY[3]);
        display.print('v');
      }

    }
    lastMenuLine = currentMenuLine;
    lastSelectionLine = currentSelectionLine;
  }
}



void sps(int x) {
  // Use the three buttons to get player's choice
  if (x==0) {
   playerChoice = SCISSORS;
  } else if (x==1) {
    playerChoice = PAPER;
  } else if (x==2) {
    playerChoice = STONE;
  }

  // Generate computer's choice randomly
  computerChoice = static_cast<Choice>(random(0, 3));
  
  // Display the game result
  display.setFont(thinPixel7_10ptFontInfo);
  display.clearWindow(0, 12, 96, 64);

  // Center the result
  display.setCursor(30, 12);
  if (playerChoice == computerChoice) {
    display.fontColor(TS_8b_Blue, TS_8b_Black); // Blue for tie
    display.print("DRAW :O");
  } else if ((playerChoice == SCISSORS && computerChoice == PAPER) ||
             (playerChoice == PAPER && computerChoice == STONE) ||
             (playerChoice == STONE && computerChoice == SCISSORS)) {
    display.fontColor(TS_8b_Green, TS_8b_Black); // Green for win
    display.print("WIN :)");
    credits += 2;
  } else {
    display.fontColor(TS_8b_Red, TS_8b_Black); // Red for lose
    display.print("LOSS :(");
    credits -= 1;
  }

  // Show choices in two lines
  display.setFont(thinPixel7_10ptFontInfo);
  display.setCursor(24, 24); // for player's choice
  display.print("P: ");
  switch (playerChoice) {
    case SCISSORS: display.print("Scissors"); break;
    case PAPER: display.print("Paper"); break;
    case STONE: display.print("Stone"); break;
  }
  display.setCursor(24, 36); // for computer's choice
  display.print("C: ");
  switch (computerChoice) {
    case SCISSORS: display.print("Scissors"); break;
    case PAPER: display.print("Paper"); break;
    case STONE: display.print("Stone"); break;
  }
  display.setCursor(24, 48); // for credits
  display.print("Credits: ");
  display.print(credits);

  delay(3000); // Show for 3 seconds
  spsDisplay();
}

void broke() {
  display.clearWindow(0, 12, 96, 64);
  display.setFont(thinPixel7_10ptFontInfo);
  display.fontColor(TS_8b_White, TS_8b_Black);
  display.setCursor(30, 20);
  display.print("BROKIE");
  display.setCursor(23, 32);
  display.print("Come back");
  display.setCursor(15, 44);
  display.print("with credits!");

  delay(3000); // Show for 3 seconds
  spsDisplay();
}

void spsDisplay() {
  display.setFont(thinPixel7_10ptFontInfo); // Set thinner font for main menu
  display.fontColor(TS_8b_White, TS_8b_Black);
  display.clearWindow(0, 12, 96, 64);

  // Center the title
  display.setCursor(27, 24); // Adjusted for centering
  display.print("Scissors,");
  display.setCursor(18, 36); // New line for title
  display.print("Paper, Stone!");

  // Button indicators
  display.setCursor(0, 12); // Upper left
  display.print("<");

  display.setCursor(0, 55); // Lower left
  display.print("< Scissors");

  display.setCursor(60, 12); // Upper right
  display.print("Paper >");

  display.setCursor(60, 55); // Lower right
  display.print("Stone >");
}
