// Setup routines for FluidNC Pendant
//
// SETUP =================================
void setup() {
  //  setCpuFrequencyMhz(240);
  Serial.begin(115200);

  #ifdef DEBUG
    Serial.setDebugOutput(true);
  #else
    Serial.setDebugOutput(false);
  #endif

  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println();
  DEBUG_SERIAL.println();

  EEPROM.begin(EEPROM_SIZE);
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);
  analogReadResolution(9);  //minimum is 9 bits dus waarden 0 en 512;
  backLight = EEPROM.read(0) % 16;
  //DEBUG_SERIAL.printf("[SETUP] Read from EEPROM TFT brightess value=%d.\n", backLight);
  if (backLight < 1) backLight = 1;
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  //zet de helderheid van het backlight
  ledcSetup(0, 5000, 8); // 0-15, 5000, 8
  ledcAttachPin(TFT_BL, 0); // TFT_BL, 0 - 15
  ledcWrite(0, 64); // 0-15, 0-255 (with 8 bit resolution)
  tftPrintSimple(20, 2 * LINEHEIGHT_DEF, "(OTA)");
  tftPrintSimple( 20, 3 * LINEHEIGHT_DEF, "Connecting...");
  //if (digitalRead(PIN_RED) == LOW) otaLoop();
  ledcWrite(0, backLight * backLight); // 0-15, 0-255 (with 8 bit resolution)
  pinMode(PIN_JSX, INPUT);
  pinMode(PIN_JSY, INPUT);
  pinMode(PIN_BATT, INPUT);
  //pinMode(PIN_REMS, INPUT_PULLUP);
  //  pinMode(PIN_JSS,INPUT_PULLUP);
  pinMode(PIN_JS_VCC, OUTPUT);
  digitalWrite(PIN_JS_VCC, HIGH);
  tftUpdate(true);
  
  //  xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,1,&Task1,0);
  //  vTaskDelay(500);
  
  encoderA = new RotaryEncoder(PIN_REM1, PIN_REM2, RotaryEncoder::LatchMode::FOUR3);
  // register interrupt routine
  attachInterrupt(digitalPinToInterrupt(PIN_REM1), checkPositionA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_REM2), checkPositionA, CHANGE);
  activeTimer(true);
  // Calibrate Joystick
  for (int n = 100; n; --n) {
    calibrateX += analogRead(PIN_JSX);
    calibrateY += analogRead(PIN_JSY);
    delay(1);
  }
  calibrateX /= 100;
  calibrateY /= 100;

  pinMode(PIN_REMS, INPUT_PULLUP);
  pinMode(PIN_GREEN, INPUT_PULLUP);
  pinMode(PIN_YELLOW, INPUT_PULLUP);
  pinMode(PIN_RED, INPUT_PULLUP);
  
  buttonREMS.init(PIN_REMS);
  buttonGREEN.init(PIN_GREEN);
  buttonYELLOW.init(PIN_YELLOW);
  buttonRED.init(PIN_RED);

  ButtonConfig* buttonREDconfig = buttonRED.getButtonConfig();
  buttonREDconfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonREDconfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  ButtonConfig* buttonYELLOWconfig = buttonYELLOW.getButtonConfig();
  buttonYELLOWconfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonYELLOWconfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  ButtonConfig* buttonGREENconfig = buttonGREEN.getButtonConfig();
  buttonGREENconfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonGREENconfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);

  buttonREMS.setEventHandler(button_handleEvents);
  buttonGREEN.setEventHandler(button_handleEvents);
  buttonYELLOW.setEventHandler(button_handleEvents);
  buttonRED.setEventHandler(button_handleEvents);

  #ifdef MODE_BT
  DEBUG_SERIAL(.println"The device started in master mode, make sure remote BT device is on!");
  SerialBT.begin("ESP32test", true);
  SerialBT.setTimeout(5000);
  SerialBT.register_callback(btCallback);
  if (!SerialBT.connect(name)) {
    tftPrint(TFT_LEFT, TFT_OBLQ, TFT_RED, TFT_BLACK, 20, 2 * LINEHEIGHT_DEF, 0, "No connection");
    tftPrint(TFT_LEFT, TFT_OBLQ, TFT_RED, TFT_BLACK, 20, 3 * LINEHEIGHT_DEF, 0, "Shutting down");
    delay(2000);
    esp_deep_sleep_start();
  }
  DEBUG_SERIAL.println("Connected Succesfully to " + name);
  #endif 

  #ifdef MODE_WIFI
    ConnectWiFi();
  #endif

  getGrblState(false);

// FOR DBG purposes
 // delay(10000);
  //esp_deep_sleep_start();
}
// END of SETUP =====================================

void ConnectWiFi()
{
    for(uint8_t t = 2; t > 0; t--) {
        DEBUG_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        DEBUG_SERIAL.flush();
        delay(1000);
    }
 
    WiFi.begin(ssid, password);
 
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      DEBUG_SERIAL.print ( "." );
    }
    DEBUG_SERIAL.print("\n[SETUP] Local IP: "); DEBUG_SERIAL.println(WiFi.localIP());

    bool conn = myCNC.connect(fluidnc_host, fluidnc_port);
    if (!conn)
    {
        DEBUG_SERIAL.println("[SETUP] Not connected, entering deep sleep mode.");
        tftPrint(TFT_LEFT, TFT_OBLQ, TFT_RED, TFT_BLACK, 20, 2 * LINEHEIGHT_DEF, 0, "No connection");
        tftPrint(TFT_LEFT, TFT_OBLQ, TFT_RED, TFT_BLACK, 20, 3 * LINEHEIGHT_DEF, 0, "Shutting down");
        delay(2000);
        goto_sleep();
    }
    else 
    {
        DEBUG_SERIAL.printf("[SETUP] Connected! ID:%d\n", myCNC.activeID());
        // Test Command
        cSF(setup_test,1,"\n");
        cSF (myReply, 5);
        myCNC.cmd2(setup_test, myReply);
        DEBUG_SERIAL.printf("[SETUP] ID:%d CNC Reply: ", myCNC.activeID());
        DEBUG_SERIAL.println(myReply);
    }
}
