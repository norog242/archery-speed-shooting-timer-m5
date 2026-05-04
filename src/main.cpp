// M5 Atom S3 Step Counter/Timer Conversion from micro:bit
#include <M5AtomS3.h>
#include <math.h>
#include <Arduino.h>

int steps = 0;
unsigned long startTime = 0;
float duration = 0;

void reset() {
  steps = 0;
  startTime = 0;
  duration = 0;
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.clear();
  // Show 'Arrow' in size 3
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Arrow");
  // Show value in size 6 below
  M5.Lcd.setTextSize(6);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("%d", steps);
  M5.Lcd.setTextSize(2); // Restore default size
  delay(500);//wait half a second to avoid false detection of the first step when resetting.
}

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(2);
  reset();
}

void loop() {
  M5.update();

  // Button A pressed (AtomS3 has only one button)
  if (M5.BtnA.wasPressed()) {
    reset();
  }

  float accX, accY, accZ;
  M5.Imu.getAccelData(&accX, &accY, &accZ);
  float strength = sqrt(accX * accX + accY * accY + accZ * accZ) * 1000; // Convert to milli-g

  if (steps < 10) {
    if (strength > 1200) {
      steps++;
      if (steps == 1) {
        startTime = millis();
      }
      if (steps == 10) {
        duration = (millis() - startTime) / 1000.0;
        while (!M5.BtnA.wasPressed()) {
          M5.update();
          M5.Lcd.clear();
          // Show 'Time' in size 4
          M5.Lcd.setTextSize(3);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("Time");
          // Show value in size 6 below
          M5.Lcd.setTextSize(6);
          M5.Lcd.setCursor(0, 40);
          M5.Lcd.printf("%.2f", duration);
          delay(100);
        }
        M5.Lcd.setTextSize(2); // Restore default size
      } else {
        M5.Lcd.clear();
        // Show 'Arrow' in size 4
        M5.Lcd.setTextSize(4);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.printf("Arrow");
        // Show value in size 6 below
        M5.Lcd.setTextSize(6);
        M5.Lcd.setCursor(0, 40);
        M5.Lcd.printf("%d", steps);
        M5.Lcd.setTextSize(2); // Restore default size
      }
      delay(200); // Debounce for step detection
    }
  }
  delay(20); // Main loop delay
}