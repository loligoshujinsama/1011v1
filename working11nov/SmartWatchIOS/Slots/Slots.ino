#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>

#define	BLACK           0x00
#define	BLUE            0xE0
#define	RED             0x03
#define	GREEN           0x1C
#define WHITE           0xFF
#define	GREY            0x6D

TinyScreen display = TinyScreen(TinyScreenDefault);

int number1 = 0;
int number2 = 0;
int number3 = 0;
int credits = 10;

bool slotRunning = false;
bool win = false;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  display.begin();
  display.setFont(liberationSans_8ptFontInfo);
  display.fontColor(WHITE, BLACK);
  randomSeed(analogRead(0));
}

void loop() {
  int xPos = 25;

  display.clearWindow(xPos, 50, 96, 39); // Updates the number portion
  if (credits > 0) {
    if (slotRunning) {
      number1 = random(10);
      number2 = random(10);
      number3 = random(10);
    }
    display.setCursor(15, 10);
    display.print("$lot Machine");
    display.setCursor(15, 20);
    display.print("Credits: ");
    display.print(credits);
    display.setCursor(xPos, 50);
    display.print(number1);
    display.setCursor(xPos + 20, 50);
    display.print(number2);
    display.setCursor(xPos + 40, 50);
    display.print(number3);
    int winXPos = (96 - display.getPrintWidth("WIN")) / 2;
    if (!slotRunning && number1 == number2 && number2 == number3) {
      display.setCursor(winXPos, 35); 
      display.print("WIN");
      win = true;
    }

    if (display.getButtons(TSButtonLowerRight)) {
      slotRunning = false;
    }

    if (!slotRunning && display.getButtons(TSButtonLowerLeft)) {
      if (win) {
        credits+=10;
        win=false; // reset flag
      } else {
        credits--;
      }
      slotRunning = true;
      display.clearWindow(winXPos, 20, 96, 39);
    }

    delay(100); // Adjust the delay as needed
  } else {
    int textWidth = display.getPrintWidth("No more CREDITS");
    int textXPos = (96 - textWidth) / 2;
    display.clearScreen();
    display.setCursor(textXPos, 20);
    display.print("No more CREDITS");
    int textWidth2 = display.getPrintWidth("real COPIUM");
    int textXPos2 = (96 - textWidth2) / 2;
    display.setCursor(textXPos2, 40); // Adjust the Y position for the second line
    display.print("real COPIUM");
    delay(1000); // due to the nature of loops, 1s delay is required...
  }
}
