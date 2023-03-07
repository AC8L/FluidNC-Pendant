#define DEBUG

//#define MODE_BT
#define MODE_WIFI

#include "Pendant.h"

// Main loop ==========================================
unsigned long lastUpdate = millis();

void loop()
{
  sinceStart = millis();
  activeTimer(false);
  
  checkRotateA();
  checkJoystick();

  buttonREMS.check();
  buttonGREEN.check();
  buttonYELLOW.check();
  buttonRED.check();

  if (myCNC.isConnected() && lastUpdate + statusUpdateInterval<millis()){
        DEBUG_SERIAL.println("[MAIN] Status update...");
        checkBattery();
        getGrblState(true);
        checkConnect();
        tftUpdate(false);
        lastUpdate = millis();
    }

  if (rdX == 0 && rdY == 0 && rdZ == 0)
  {
    //buttonREMS.loop();
    //buttonGREEN.loop();
    //buttonYELLOW.loop();
  }
    //  Serial.print("."); //Check of er puntjes overblijven in de loop
}
// END of main loop =================================================
