// Timer and Sleep/Wakeup functions for FluidNC Pendant
//
void goto_sleep()
{
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  pinMode(PIN_JS_VCC, INPUT);
  esp_sleep_enable_ext0_wakeup(PIN_RED, 0);
  delay(200);
  esp_deep_sleep_start();
}

void activeTimer(bool reset) {
  static unsigned long aTimer = 0;
  if (reset) {
    aTimer = sinceStart;
    powerOffNow = false;
  }
  if ((sinceStart - aTimer > (POWEROFF_TIME - 10) * 1000 && battVolt < 65) || powerOffNow) {
    tft.fillScreen(TFT_BLACK);
    tftPrintSimple( 30, 4 * LINEHEIGHT_DEF, "Poweroff");
    delay(1000);
    tftUpdate(true);
    delay(1000);
    if ((sinceStart - aTimer > (POWEROFF_TIME) * 1000 && battVolt < 65) || powerOffNow) {
      tft.fillScreen(TFT_BLACK);
      digitalWrite(TFT_BL, LOW);
      tft.writecommand(TFT_DISPOFF);
      tft.writecommand(TFT_SLPIN);
      goto_sleep();
    }
  }
}
