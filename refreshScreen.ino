void refreshScreen()
{
  // Always print the current surface temp and power level on the first line
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp= ");
  lcd.print(temp, 1);
  lcd.print("F ");
  lcd.print(y, 0);

  if (isnan(temp))
  {
    lcd.setCursor(0, 0);
    lcd.print("T/C Problem");
  }

  lcd.setCursor(0, 1); //Set the cursor to the second line

  if (gainSet)
  {
    switch (mode)
    {
      case 1:
        lcd.print("P = ");
        lcd.print(Kp, 3);
        //Editing P gain
        break;
      case 2:
        lcd.print("I = ");
        lcd.print(Ki, 5);
        //Editing I gain
        break;
      case 3:
        lcd.print("D = ");
        lcd.print(Kd, 3);
        //Editing D gain
        break;
      case 4:
        lcd.print("T. cor. =");
        lcd.print(Tadjust, 6);
        //Editing Temp Correction factor
        break;
      case 5:
        lcd.print("Preset1=");
        lcd.print(preset1, 0);
        //Editing preset temp 1
        break;
      case 6:
        lcd.print("Preset2=");
        lcd.print(preset2, 0);
        //Editing preset temp 2
        break;
      case 7:
        lcd.print("Preset3=");
        lcd.print(preset3, 0);
        //Editing preset temp 3
        break;
    }
  }
  else
  {
    lcd.print("Target ");
    lcd.print(targetTemp, 0);
    lcd.print(" F");
  }
}

