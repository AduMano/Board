#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configure the I2C LCD (adjust address and dimensions as needed).
static LiquidCrystal_I2C lcd(0x27, 16, 2);

void initLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Parcel Locker");
}

// Helper function to center text in a fixed width (16 characters)
String centerText(const String &text, int width = 16)
{
  int len = text.length();
  if (len >= width)
  {
    // If text is longer than width, return the leftmost width characters
    return text.substring(0, width);
  }

  // Calculate the left padding needed to center the text
  int padding = (width - len) / 2;
  String result = "";

  // Prepend spaces for left padding
  for (int i = 0; i < padding; i++)
  {
    result += " ";
  }

  // Add the text
  result += text;

  // Append spaces to fill the remaining width
  while (result.length() < width)
  {
    result += " ";
  }

  return result;
}

void displayMessage(const String &line1, const String &line2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
void displayMessageAutoCenter(const String &line1, const String &line2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(centerText(line1, 16));
  lcd.setCursor(0, 1);
  lcd.print(centerText(line2, 16));
}

#endif
