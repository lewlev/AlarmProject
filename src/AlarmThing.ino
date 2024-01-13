// if the project fails to compile you may need to "include arduino_secrets.h" with #include line here
// the include is needed if the project is editied using the local PC IDE editor 
// if the project is created and edited entirely with arduino cloud web editor it is not required
                         
// software version info
#define VerMaj 1
#define VerMin 9




//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  AlarmProject Part I  Alarm Controller  main ino   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  


// Cloud Variables 
//  int alarmHist;             // count of alarms since last system armed used to graph alarm history in the history dash board
//  bool alarmAct;             // Dashboard indicator of of current alarm status (green tick alarm active, red cross no alarm) kinda counter inutative but reverse logic is no better 
//  bool aRMED;                // Dashboard Armed switch ( On is armed) 
//  bool blinkPower;           // Dashboard indicator flashes every second when system is powered uo abd running
//  bool mUTESIREN;            // Dashboard switch when on siren is muted.
//  bool pIR1;                 // Dashboard PIR status indictore (green clear, red alarmed) 
//  bool pIR2;                 // Dashboard PIR status indictore (green clear, red alarmed)
//  bool pIR3;                 // Dashboard PIR status indictore (green clear, red alarmed)
//  bool pIR4;                 // Dashboard PIR status indictore (green clear, red alarmed)
//  bool sIRENsTROBEWidget;    // Dashboard indicator green tick  indicates siren and or strobe currently on  ( ran out of vaiables so had to combine)

*/

#include "driver/gpio.h"
#include "thingProperties.h"   // Arduino IOT
#include <Arduino_ESP32_OTA.h> // enable over the air firmare upload
#include <EEPROM.h>            // Used to store the system state.
#include <HTTPClient.h>        // required for IFTTT integration


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!! User defined options
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Comment these  out to allow emails and notifications
//#define StopSendEmail
//#define StopSendNotify

// UnComment this line to enable test mode. Generaly sets timers to low values, speed's up debuging process.
//#define ALARMTEST

// comment out the line below to turn off debug printing
#define DEBUG

// comment out this line for PIRS that produce a low output when in the alarmed state.  
#define AlarmHigh

//   #################### Enter ifttt key here  ################
//   should look like    this   const char *key = "cMvrU_lQ51_jFlNXWJTYKa"; //  used for both email and push notofications
const char *key = "your iftt key from ifttt account";          //  used for both email and push notofications


// To adjust  alarm hold time and maximium siren on time see
// !!!!!!!!!!!!!!!!!!!!!!!! End User time out preferences
// further down.


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!! End User defined options debuging options
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 // turn debug serial printing on/offf
#ifdef DEBUG           
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)   // DPRINT(...)
#define DPRINTLN(...) // DPRINTLN(...)
#endif

int Pir1Pin = 19;                  // PIR pin numbers
int Pir2Pin = 4;                   // PIR pin numbers
int Pir3Pin = 21;                  // PIR pin numbers
int Pir4Pin = 16;                  // PIR pin numbers
int StrobePin = 17;                // Stobe driver relay pin
int SirenPin = 18;                 // Sirens Driver relay pin

unsigned long AE_Start = millis(); // Alarm event start time
int AlState = 0;                   // System in logical Alarm condition   0 = clear  1=alarm condition
bool SysUp = false;                // when setup finshes system is up and SysUp will be set rue

#define EEPROM_SIZE 10   //         Size of EEPROM alllocated
byte EPaRMED = 0;        //         last known value of aRMED System in logical Alarm condition   0 = clear  1=alarm condition
byte EPmUTESIREN = 0;    //         last known value of mUTESIREN
bool mUTESTROBE = false; //         only effected by loss of connection no user disable in firmware

// !!!!!!!!!!!!!!!!!!!!!!!!!!  Test or Live mode  system time outs  !!!!!!!!!!!!!!!!!!!!!!!!!!
#ifdef ALARMTEST                   

// TEST mode time out settings
int AE_Hold = 1;                     // Test alarm state and emails are limited by this delay in minutes
unsigned long MaxSirenTime = 20000;  // test 20 seconds Siren will turn off after this delay even if Alarm still on 3 minutes in milli seconds
unsigned long SirenLockout = 100000; // test 1min 40secduration of lockout after siren has timmed out(MaxSirenT expired) 15 minutes in milli seconds

#else

// Live mode time out settings
int AE_Hold = 10; //                     10 minutes  alarm state and emails are limited by this delay in minutes
//                                       Alarm condition will hold for a minimium of this duration
//                                       Like wise the cleared condition will hold for this duration
unsigned long MaxSirenTime = 180000; //  3 minutes   Siren will turn off after this delay even if Alarm still on 3 minutes in milli seconds
unsigned long SirenLockout = 900000; //  15 miniutes duration of lockout after siren has timmed out(MaxSirenT expired) 15 minutes in milli seconds

#endif

unsigned long SirenStart = millis() - MaxSirenTime; // Siren on start time                                            milli seconds
unsigned long SirenEndTime = millis();              // Siren turn off time milli seconds
unsigned long BlinkerTime = millis();               // Used with power / run dashboard widget

int StrobeStat = 0;          //                     logical status of strobe light 1 when strobe is on
int SirenStat = 0;           //                     logical status of sirens 1 when sirens on even if the sirens are muted by mUTESIREN dashboard switch

bool Connected = false;      //                     flag that indicates ESP32 has connected to Arduino cloud
bool ConnectionLost = false; //                     flag that inidcates the ESP had lost contact with Arduino cloud
bool IoTSynced = false;      //                     flag that indicates IOT cloud variables are in sysnc with local version


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


// prototypes
int SoundAlarm(int mode);
void onIoTSync();



// ifttt appletts used to send notifications and emails
// const char *event = "Alarm_Event"; //                IFTT email apllete name
const char *iftttURL1 = "https://maker.ifttt.com/trigger/"; //                IFTT URL 1
const char *iftttURL2 = "/with/key/";                       //                IFTT URL 2




// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!       Setup
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void setup()
{

    // Initialize serial and wait for port to open:
    Serial.begin(115200);
    // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
    delay(1000);
    while (!Serial){}

    Serial.print("Ver ");
    Serial.print(VerMaj);
    Serial.print(".");
    Serial.println(VerMin);

    DPRINTLN("initProperties");
    // Defined in thingProperties.h
    initProperties();
    delay(1500);
    // Connect to Wi-Fi
    WiFi.begin(SECRET_SSID, SECRET_OPTIONAL_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        // DPRINT(".");
    }
    DPRINTLN("");
    DPRINTLN("WiFi on");
    WiFi.mode(WIFI_STA);
    // Initialise EEPROM - used to hold system state
    EEPROM.begin(EEPROM_SIZE);

    // Connect to Arduino IoT Cloud
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    // set up IOT call back events
    ArduinoCloud.addCallback(ArduinoIoTCloudEvent::SYNC, onIoTSync);
    ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, OnConnect);
    ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, OnDisconnect);

    /*
       The following function allows you to obtain more information
       related to the state of network and IoT Cloud connection and errors
       the higher number the more granular information you’ll get.
       The default is 0 (only errors).
       Maximum is 4
    */

    setDebugMessageLevel(4);
    ArduinoCloud.printDebugInfo();

    // define ESP32 pin usage

    //strobe and siren relay pins
    pinMode(StrobePin, OUTPUT);
    pinMode(SirenPin, OUTPUT);

    // PIR pins, set to  input only, no internal pull downs, on board add external 15k ohm pullups connected to 3v3
    gpio_set_direction(gpio_num_t(Pir1Pin), GPIO_MODE_INPUT);
    gpio_set_direction(gpio_num_t(Pir2Pin), GPIO_MODE_INPUT);
    gpio_set_direction(gpio_num_t(Pir3Pin), GPIO_MODE_INPUT);
    gpio_set_direction(gpio_num_t(Pir4Pin), GPIO_MODE_INPUT);
    // PIR pins, no internal pull downs.
    gpio_pulldown_dis(gpio_num_t(Pir1Pin));
    gpio_pulldown_dis(gpio_num_t(Pir2Pin));
    gpio_pulldown_dis(gpio_num_t(Pir3Pin));
    gpio_pulldown_dis(gpio_num_t(Pir4Pin));
    // Note  on board add external 15k ohm pullups from each pir pin connected to 3v3. 

    //                                            set both siren's off (internal and external are on the same relay) and strobe off
    digitalWrite(SirenPin, LOW);
    digitalWrite(StrobePin, LOW);

    AE_Start = millis() - AE_Hold * 60000; //     at startup allow alarms to be triggered immediately when SysUp
    DPRINTLN("UpdateIOT");
    int count = 0;
    while (IoTSynced == false)
    {
        ArduinoCloud.update(); //                this loop here till cloud variables are in sync
        delay(200);
        DPRINT("+");
        count++;
        if (count > 100)
        {
            DPRINTLN("Timmed out"); //           waiting for IOT connect");
            fatalError();
        }
    }

    // DPRINTLN("Exit Setup");
    SysUp = true;                     // set the SysUp flag
    DPRINTLN("SysUp");
}
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! end of setup !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  Main loop
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void loop()
{

    // read PIRS
    
#ifdef AlarmHigh // PIR need to reverse the logic
    // PIR's Activehigh, in alarm condition,       digitalRead(PirxPin) high(1) pIRx will be low  (0)  LED on dash board will be red x
    //                   in all clear condition ,  digitalRead(PirxPin) low (0) pIRx will be high (1)  LED on dash board will be green tick
    pIR1 = !digitalRead(Pir1Pin);
    pIR2 = !digitalRead(Pir2Pin);
    pIR3 = !digitalRead(Pir3Pin);
    pIR4 = !digitalRead(Pir4Pin);
#else
    // PIR's !Activehigh, in alarm condition,       digitalRead(PirxPin) low (0) pIRx will be low  (0)  LED on dash board will be red x
    //                    in all clear condition ,  digitalRead(PirxPin) high(1) pIRx will be high (1)  LED on dash board will be green tick
    pIR1 = digitalRead(Pir1Pin);
    pIR2 = digitalRead(Pir2Pin);
    pIR3 = digitalRead(Pir3Pin);
    pIR4 = digitalRead(Pir4Pin);
#endif

    if ((StrobeStat + SirenStat) > 0)
    {
        sIRENsTROBEWidget = true; //               turn on Dashboard indicator widget
    }
    else
    {
        sIRENsTROBEWidget = false; //              turn off Dashboard indicator widget
    }

    ArduinoCloud.update(); //                     testing indicated this update takes about 10ms for 1st itteration  then 4 or 5 itterations  @ 1ms or less then 10ms and so on.

    // toggle power blinker on dash board
    if (millis() > BlinkerTime + 1000)
    {
        blinkPower = !blinkPower;
        BlinkerTime = millis();
    }
    // Monitor siren on time turn off when on limmit is reached
    if ((SirenStat > 0) && (millis() > SirenStart + MaxSirenTime)) //          if on timeout exceeded turn it off
    {
        SoundAlarm(0);
    }

    if (!aRMED) //                                                             check last setting armed switch on dashboard
    {
        // turned off so set state to clear and turn off stoble and siren
        AlState = false;    //                                                  not armed set state to idle
        alarmAct = AlState; //                                                  alarmAct is DB representation of AIstate
        // Turn of the siren if its on
        ActStrobe(0); //                                                        strobe off
        if (SirenStat > 0)
        {
            SoundAlarm(0); //                                                   siren off
        }
       
    }
    else // if (!aRMED)   so armed
    {
        alarmAct = AlState;                  // ensure Alarm widget on DB reflects system state
        if (!(pIR1 && pIR2 && pIR3 && pIR4)) // check PIRS for any movement this condition true alarm condition exists
        {
            if (AlState) // alarm condition already exists
            {

                if ((millis() - AE_Start) < (AE_Hold * 60000)) return; // locked out do nothing and got start loop
                // Alarm lock out expired and alarm still exists restart the event
                AE_Start = millis(); // unsure about this, restart the event
                SoundAlarm(1);       // turn on internal and external alarm and strobe if siren not locked out
                ActStrobe(1);
                SendEmail("Alarm is still Active"); // send eail  this function respects lockout periodes
                SendNotification();                 // send push notification this function respects lockout periodes
            }
            else //  elseif (AlState) not in alarm condition but pirs tripped
            {
                DPRINTLN("New Alarm ");
                AlState = true;     // set alarm state on
                alarmAct = AlState; // set alarm status in dash board
                SoundAlarm(1);      // turn sirens on if not already on
                ActStrobe(1);
                alarmHist++;
                SendNotification();
                SendEmail("Alarm is now Active");
                AE_Start = millis(); // capture Alarm event start time
            }                        // else //  elseif (AlState) not in al
        }
        else // if (!(pIR1&&pIR2&&pIR3&&pIR4))   pirs are all clear
        {
            if (!AlState) // not in an Alarm state and no PIRS triped (normal state)
            {
                alarmAct = AlState; // show Alarm off on Dashboard
                return;             // got start loop
            }
            else // if (!AlState)  in alarm state
            {
                if ((millis() - AE_Start) > (AE_Hold * 60000)) // if true not locked into alarm hold delay
                {
                   DPRINTLN("Alarm Cleared ");
                    AlState = false; // set alarm state off
                    alarmAct = AlState; // set alarm status in dash board off
                    SoundAlarm(0);      // turn off sirens
                    ActStrobe(0);       // turn strobe off
                    // cant realy send a push notification limmited to two appelts for free IFTTT
                    SendEmail("Alarm is now Cleared"); // send eail  this function respects lockout periodes
                }                                      // AE_Start = millis();   // test this
            }                                          // else if (!AlState)
        }                                              // else if (!(pIR1&&pIR2&&pIR3&&pIR4))
    }                                                  // else if (!aRMED)
}

// ------------------------------------------------------------------------------------------
// If we get here, then something bad has happened so easiest thing is just to restart.
// ------------------------------------------------------------------------------------------
void fatalError()
{
    Serial.println("Fatal error - restarting.");
    delay(1000);
    ESP.restart();
}
// Use a webhook to send an email via ifttt when an alarm is detected
bool SendEmail(String Emessage)
{
#ifndef StopSendEmail   // only send if configured to do so
    bool Ret = true;
    HTTPClient http;

    String urlhttp = String(iftttURL1) + "Alarm_Event" + String(iftttURL2) + String(key);
    http.begin(urlhttp);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST("{\"value1\":\"" + Emessage + "\",\"value2\":\" \",\"value3\":\" \"}"); //            , {\"value2\":"30"}, {\"value3\":"40"}");
    if (httpResponseCode > 0)
    {
    }
    else
    {
        Ret = false;
    }
    http.end();
    return Ret;
#else
    DPRINTLN("Email: " + Emessage);
    return true;
#endif
}

// Use a webhook to send a notificatiom via ifttt when an alarm is detected
// need the ifttt ap on the recipients phone
bool SendNotification()
{
#ifndef StopSendNotify   // only send if configured to do so
    bool Ret = true;
    HTTPClient http;
    // String urlhttp = "https://maker.ifttt.com/trigger/Alarm_Notification/with/key/" + String(key);
    String urlhttp = String(iftttURL1) + "Alarm_Notification" + String(iftttURL2) + String(key);
    http.begin(urlhttp);
    int httpResponseCode = http.POST("");
    if (httpResponseCode > 0)
    {
    }
    else
    {
        Ret = false;
    }
    http.end();
    return Ret;
#else
    DPRINTLN("Push Notify: ");
    return true;
#endif
}

// sound an alarm;
// mode
// 0 Silence all sirens
// 1 sirens on
// only turn on siren if not exceeded to on time and not in the locked out period

int SoundAlarm(int mode) // internal and external alarm and strobe
{
    // If requested mode is off (0) just do it
    if (mode == 0)
    {
        SwitchSiren(0);                             // turn off
        if (SirenStat > 0) SirenEndTime = millis(); // if siren was on set end siren time
        SirenStat = 0;
        return 0;
    }

    // Check if the siren is locked out after the last sounding (Try not to annoy neighbours)
    if ((SirenEndTime > SirenStart) && (millis() < (SirenEndTime + SirenLockout))) // Siren is lock out period
    {
        SwitchSiren(0); // to be sure turn it off
        SirenStat = 0;
        return 0;
    }

    // Check that the siren on time limit is not exceeded
    // if so check if the siren is already in the current mode if so just exit
    if (millis() < (SirenStart + MaxSirenTime)) // within siren on time limit
    {
        if (SirenStat == mode)
        {
            return SirenStat; // already in the correct mode
        }
    }
    SwitchSiren(mode);
    SirenStat = mode; // logical siren state not effected by muting
    return SirenStat;
}

// Turn strobe light on or off just follows system alarm status no time outs or muting
int ActStrobe(int mode)
{
    if (mUTESTROBE) // over rides logical setting
    {
        digitalWrite(StrobePin, LOW); // IF STROBE IS MUTED DONT TURN IT ON
    }
    else
    {
        digitalWrite(StrobePin, mode); // NO MUTING STOBE FOLLOWS MODE SET
    }
    StrobeStat = mode;
    return StrobeStat;
}

// Turn Siren on or off.  Respects mUTESIREN flag and restarts siren starttime when required.
void SwitchSiren(int SSmode)
{

    if (mUTESIREN)
    {
        digitalWrite(SirenPin, LOW); // IF SIREN IS MUTED TURN IT OFF
    }
    else
    {
        digitalWrite(SirenPin, SSmode); // NO MUTING Siren FOLLOWS MODE SET
    }

    if (SSmode > 0) // if turning on set start time
    {
        SirenStart = millis();
    }
}

void onIoTSync()
{
    // note this is a once off event that is the sync event only happens once after connection to cloud is established.
    // It does not as I assumed, reset itself when a cloud variable is changed locally in the code but Arduino_Update() has not yet run.
    // If the device looses connetion to the cloud then it does reset efectively
    IoTSynced = true;
}

void OnConnect()
{
    DPRINTLN("Connected.");

    Connected = true;
    if (ConnectionLost == true) // recovering from disconnect restore last settings
    {
        ConnectionLost == false;
        EEPROM.get(0, EPaRMED);   // get last armed status 
        aRMED = (bool)EPaRMED;    // set the local variable 
        EEPROM.get(1, EPmUTESIREN);
        mUTESIREN = (bool)EPmUTESIREN;
        pIR1 = 0;     // set cleared
        pIR2 = 0;     // set cleared
        pIR3 = 0;     // set cleared
        pIR3 = 0;     // set cleared
        alarmAct = 0; // no alarm
        sIRENsTROBEWidget = false;
        mUTESTROBE = false; // not user setable only comes on when connection lost
    }
    else // cold start
    {
        aRMED = false;
        mUTESIREN = false;
        mUTESTROBE = false;
        pIR1 = 0;     // set cleared
        pIR2 = 0;     // set cleared
        pIR3 = 0;     // set cleared
        pIR3 = 0;     // set cleared
        alarmAct = 0; // no alarm
        sIRENsTROBEWidget = false;
    }
}

void OnDisconnect()
{
    DPRINTLN("Disconnected");

    IoTSynced = false;
    ConnectionLost = true;
    if (SysUp) // save aRMED mUTESIREN status to EEPROM and mute the siren
    {

        EPaRMED = 0;
        if (aRMED) EPaRMED = 1; // save the current aRMED setting
        EEPROM.put(0, EPaRMED);
        EPmUTESIREN = 0;
        if (mUTESIREN) EPmUTESIREN = 1; // save the current mUTESIREN setting
        EEPROM.put(1, EPmUTESIREN);
        EEPROM.commit();
        // for piece of mind of owner and neighbours turn the alarms and lights off when there is no remote control possible
        // keep the sysetm aRMED it was before so the history keeps going and email alerts may be sent depending what has caused the
        // Disconnect
        mUTESIREN = true;  // mute the siren when we cant communicate with panel
        mUTESTROBE = true; // mute the strobe when we cant communicate with panel
    }
}

void onARMEDChange()
{

    if (aRMED)
    {
        SirenEndTime = millis() - SirenLockout; //   allow imediate siren on rearmining expire the lockout
        alarmHist = 0;                          //                            the alarm count goes back to 0 when the alarm is rearmed
    }
    EPaRMED = 0;
    if (aRMED) EPaRMED = 1;
    EEPROM.put(0, EPaRMED); //                       write and commit the new value to EEPROM
    EEPROM.commit();
}

void onMUTESIRENChange() // Dasboard user controlled switch
{
    DPRINTLN("onMUTESIRENChange()");
    SwitchSiren(SirenStat); // set siren on or off, based on current SirenStat and  new mUTESIREN value
    EPmUTESIREN = 0;
    if (mUTESIREN) EPmUTESIREN = 1;
    EEPROM.put(1, EPmUTESIREN); // write and commit the new value to EEPROM
    EEPROM.commit();
}
void onPIR3Change()
{
}

void onBlinkPowerChange() {}

void onPIR4Change()
{
}

void onAlarmHistChange()
{
}

void onSIRENsTROBEWidgetChange()
{
}

void onPIR1Change()
{
}

void onPIR2Change()
{
}

void onAlarmChange()
{
}

void onAlarmActChange()
{
    // Add your code here to act upon AlarmAct change
}
