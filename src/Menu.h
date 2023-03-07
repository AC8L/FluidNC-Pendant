// Menu choice handling code (menu by green button)
// for FluidNC pendant
//
void UI_Menu()
{
    switch (menuChoice) {
          case poweroff:
            powerOffNow=true;
            break;
          case brightness:
            pState = MenuBrightness;
            break;
          case spindleonoff:
            myCNC.SpindleOnOff(setSpindleSpeed);
            pState = Pendant;
            break;
          case spindlespeed:
            spindleDisp = setSpindleSpeed;
            pState = MenuSpindleSpeed;
            break;
          case steps:
            smallSteps = !smallSteps;
            pState = Pendant;
            break;
          case unlock: //Unlock
            pState = Pendant;
            myCNC.Unlock();
            break;
          case home: //Home
            pState = Pendant;
            myCNC.Home();
            break;
          case resetgrbl: //Reset
            pState = Pendant;
            myCNC.Reset();
            break;
          case setwxy0:  //XY=0
            pState = MenuConfirm;
            break;
          case setwz0:  //Z=0
            pState = MenuConfirm;
            break;
          case probe:  //probe
            myCNC.fluidCMD( (const char *)"G21G91" );
            myCNC.fluidCMD((const char *)"G38.2 Z-10F100");
            myCNC.fluidCMD((const char *)"G0Z0.3");
            myCNC.fluidCMD((const char *)"G38.2Z-2F10");
            myCNC.fluidCMD((const char *)"G10 L20 P1 Z21.35");
            myCNC.fluidCMD((const char *)"G91");
            myCNC.fluidCMD((const char *)"G0 Z8.65");
            myCNC.fluidCMD((const char *)"G90");
            pState = Pendant;
            break;
          case gawxy0:  //Set spindle to work coordinates origin position: wXY=0
            DEBUG_SERIAL.println("[UI_Menu] Restore origin.");
            pState = Pendant;
            tftUpdate(true);
            myCNC.fluidCMD((const char *)"$J=G53Z0F1000");  //Lift Spindle
            delay(100);
            waitEndJog();
            myCNC.fluidCMD((const char *)"$J=X0 Y0 F5000");
            delay(100);
            getGrblState(false);
            delay(100);
            waitEndJog();
            break;
          default:
          case quit:
            pState = Pendant;
            break;
        }
}