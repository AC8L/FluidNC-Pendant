// Button event handling code for FluidNC Pendant
//

void yellow_button_handleEvents(uint8_t eventType)
{
    switch (eventType)
    {
        case AceButton::kEventPressed:
            DEBUG_SERIAL.println("[YellowButtonHandler] pressed.");
            activeTimer(true);
        break;

        case AceButton::kEventLongPressed:
            DEBUG_SERIAL.println("[YellowButtonHandler] Long pressed.");
            activeTimer(true);
            if (myCNC.mState() == Door && pState == Pendant)
            { //Only reset from pendant state and not from menu
                myCNC.Reset(); //Reset
                getGrblState(false);
            } else 
            {
                myCNC.Reset();  //Reset first
                myCNC.Unlock(); //If there is an alarm, the controller will no longer accept commands, solving that with sending "Unlock"
                myCNC.SafetyDoor(); // Raise Safety Door Alarm
                pState = Pendant;
                getGrblState(false);
            }
            break;
    }
}

void red_button_handleEvents(uint8_t eventType)
{
  char sbuf[20];
  //DEBUG_SERIAL.println("[RedButtonHandler] Event raised.");
  switch (eventType) {  
    case AceButton::kEventReleased:
      DEBUG_SERIAL.println("[RedButtonHandler] released.");
      if (rState != RotSpindlespeed) {
        jState = jsXYaxis;
        tftUpdate(true);
      }
      break;
    case AceButton::kEventLongPressed:
      DEBUG_SERIAL.println("[RedButtonHandler] Long pressed.");
      //if (pState==Menu) 
      powerOffNow=true; 
      break;
    case AceButton::kEventPressed:
      DEBUG_SERIAL.println("[RedButtonHandler] pressed.");
      if (rState != RotSpindlespeed) {
        activeTimer(true);
        jState = jsZaxis;
        tftUpdate(true);
      }

      if (pState != Pendant) {
        pState = Pendant;
        tftUpdate(true);
      }
      else {
        switch (myCNC.mState()) 
        {
          case Alarm:
            DEBUG_SERIAL.println("[RedButtonHandler] Unlocking FluidNC...");
            myCNC.Unlock();
            break;
          case Run:
            myCNC.FeedHold();
            break;
          case Hold:
            myCNC.CycleStartResume();
            break;
        }
      }

      if (pState == Pendant && rState == RotSpindlespeed) {
        sprintf(sbuf, "M%d S%d", myCNC.isSpindleOn() ? 5 : 3, setSpindleSpeed);
        DEBUG_SERIAL.printf("[red_button_handleEvents] Set Spindle command=%s\n", sbuf);
        myCNC.fluidCMD(sbuf);
        getGrblState(true);
      }
      break;
    default:
      DEBUG_SERIAL.println("[RedButtonHandler] Other button event.");
  }
}

void RemsGreenSingleClickButtonHandler(AceButton *button, uint8_t eventType)
{
      char sbuf[20];
      uint8_t mybtn_PIN = button->getPin();
      //Serial.println((String)pState + " " + (String)menuChoice);
      activeTimer(true);
      if (pState == Pendant) {
        pState = Menu;
        menuChoice = quit;
      } else if (pState == Menu) {
        UI_Menu();
        //        tft.fillScreen(TFT_BLACK);
      } else if (pState == MenuConfirm) {
        if (menuChoice == setwxy0) {
          myCNC.fluidCMD((const char *)"G10 P1 L20 X0 Y0");
        } else if (menuChoice == setwz0) {
          myCNC.fluidCMD((const char *)"G10 P1 L20 Z0");
        }
        getGrblState(false);
        pState = Pendant;
      } else if (pState == MenuSpindleSpeed) {
        sprintf(sbuf, "S %d", setSpindleSpeed);
        DEBUG_SERIAL.printf("[RemsGreenSingleClickButtonHandler] Set Spindle speed command=%s\n", sbuf);
        myCNC.fluidCMD(sbuf);
        pState = Pendant;
      } else if (pState == MenuBrightness) {
        //DEBUG_SERIAL.printf("[RemsGreenSingleClickButtonHandler] Writing to EEPROM TFT brightess value=%d.\n", backLight);
        EEPROM.write(0, backLight);
        EEPROM.commit();
        pState = Pendant;
      }
      tftUpdate(true);

    if (mybtn_PIN==PIN_GREEN && pState == RotateChoice) 
    {
      pState = Pendant;
      tftUpdate(true);
    }
}

void GreenButtonLongClickHandler()
{
    if (pState == Pendant) {
      pState = RotateChoice;
      rState = RotJogspeed;
      tftUpdate(true);
    }
}

void button_handleEvents(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  switch (button->getPin()) {  
    case PIN_YELLOW:
        yellow_button_handleEvents(eventType);
        break;
    case PIN_RED:
        red_button_handleEvents(eventType);
        break;
    case PIN_GREEN:
        if (eventType==AceButton::kEventLongPressed)
        {
          //DEBUG_SERIAL.println("[GreenButtonHandler] Long Pressed.");
          //RemsGreenSingleClickButtonHandler(button, eventType);
        }
        if (eventType==AceButton::kEventPressed)
        {
          DEBUG_SERIAL.println("[GreenButtonHandler] Pressed.");
          GreenButtonLongClickHandler();
        }
        if (eventType==AceButton::kEventReleased  && pState == RotateChoice)
        {
          DEBUG_SERIAL.println("[GreenButtonHandler] Released.");
          pState = Pendant;
          tftUpdate(true);
        }
        break;
    case PIN_REMS:
        if (eventType==AceButton::kEventPressed)
        {
          DEBUG_SERIAL.println("[OpEncoderClickButtonHandler] Pressed.");
          RemsGreenSingleClickButtonHandler(button, eventType);
        }
        break;
  }
}
