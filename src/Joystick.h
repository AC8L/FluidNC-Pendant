// Joystck code for FluidNC Pendant
//
void forceEndJog() {
  DEBUG_SERIAL.println("[forceEndJog] 'Cancel'");
  myCNC.JogCancel();
  delay(100);
  getGrblState(false);
  while (myCNC.mState() == Jog) {
    DEBUG_SERIAL.println("[forceEndJog] 'D'");
    myCNC.JogCancel();
    delay(100);
    getGrblState(true);
  }
}

void waitEndJog() {
  getGrblState(false);
  while (myCNC.mState() == Jog) {
    delay(100);
    getGrblState(true);
    DEBUG_SERIAL.printf("[waitEndJog] Jog Wait. mState=%d\n", myCNC.mState());
  }
}

const int rdRawMin = -30;
const int rdRawMax = 30;
const int hysteresis = 10;

int readJSX() {
  int rdRaw = 0;
  for (int n = 10; n; --n) {
    rdRaw += analogRead(PIN_JSX) - calibrateX;
  }
  rdRaw /= 10;
  // soft hysteresis
  if (rdRaw < rdRawMin + hysteresis && rdRaw > rdRawMin && rdX == -1) rdRaw = rdRawMin;
  else if (rdRaw > rdRawMax - hysteresis && rdRaw < rdRawMax && rdX == 1) rdRaw = rdRawMax;
  return (rdRaw);
}

int readJSY() {
  int rdRaw = 0;
  for (int n = 10; n; --n) {
    rdRaw += analogRead(PIN_JSY) - calibrateY;
  }
  rdRaw /= 10;
  // soft hysteresis
  if (rdRaw < rdRawMin + hysteresis && rdRaw > rdRawMin && rdY == 1) rdRaw = rdRawMin;
  else if (rdRaw > rdRawMax - hysteresis && rdRaw < rdRawMax && rdY == -1) rdRaw = rdRawMax;
  return (rdRaw);
}

int checkJoystick() {
  char jogStr[MAX_JOGSTR];
  static int rdXprev = 0, rdYprev = 0, rdZprev = 0;
  static float sX, sY, sZ, speedXYZ;
  /*
    if (readJSY() <= rdRawMin || readJSY() >= rdRawMax ||
        readJSX() <= rdRawMin || readJSX() >= rdRawMax)
      checkConnectBt();
  */
  if (myCNC.mState() != Idle || pState != Pendant || rState == RotSpindlespeed) return 0;

  while (true) {
    //sinceStart = millis() + tExec;
    checkRotateA();
    rdX = 0;
    rdY = 0;
    rdZ = 0;
    if (jState == jsZaxis) {
      if (readJSY() <= rdRawMin) rdZ = 1;
      else if (readJSY() >= rdRawMax) rdZ = -1;
    } else {
      if (readJSX() <= rdRawMin) rdX = -1;
      else if (readJSX() >= rdRawMax) rdX = 1;
      if (readJSY() <= rdRawMin) rdY = 1;
      else if (readJSY() >= rdRawMax) rdY = -1;
    }
    if (rdX == 0 && rdY == 0 && rdZ == 0) {
      if (myCNC.mState() == Jog) {
        forceEndJog();
        lastJogCmdXYZ = 0;
      }
      return 0;
    }
    activeTimer(true);
    if (rdX != rdXprev || rdY != rdYprev || rdZ != rdZprev) {
      rdXprev = rdX;
      rdYprev = rdY;
      rdZprev = rdZ;
      myCNC.JogCancel();
      lastJogCmdXYZ = 0;
    }
//    if (sinceStart - lastGrblState > updateInterval) {
      //      Serial.println("A");
//      getGrblState(true);
//    }
    multXYZ = jogSpeed > jogSpeedMax ? jogSpeedMax : jogSpeed;
    if (sinceStart - lastJogCmdXYZ > tExec) {
      sX = abs(rdX * rdX) * multXYZ;
      sY = abs(rdY * rdY) * multXYZ;
      sZ = abs(rdZ * rdZ) * multXYZ;
      //      if (sX>=sY) sY=0; else sX=0; //alleen via X of via Y bewegen
      if (sZ) speedXYZ = sZ;
      else speedXYZ = sqrt(sX * sX + sY * sY);
      distX = ((jogSpeedMax - sX) / 200000000 + 0.000018) * sX * (rdX ? (rdX < 0 ? -1 : 1) : 0) * tExec;
      distY = ((jogSpeedMax - sY) / 200000000 + 0.000018) * sY * (rdY ? (rdY < 0 ? -1 : 1) : 0) * tExec;
      distZ = ((jogSpeedMax - sZ) / 200000000 + 0.000018) * sZ * (rdZ ? (rdZ < 0 ? -1 : 1) : 0) * tExec;
      snprintf(jogStr, MAX_JOGSTR, "$J=G21G91X%fY%fZ%fF%d", distX, distY, distZ, (int)speedXYZ);
      lastJogCmdXYZ = sinceStart;
      myCNC.set_mState(Jog); //force
      myCNC.fluidCMD(jogStr);
      DEBUG_SERIAL.printf("[checkJoystick] jogStr=%s\n", jogStr);
      delay(100);
      getGrblState(true);
    }
  }
}
