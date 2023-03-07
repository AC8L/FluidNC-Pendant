// Miscellaneous auxilliary functions for FluidNC Pendant
//
void otaLoop() {
  char sbuf[20];
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  tft.fillScreen(TFT_BLACK);
  tftPrintSimple( 0, 1 * LINEHEIGHT_DEF, "WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  DEBUG_SERIAL.println(WiFi.localIP());
  tftPrintSimple( 0, 2 * LINEHEIGHT_DEF, "OTA");
  ArduinoOTA.setHostname("ESP32-pendant");
  ArduinoOTA.begin();
  while (true) {
    ArduinoOTA.handle();
    delay(1000);
    checkBattery();
    sprintf(sbuf, "Batt: %d", (int) battVolt);
    tftPrintSimple( 100, 4 * LINEHEIGHT_DEF, sbuf);
  }
}

void checkConnectWS()
{
  if ( !myCNC.isConnected() )
  {
    DEBUG_SERIAL.println("[MAIN] FluidNC WebSocket: Not connected.");
    tftShutdown("No WebSocket connection");
  }
}

void checkConnect()
{
  #ifdef MODE_WIFI
    checkConnectWS();
  #endif
}

void checkBattery() {
  float b = (analogRead(PIN_BATT) - 180) / 1.9;
  if (b > 100) b = 100;
  if (b > battVolt + 1) battVolt += (b - battVolt) / 2;
  if (b < battVolt - 1) battVolt -= (battVolt - b) / 2;
}

void getGrblState(bool full)
{
    #ifdef MODE_BT
        getGrblStateBT(full);
    #endif
    #ifdef MODE_WIFI
        myCNC.getGrblState(full);
    #endif
}
