#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// Call this function when you want to show a loading screen effect.
void displayLoadingScreen(LiquidCrystal_I2C &lcd)
{
  // Clear the display and show a header message.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   LOADING...   "); // Centered header message (adjust spaces if needed)

  // Animate a simple progress bar on the second row.
  for (int progress = 0; progress <= 16; progress++)
  {
    lcd.setCursor(0, 1);
    // Print filled blocks for progress.
    for (int i = 0; i < progress; i++)
    {
      lcd.print("#");
    }
    // Print empty spaces for remaining positions.
    for (int i = progress; i < 16; i++)
    {
      lcd.print(" ");
    }
    delay(150); // Adjust delay for desired speed.
  }

  // Hold the full progress bar for a moment.
  delay(500);
}

#endif
