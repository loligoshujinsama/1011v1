#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>

TinyScreen display = TinyScreen(TinyScreenDefault);
unsigned long startTime;
unsigned long timerDuration = 25 * 60 * 1000; // 25 minutes in milliseconds

void initializePomodoroTimer() {
  Wire.begin();
  display.begin();
  display.setBrightness(10);
  startTime = millis(); // Record the start time
}

void updatePomodoroTimer() {
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
  display.setCursor(48 - (width / 2), 10);
  display.fontColor(TS_8b_Green, TS_8b_Black);
  display.print("Pomodoro Timer");

  char timeString[10];
  sprintf(timeString, "%02lu:%02lu", minutes, seconds);

  display.clearWindow(0, 25, 96, 39);
  display.setFont(liberationSans_22ptFontInfo);
  display.setCursor(10, 25);
  display.fontColor(TS_8b_Blue, TS_8b_Black);
  display.print(timeString);
}

void setup(void) {
  initializePomodoroTimer();
}

void loop() {
  updatePomodoroTimer();
}
