// Code for Optical Rotator for FluidNC Pendant
//

IRAM_ATTR void checkPositionA() {
  encoderA->tick();  // just call tick() to check the state.
}

void checkRotateA() {
  char jogStr[MAX_JOGSTR];
  //char sbuf[64];
  static int posPrev = 0, pos, dir;
  //  encoderA->tick(); // just call tick() to check the state.
  pos = encoderA->getPosition();
  if (posPrev == pos) return;
  activeTimer(true);
  posPrev = pos;
  dir = (int)encoderA->getDirection();
  Serial.print("[checkRotateA] pState=");
  Serial.println(pState);
  if (pState == RotateChoice) {
    tftUpdate(true);
    if (dir == 1) {
      if (rState == RotOvrapid) rState = RotOvspeed;
      else if (rState == RotOvspeed) rState = RotOvfeed;
      else if (rState == RotOvfeed) rState = RotJogspeed;
      else if (rState == RotJogspeed) rState = RotSpindlespeed;
      else if (rState == RotSpindlespeed) rState = RotZaxis;
      else if (rState == RotZaxis) rState = RotYaxis;
      else rState = RotXaxis;
    } else {
      if (rState == RotXaxis) rState = RotYaxis;
      else if (rState == RotYaxis) rState = RotZaxis;
      else if (rState == RotZaxis) rState = RotSpindlespeed;
      else if (rState == RotSpindlespeed) rState = RotJogspeed;
      else if (rState == RotJogspeed) rState = RotOvfeed;
      else if (rState == RotOvfeed) rState = RotOvspeed;
      else rState = RotOvrapid;
    }
    tftUpdate(true);
  } else if (pState == Pendant && digitalRead(PIN_GREEN)==HIGH) {
    if (rState < 3 || pState == RotateChoice) { //een van de assen
      static unsigned long lastStepCmd = 0;
      float dist = 0;
      if (dir == 1) dist = 0.001 * (jogSpeed > 5000 ? 5000 : jogSpeed);
      else dist = -0.001 * (jogSpeed > 5000 ? 5000 : jogSpeed);
      switch (rState) {
        case RotZaxis:
          snprintf(jogStr, MAX_JOGSTR, "$J=G21G91X0Y0Z%fF%d", dist, 1000);
          break;
        case RotYaxis:
          snprintf(jogStr, MAX_JOGSTR, "$J=G21G91X0Y%fZ0F%d", dist, 1000);
          break;
        case RotXaxis:
          snprintf(jogStr, MAX_JOGSTR, "$J=G21G91X%fY0Z0F%d", dist, 1000);
          break;
        default:
          break;
      }
      if (pState == RotateChoice) snprintf(jogStr, MAX_JOGSTR, "$J=G21G91X0Y0Z%fF%d", dist, 1000);
      lastStepCmd = sinceStart;
      myCNC.fluidCMD(jogStr);
      DEBUG_SERIAL.printf("[checkRotateA] jogStr=%s\n", jogStr);
      while (sinceStart - lastStepCmd < (20 + (jogSpeed > 5000 ? 5000 : jogSpeed) / 20)) {
        sinceStart = millis() + tExec;
        encoderA->tick();
      }
    }
    else if (rState == RotOvfeed) {
      myCNC.fluidCMD(dir == 1 ? 0x91 : 0x92);
    }
    else if (rState == RotOvspeed) {
      myCNC.fluidCMD(dir == 1 ? 0x9A : 0x9B);
    }
    else if (rState == RotOvrapid) {
      myCNC.fluidCMD(dir == 1 ? (myCNC.ovRapid() < 50 ? 0x96 : 0x95) : (myCNC.ovRapid() > 50 ? 0x96 : 0x97));
    }
    else if (rState == RotSpindlespeed) {
      if (dir == 1) {
        if (setSpindleSpeed < 3000) {
          setSpindleSpeed = 3000;
        }
        else {
          setSpindleSpeed += 1000;
          if (setSpindleSpeed > 24000) setSpindleSpeed = 24000;
        }
      } else {
        setSpindleSpeed -= 1000;
        if (setSpindleSpeed < 3000) {
          setSpindleSpeed = 3000;
        }
      }
      sprintf((char *)g_sbuf, "M%d S%d", myCNC.isSpindleOn() ? 3 : 5, setSpindleSpeed);
      DEBUG_SERIAL.printf("[checkRotateA] set spindle speed command: %s.\n", g_sbuf);
      myCNC.fluidCMD((const char *)g_sbuf);
      if (!myCNC.isSpindleOn()) {
        sprintf((char *)g_sbuf, "M5 S%d", setSpindleSpeed);
        DEBUG_SERIAL.printf("[checkRotateA] turn on spindle and set speed command: %s.\n", g_sbuf);
        myCNC.fluidCMD((const char *)g_sbuf);
      }
      getGrblState(true);
      myCNC.set_reportedSpindleSpeed(setSpindleSpeed); //tot we beter weten
      tftUpdate(false);
    } else { //rState is RotJogspeed
      forceEndJog();
      lastJogCmdXYZ = 0;
      if (dir == 1) {
        if (smallSteps) {
          if (jogSpeed <= 20) ++jogSpeed;
          else jogSpeed *= 1.05;
        } else {
          jogSpeed *= 2.5;
          if (jogSpeed >= 1000) {
            jogSpeed = (jogSpeed / 1000) * 1000;
          } else if (jogSpeed >= 100) {
            jogSpeed = (jogSpeed / 100) * 100;
          } else if (jogSpeed >= 10) {
            jogSpeed = (jogSpeed / 10) * 10;
          }
        }
      } else {
        if (smallSteps) {
          if (jogSpeed <= 20) --jogSpeed;
          else jogSpeed /= 1.05;
        } else if (jogSpeed > 5000) jogSpeed = 5000;
        else {
          jogSpeed /= 2;
          if (jogSpeed >= 1000) {
            jogSpeed = (jogSpeed / 1000) * 1000;
          } else if (jogSpeed >= 100) {
            jogSpeed = (jogSpeed / 100) * 100;
          } else if (jogSpeed >= 10) {
            jogSpeed = (jogSpeed / 10) * 10;
          }
        }
      }
      if (jogSpeed < 2) jogSpeed = 2;
      if (jogSpeed > jogSpeedMax) jogSpeed = jogSpeedMax;
      tftUpdate(false);
    }
  } else if (pState == Menu) {
    if (dir == 1) {
      if (menuChoice > 0) menuChoice--;
    } else if (menuChoice < maxMenu) menuChoice++;
    tftUpdate(false);
  } else if (pState == MenuSpindleSpeed) {
    if (dir == 1) {
      setSpindleSpeed += 1000;
      if (setSpindleSpeed > 24000) setSpindleSpeed = 24000;
    } else {
      setSpindleSpeed -= 1000;
      if (setSpindleSpeed < 3000) setSpindleSpeed = 3000;
    }
  } else if (pState == MenuBrightness) {
    if (dir == 1) {
      if (backLight < 16) {
        backLight++;
      }
    } else if (backLight > 1) {
      backLight--;
    }
    ledcWrite(0, backLight * backLight); // 0-15, 0-255 (with 8 bit resolution)
    tftUpdate(true);
  }
}
