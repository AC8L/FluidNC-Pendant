// Functions drawing/printing on TFT screen for FluyidNC Pendant
//
void tftShutdown(String msg)
{
    tftPrint(TFT_LEFT, TFT_OBLQ, TFT_RED, TFT_BLACK, 20, 2 * LINEHEIGHT_DEF, 0, msg);
    tftPrint(TFT_LEFT, TFT_OBLQ, TFT_RED, TFT_BLACK, 20, 3 * LINEHEIGHT_DEF, 0, "Shutting down");
    delay(2000);
    ESP.restart();
}

void printAxisCoords(float m, float w, byte row) {
  char sbuf[20];
  tft.fillRect(18, row * LINEHEIGHT_DEF + 2, 164, LINEHEIGHT_DEF, TFT_BLACK); //clear to eol
  if (m > 0) tftPrint(TFT_LEFT, TFT_TRANSP, TFT_LIGHTBLUE, TFT_BLACK, 18, row * LINEHEIGHT_DEF, 0, "+");
  else if (m < 0) tftPrint(TFT_LEFT, TFT_TRANSP, TFT_LIGHTBLUE, TFT_BLACK, 20, row * LINEHEIGHT_DEF, 0, "-");
  sprintf(sbuf, "%03d.%03d", (int)abs(m) % 1000, (int)(fabsf(m) * 1000) % 1000);
  tftPrint(TFT_LEFT, TFT_TRANSP, TFT_LIGHTBLUE, TFT_BLACK, 27, row * LINEHEIGHT_DEF + 2, 0, sbuf);
  if (w > 0) tftPrint(TFT_LEFT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 98, row * LINEHEIGHT_DEF, 0, "+");
  else if (w < 0) tftPrint(TFT_LEFT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 100, row * LINEHEIGHT_DEF, 0, "-");
  sprintf(sbuf, "%03d.%03d", (int)abs(w) % 1000, (int)(fabsf(w) * 1000) % 1000);
  tftPrint(TFT_LEFT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 108, row * LINEHEIGHT_DEF + 2, 0, sbuf);
}

void printCircleBlue (byte x, byte y) {
  tft.fillCircle(x, y, 8, pState == RotateChoice ? TFT_MEDIUMBLUE : TFT_DARKBLUE);
}

void tftPrintAllButtons() {
  if (pState != RotateChoice) {
    if (myCNC.mState() != Door) tftPrintButtonOrange("Abort");
    else if (pState == Pendant) tftPrintButtonOrange("Reset");
  }
  if (pState == Pendant) {
    tftPrintButtonGreen("Mnu/Rot");
    if ((myCNC.mState() == Idle || myCNC.mState() == Jog) && rState != RotSpindlespeed) {
      tftPrintButtonRed("XYZ");
    }
    if (myCNC.mState() == Alarm) tftPrintButtonRed("Unlock");
    else if (rState == RotSpindlespeed) tftPrintButtonRed("Spindle");
    else if (myCNC.mState() == Run) tftPrintButtonRed("Hold");
    else if (myCNC.mState() == Hold) tftPrintButtonRed("Resume");
  }
  else if (pState == MenuBrightness) {
    tftPrintButtonGreen("Save");
    tftPrintButtonRed("Exit");
  }
  else if (pState == Menu || pState == MenuConfirm) {
    tftPrintButtonGreen("OK");
    tftPrintButtonRed("Cancel");
  }
}

void tftPrintButtonOrange(String txt) {
  tftPrintButton(TFT_ORANGE, TFT_WHITE, 161, 112, 79, txt);
}

void tftPrintButtonGreen(String txt) {
  tftPrintButton(TFT_DARKGREEN, TFT_WHITE, 81, 112, 78, txt);
}

void tftPrintButtonRed(String txt) {
  tftPrintButton(TFT_RED, TFT_WHITE, 0, 112, 79, txt);
}

void tftPrintButton(int bc, int tc, byte col, byte row, byte len, String txt) {
  tft.fillRect(col, row, len, LINEHEIGHT_DEF + 3, TFT_BLACK);
  tft.fillRoundRect(col, row, len, LINEHEIGHT_DEF + 1, 3, bc);
  tft.fillRoundRect(col + 2, row + 2, len - 4, LINEHEIGHT_DEF - 3, 3, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(tc);
  tft.setFreeFont(&FreeSans9pt7b);
  //  tft.setFreeFont(&FreeSansBold9pt7b);
  //  tft.setFreeFont(&FreeSans12pt7b);
  tft.drawString(txt, col + len / 2, row + LINEHEIGHT_DEF / 2 - 2);
  tft.setTextDatum(TL_DATUM);
}

void tftUpdate(bool force) {
  char sbuf[20];
  static float mXdisp = 0, mYdisp = 0, mZdisp = 0;  //Getoonde machine positions
  static float wXdisp = 0, wYdisp = 0, wZdisp = 0;  //Getoonde work positions
  static byte menuChoiceDisp = -1, mStateDisp = -1, blDisp = 0;
  static int reportedSpindleSpeedDisp = 0, setSpindleSpeedDisp = 0, battVoltDisp = 0;
  static int ovSpeedDisp = 0, ovFeedDisp = 0, ovRapidDisp = 0;
  static bool cnc_ConnDisp = false, spindleOnDisp = false;
  static int pStateButton = -1, mStateButton = -1, rStateButton = -1, menuChoiceButton = -1;
  if (force) tft.fillScreen(TFT_BLACK);
  if (pState == Pendant || pState == RotateChoice) {
    if (mXdisp != myCNC.mX() || wXdisp != myCNC.wX() || mYdisp != myCNC.mY() || wYdisp != myCNC.wY() || mZdisp != myCNC.mZ() || wZdisp != myCNC.wZ()) activeTimer(true);
    if (mXdisp != myCNC.mX() || wXdisp != myCNC.wX() || force) {
      if (rState == RotXaxis) printCircleBlue(8, LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_LEFT, TFT_TRANSP, (jState == jsXYaxis && rState != RotSpindlespeed) ? TFT_LIGHTRED : TFT_WHITE, TFT_BLACK, 2, 0 * LINEHEIGHT_DEF + 2, 0, "X");
      printAxisCoords(myCNC.mX(), myCNC.mX() - myCNC.wX(), 0);
      mXdisp = myCNC.mX();
      wXdisp = myCNC.wX();
    }
    if (mYdisp != myCNC.mY() || wYdisp != myCNC.wY() || force) {
      if (rState == RotYaxis) printCircleBlue(8, LINEHEIGHT_DEF + LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_LEFT, TFT_TRANSP, (jState == jsXYaxis && rState != RotSpindlespeed) ? TFT_LIGHTRED : TFT_WHITE, TFT_BLACK, 2, 1 * LINEHEIGHT_DEF + 2, 0, "Y");
      printAxisCoords(myCNC.mY(), myCNC.mY() - myCNC.wY(), 1);
      mYdisp = myCNC.mY();
      wYdisp = myCNC.wY();
    }
    if (mZdisp != myCNC.mZ() || wZdisp != myCNC.wZ() || force) {
      if (rState == RotZaxis) printCircleBlue(8, LINEHEIGHT_DEF * 2 + LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_LEFT, TFT_TRANSP, (jState == jsZaxis && rState != RotSpindlespeed) ? TFT_LIGHTRED : TFT_WHITE, TFT_BLACK, 2, 2 * LINEHEIGHT_DEF + 2, 0, "Z");
      printAxisCoords(myCNC.mZ(), myCNC.mZ() - myCNC.wZ(), 2);
      mZdisp = myCNC.mZ();
      wZdisp = myCNC.wZ();
    }
    if (myCNC.reportedSpindleSpeed() != reportedSpindleSpeedDisp || setSpindleSpeedDisp != setSpindleSpeed || spindleOnDisp != myCNC.isSpindleOn() || force) {
      if (rState == RotSpindlespeed) printCircleBlue(8, LINEHEIGHT_DEF * 3 + LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_LEFT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 2, 3 * LINEHEIGHT_DEF + 2, 0, "S");
      sprintf(sbuf, "%03d", myCNC.isSpindleOn() ? (int)(myCNC.reportedSpindleSpeed()) : setSpindleSpeed);
      tftPrint(TFT_LEFT, TFT_OBLQ, myCNC.isSpindleOn() ? TFT_DARKORANGE : TFT_WHITE, TFT_BLACK, 27, 3 * LINEHEIGHT_DEF + 2, 55, sbuf);
      if (rState == RotSpindlespeed) {
        //        tftPrintButton(TFT_RED,TFT_WHITE, 80, 3 * LINEHEIGHT_DEF+2, 100, spindleOn?"Off":"On");
      }
      reportedSpindleSpeedDisp = myCNC.reportedSpindleSpeed();
      setSpindleSpeedDisp = setSpindleSpeed;
      spindleOnDisp = myCNC.isSpindleOn();
    }
    if (jogSpeedDisp != jogSpeed || force) {
      if (rState == RotJogspeed) printCircleBlue(232, 3 * LINEHEIGHT_DEF + LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_RIGHT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 228, 3 * LINEHEIGHT_DEF + 2, 10, "J");
      if (jogSpeed < 1000) snprintf(sbuf, 20, "%d/.%03d ", ((int)(jogSpeed > jogSpeedMax ? jogSpeedMax : jogSpeed)) % 10000, (int)(jogSpeed) % 1000);
      else snprintf(sbuf, 20, "%d/%d", ((int)(jogSpeed > jogSpeedMax ? jogSpeedMax : jogSpeed)) % 10000, (int)(jogSpeed > 5000 ? 5000 : jogSpeed) / 1000);
      sbuf[6] = 0;
      tftPrint(TFT_RIGHT, TFT_OBLQ, TFT_WHITE, TFT_BLACK, 218, 3 * LINEHEIGHT_DEF + 2, 60, sbuf);
      jogSpeedDisp = jogSpeed;
    }
    if (ovFeedDisp != myCNC.ovFeed() || force) {
      if (rState == RotOvfeed) printCircleBlue(231, 2 * LINEHEIGHT_DEF + LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_RIGHT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 228, 2 * LINEHEIGHT_DEF + 2, 10, "F");
      sprintf(sbuf, "%d", myCNC.ovFeed());
      tftPrint(TFT_RIGHT, TFT_OBLQ, TFT_OVERYELLOW, TFT_BLACK, 218, 2 * LINEHEIGHT_DEF + 2, 30, sbuf);
      ovFeedDisp = myCNC.ovFeed();
    }
    if (ovSpeedDisp != myCNC.ovSpeed() || force) {
      if (rState == RotOvspeed) printCircleBlue(231, LINEHEIGHT_DEF + LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_RIGHT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 227, LINEHEIGHT_DEF + 2, 10, "S");
      sprintf(sbuf, "%d", myCNC.ovSpeed());
      tftPrint(TFT_RIGHT, TFT_OBLQ, TFT_OVERYELLOW, TFT_BLACK, 218, 1 * LINEHEIGHT_DEF + 2, 30, sbuf);
      ovSpeedDisp = myCNC.ovSpeed();
    }
    if (ovRapidDisp != myCNC.ovRapid() || force) {
      if (rState == RotOvrapid) printCircleBlue(231, LINEHEIGHT_DEF / 2 - 1);
      tftPrint(TFT_RIGHT, TFT_TRANSP, TFT_WHITE, TFT_BLACK, 227, 2, 10, "R");
      sprintf(sbuf, "%d", myCNC.ovRapid());
      tftPrint(TFT_RIGHT, TFT_OBLQ, TFT_OVERYELLOW, TFT_BLACK, 218, 2, 30, sbuf);
      ovRapidDisp = myCNC.ovRapid();
    }
    if (cnc_ConnDisp != myCNC.isConnected() || force) {
      //      tftPrintLen(btConnected ? TFT_WHITE : TFT_RED, 190, 3 * LINEHEIGHT_DEF+2, 15, "B");
      cnc_ConnDisp = myCNC.isConnected();
    }
    if (battVoltDisp != battVolt || force) {
      sprintf(sbuf, "%d", (int) battVolt);
      tftPrint(TFT_RIGHT, TFT_OBLQ, TFT_WHITE, TFT_BLACK, 218, 4 * LINEHEIGHT_DEF + 2, 20, sbuf);
      battVoltDisp = battVolt;
    }
    if (mStateDisp != myCNC.mState() || force) {
      sprintf(sbuf, "%s", mStateStr[myCNC.mState()]);
      //      tftClearAt(185,4,8);
      if (myCNC.mState() == Alarm || myCNC.mState() == Door || myCNC.mState() == Unknwn)
        tftPrintBig(TFT_DARKRED, TFT_BLACK, 120, 4 * LINEHEIGHT_DEF + 13, 70, sbuf);
      else if (myCNC.mState() == Hold)
        tftPrintBig(TFT_ORANGE, TFT_BLACK, 120, 4 * LINEHEIGHT_DEF + 13, 70, sbuf);
       else if (myCNC.mState() == Run || myCNC.mState() == Jog)
        tftPrintBig(TFT_BLACK, TFT_ORANGE, 120, 4 * LINEHEIGHT_DEF + 13, 70, sbuf);
      else tftPrintBig(TFT_BLACK, TFT_MEDIUMGREEN, 120, 4 * LINEHEIGHT_DEF + 13, 70, sbuf);
      mStateDisp = myCNC.mState();
    }
  } else if (pState == Menu) {
    if (menuChoiceDisp != menuChoice || force) {
      menuChoiceDisp = menuChoice;
      tft.fillScreen(TFT_BLACK);
      static byte first = quit - 1;
      first = menuChoice > first + 4 ? menuChoice - 4 : first;
      first = menuChoice < first ? menuChoice : first;
      for (byte n = first; n < (first + 5); n++) {
        tftPrint(TFT_LEFT, TFT_OBLQ, TFT_GREEN, TFT_BLACK, 0, (menuChoice - first)*LINEHEIGHT_DEF, 0, ">");
        switch (n) {
          case poweroff:
            tftPrintColor(TFT_RED, 30, (n - first)*LINEHEIGHT_DEF, "Power off");
            break;
          case brightness:
            tftPrintSimple(30, (n - first)*LINEHEIGHT_DEF, "Brightness");
            break;
          case spindleonoff:
            sprintf(sbuf, "Spindle %s", myCNC.isSpindleOn() ? "off" : "on");
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, sbuf);
            break;
          case spindlespeed:
            sprintf(sbuf, "Spindle %d", setSpindleSpeed);
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, sbuf);
            break;
          case unlock:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Unlock");
            break;
          case home:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Home");
            break;
          case resetgrbl:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Reset Grbl");
            break;
          case setwxy0:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Set Wxy=0");
            break;
          case setwz0:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Set Wz=0");
            break;
          case probe:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Probe Z");
            break;
          case gawxy0:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Go to Wxy=0");
            break;
          case steps:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Steps ");
            tftPrintSimple( 150, (n - first)*LINEHEIGHT_DEF, smallSteps ? "++" : "--");
            break;
          case quit:
          default:
            tftPrintSimple( 30, (n - first)*LINEHEIGHT_DEF, "Exit");
            break;
        }
      }
    }
  } else if (pState == MenuSpindleSpeed) {
    if (spindleDisp != setSpindleSpeed || force) {
      tft.fillScreen(TFT_BLACK);
      tftPrintSimple( 30, 2 * LINEHEIGHT_DEF, "rpm:");
      sprintf(sbuf, "%d", setSpindleSpeed);
      tftPrintSimple( 30, 3 * LINEHEIGHT_DEF, sbuf);
      spindleDisp = setSpindleSpeed;
    }
  } else if (pState == MenuConfirm) {
    if (force) {
      tftPrintSimple( 80, 3 * LINEHEIGHT_DEF, "Confirm..");
    }
  } else if (pState == MenuBrightness) {
    if (blDisp != backLight || force) {
      tft.fillScreen(TFT_BLACK);
      tftPrintSimple( 60, 2 * LINEHEIGHT_DEF, "Brightness:");
      sprintf(sbuf, "%d", backLight);
      tftPrintSimple( 100, 4 * LINEHEIGHT_DEF, sbuf);
      blDisp = backLight;
    }
  }
  if (pStateButton != pState || mStateButton != myCNC.mState() || rStateButton != rState || menuChoiceButton != menuChoice || force)
    tftPrintAllButtons();
  pStateButton = pState;
  mStateButton = myCNC.mState();
  rStateButton = rState;
  menuChoiceButton = menuChoice;
}

void tftPrint(int align, bool transp, int fg, int bg, byte col, byte row, byte len, String txt) {
  tft.setFreeFont(&FreeSans9pt7b);
  if (transp) tft.setTextColor(fg);
  else {
    tft.setTextColor(fg, bg);
    if (align == TFT_RIGHT) {
      if (len) tft.fillRect(col - len, row, len, LINEHEIGHT_DEF, TFT_BLACK);
      tft.setTextDatum(TR_DATUM);
    }
    else if (align == TFT_CENTER) {
      if (len) tft.fillRect(col - len / 2, row, len, LINEHEIGHT_DEF, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
    }
    else {
      if (len) tft.fillRect(col, row, len, LINEHEIGHT_DEF, TFT_BLACK);
      tft.setTextDatum(TL_DATUM);
    }
  }
  tft.drawString(txt, col, row);
  tft.setTextDatum(TL_DATUM);
}

void tftPrintBig(int bg, int fg, byte col, byte row, byte len, String txt) {
  tft.setTextDatum(MC_DATUM);
  tft.fillRect(col - len / 2, row - LINEHEIGHT_DEF - 10, len, 2 * LINEHEIGHT_DEF - 10 , bg);
  tft.setTextColor(fg);
  //&FreeMonoBold9pt7b
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.drawString(txt, col, row - 17);
  tft.setTextDatum(TL_DATUM);
}
void tftPrintColor(int color, byte col, byte row, String txt) {
  tftPrint(TFT_LEFT, TFT_OBLQ, color, TFT_BLACK, col, row, 0, txt);  
}

void tftPrintSimple(byte col, byte row, String txt) {
  tftPrint(TFT_LEFT, TFT_OBLQ, TFT_WHITE, TFT_BLACK, col, row, 0, txt);
}
