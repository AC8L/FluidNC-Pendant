// Configuration file for FluidNC Pendant
// To keep WiFi username/password and fluidNC hostname separate from the rest of the code
// rename this file to Config.h and fill-in your data
//
const char *ssid = "<your Wi-Fi SSID>";
const char *password = "<your Wi-Fi SSID password>";
const char *fluidnc_host = "fluidnc.local"; // this is default MDNS name, feel free to change to match your setup
const uint16_t fluidnc_port = 81; // chnage only if FLuidNC websocket port id chnaged (rare cases)
