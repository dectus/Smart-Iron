void readEncoderButton()
{
  if (!digitalRead(encoderButton))
  {

    mode++;
    if (gainSet)
    {
      if (mode > 7)
      {
        mode = 1;
      }
    }
    else if (mode > 4)
    {
      mode = 1;
    }
    while (!digitalRead(encoderButton))
    {
      delay(100);
      count++;
      if (count >= 30)
      {
        gainSet = !gainSet;
        if (!gainSet)
        {
          lcd.clear();
          lcd.print("Settings Saved");
          delay(1000);
          EEPROM.put(64, Kp);
          EEPROM.put(128, Ki);
          EEPROM.put(192, Kd);
          EEPROM.put(255, Tadjust);
          EEPROM.put(320, preset1);
          EEPROM.put(384, preset2);
          EEPROM.put(448, preset3);
        }
        count = 0;
        refreshScreen();
      }
    }
    count = 0;
    I = 0;
  }
}


void updateEncoder()
{
  int MSB = digitalRead(encoderPin2); //MSB, most significant bit
  int LSB = digitalRead(encoderPin1); //LSB = least significant bit

  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
  {
    encoderValue --;

    switch (gainSet)
    {
      case true:
        switch (mode)
        {
          case 1:
            Kp -= 0.0025;
            break;
          case 2:
            Ki -= 0.0000025;
            break;
          case 3:
            Kd -= 0.00025;
            break;
          case 4:
            Tadjust -= 0.000025;
            break;
          case 5:
            preset1 -= 0.25;
            break;
          case 6:
            preset2 -= 0.25;
            break;
          case 7:
            preset3 -= 0.25;
            break;
        }
        break;
      case false:
        if (mode == 1)
        {
          targetTemp -= 0.25;
          I = 0;
        }
        break;
    }
  }

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue ++;

    switch (gainSet)
    {
      case true:
        switch (mode)
        {
          case 1:
            Kp += 0.0025;
            break;
          case 2:
            Ki += 0.0000025;
            break;
          case 3:
            Kd += 0.00025;
            break;
          case 4:
            Tadjust += 0.000025;
            break;
          case 5:
            preset1 += 0.25;
            break;
          case 6:
            preset2 += 0.25;
            break;
          case 7:
            preset3 += 0.25;
            break;
        }
        break;
      case false: 
        targetTemp += 0.25;
        I = 0;
        break;
    }
  }

  lastEncoded = encoded; //store this value for next time

}

