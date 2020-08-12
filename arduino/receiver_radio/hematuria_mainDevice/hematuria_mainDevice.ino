

/******************* 
 * Date: May 22 2020
 * Author: Rohit Malyala
 * 
 * MIT License
 * Diagrams and reference material (c) 2020 Rohit Malyala
 * 
 * Completed in accordance with the requirements of MEDD419 FLEX of the UBC MD Undergraduate curriculum
 * Funded by Vancouver Coastal Health and VGH Urology, supervised by Dr. Christopher Nguan
 * 
*******************/

// ---------
// LIBRARIES
// ---------

#include "HX711.h"                    // library for the load calibrated.

#include "Adafruit_AS726x.h"          // library for the spectrophotometer module.
#include <Wire.h>                       // library for I2C communication (aka "Wire").
                                      // library for the TFT display.
#include <Adafruit_GFX.h>               // Core graphics library.
// #include <Adafruit_ST7735.h>            // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h>            // Hardware-specific library for ST7789.
#include <SPI.h>                        // Serial Peripheral Interface (SPI) library.
// #include <SD.h>

// #include "RunningAverage.h"           // running average

                                      // library for the rf module
// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;



//-------------------------
// PINOUTS AND DECLARATIONS
//-------------------------

// Required pins for the HX711 load calibrated circuit
// -----------------------------------------------
#define LOADCELL_DOUT_PIN 2 // in our build, uses a green wire;
#define LOADCELL_SCK_PIN 3  // in our build, uses a white wire;
float trueWeight;
float displayWeight;
HX711 scale;

// Required pins for the TFT display
// ---------------------------------
#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


// Pins and objects for the photodiode module
// ------------------------------------------
Adafruit_AS726x ams;                            //create the object
// uint16_t sensorValues[AS726x_NUM_CHANNELS];     //buffer to hold raw values. Not used in this project (but possibly worth trying out instead of using calibrated values.
float calibratedValues[AS726x_NUM_CHANNELS];  //buffer to hold calibrated values
// List of functions in the AS726x library: https://adafruit.github.io/Adafruit_AS726x/html/class_adafruit___a_s726x.html#abe1784ea3362da5ed976c5bb1637be79

// Global variables for graphing+sd card writing+radio signalling
//---------------------------------------------------------------

struct dataStruct{
    float send_hgbratio ;
    float send_loadrate;
    float send_load;
    // unsigned long counter;
    }myData;

byte tx_buf[sizeof(myData)] = {0};



// SETUP AND INIT SCREEN
// ---------------------

// storage of "temporal window" of 10 data points:
long time_0;
long time_1;
float scale_1;
float loadSlope;
float loadSlopeHourly;
float purple_ratio;

void setup(void) {

  Serial.begin(38400);   // set baud rate

  // Initialize ASK Object
    rf_driver.init();

  tft.init(240, 240);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(F("Initializing the Open Hematuria Monitor (OHM). Please ensure that the collection bag is empty. Scale is initializing. \n\n"
              "Development supported by Vancouver Coastal Health, Vancouver General Hospital Department of Urological Sciences.\n\n" 
              "Project undertaken and completed in accordance with curriculum requirements for MEDD 419 'FLEX' of the UBC MD Undergraduate under the supervision of Dr. Chris Nguan."));
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(241.f);     // this value is obtained by calibrating the scale with known weights. Edit it if your load calibrated is off.
  scale.tare();               // reset the scale to 0


  //begin and make sure we can talk to the spectral sensor
  if(!ams.begin()){
    // tft.print(F("The device could not connect to the spectral sensor! Please check your wiring."));
    while(1);
  }

  

  // wait four seconds, then flush screen before going into loop function
  delay(3000);
  tft.fillScreen(ST77XX_BLACK);
  

}


// LOOP FUNCTION
// --------------------
/*  this code is set up such that various functions in the section below are called upon in a certain order to display certain types
    of information at a time. The available screens are: 
    
    screenUpdate_rawtextdata(). 
    This shows immediately current sensor data as raw text. It shows the weight sensed on the load sensor; it shows the rate of
    change in the load sensor data (and builds a "flow rate" out of it), and it shows the spectrophotometer (calibrated)
    readings in each of the six measured wavelengths.

    screenUpdate_weightgraphs()
    This screen shows two graphs of the moving averages for current weight, and for the rate of change in the weight.

    screenUpdate_hematuriagraphs()
    This screen shows the estimated rate of blood loss.
*/

void loop() {

  time_0 = millis();
  
  ams.startMeasurement(); //begin a measurement
  
  bool rdy = false;
  while(!rdy){
    delay(5);
    rdy = ams.dataReady();
  }
 
  ams.readCalibratedValues(calibratedValues); 

  
  purple_ratio = calibratedValues[AS726x_VIOLET]/calibratedValues[AS726x_RED];
   
  screenUpdate_rawtextdata();

  time_1=millis();
  scale_1=scale.get_units(20);
  loadSlope=10*round(((scale_1-trueWeight)/(time_1-time_0)*(1000)*(60))/10);
  loadSlopeHourly=loadSlope/1000*60;

  // logarithmic relationship between the purple:red transmittance ratio and hgb color grade intensity.
  // myData.send_hgbratio=-1.84443*log(purple_ratio)+3.1772;
  // new formula uses exponential regression b/c no axis intercepts theoretically possible
  myData.send_hgbratio=7.90445*pow(0.37612, purple_ratio);
  myData.send_loadrate=loadSlopeHourly;
  myData.send_load=scale_1; 

  memcpy(tx_buf, &myData, sizeof(myData) );
  byte zize=sizeof(myData);
  rf_driver.send((uint8_t *)tx_buf, zize);

  // driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();
  
  
}






// "DISPLAYING" FUNCTIONS
// 1: raw text
// 2: graph of load+flow rate
// 3: graph of hematuria grade
// -----------------------------------

void screenUpdate_rawtextdata(){

  // Display load sensor data
  tft.setCursor(0, 0);

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(F("LOAD CELL DATA"));
  // figure out how to make an underline lol
  tft.println(F("Weight on load cell:"));
  
  trueWeight=scale.get_units(20);
  if (trueWeight<0){
    displayWeight =0;}
  else{
    displayWeight = trueWeight;}
  
  tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);
  tft.print(displayWeight);  
  tft.print(F(" g             "));

  tft.println("");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(F("\nRate of change on load cell:"));
  
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.print(loadSlope);
  tft.println(F(" mL/minute              "));
  tft.print(loadSlopeHourly);
  tft.print(F(" L/hour                  "));

  //Display spectrophotometer data. While loop included at start is to "wait" until data is available.
    uint8_t temp = ams.readTemperature();
  
  // alternatively: 
  // ams.readRawValues(sensorValues);

  tft.println("\n");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(F("SPECTROMETER DATA"));
  tft.print(F("Temperature: "));
  tft.print(temp);
  tft.println(F(" degrees C\n"));
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  tft.print(purple_ratio);
  tft.println(" (red:purple ratio)        ");
  float regression_grade_est = 7.90445*pow(0.37612, purple_ratio);
  
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(regression_grade_est);
  tft.println(" (hematuria grade estimate)");
  
  //"textual" color grading readout.
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  colorGrade_hematuria();
  print_spectrophotometerValues();
  
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(F("\n\nDevice running for: "));
  calcDisplayElapsedTime();
  tft.print(" (h:m:s)     ");
 
}

/*
void graph(){
  // read the sensor and map it to the screen height
  int drawHeight = map(loadSlope, 0, 1023, 0, tft.height());

  // print out the height to the serial monitor
  Serial.println(drawHeight);

  // draw a line in a nice color
  tft.line(250, 180, 10);
  tft.line(xPos, tft.height() - drawHeight, xPos, tft.height());

  // if the graph has reached the screen edge
  // erase the screen and start again
  if (xPos >= 160) {
    xPos = 0;
    tft.background(250, 16, 200);
  } else {
    // increment the horizontal position:
    xPos++;
  }

  delay(16);
}
*/


// DATA "OBTAIN/PROCESS" FUNCTIONS

void calcDisplayElapsedTime(){

  long hours=(millis()/(1000)/60/60);
  long minutes=(millis()/1000/60)%60;
  long seconds=millis()/(1000)%60;
  
  if(hours<10){
    tft.print('0');}
  tft.print(hours);
  tft.print(":");
  if(minutes<10){
    tft.print('0');}
  tft.print(minutes);
  tft.print(":");
  if(seconds<10){
    tft.print('0');}
  tft.print(seconds);
  
}

void colorGrade_hematuria(){

  // the color calculation uses RATIOS between the calibrated sensor values rather than specific 
  // intervals of wattages/area according to a lookup table. The justification for this is that 
  // the clip-on sensor isn't fully light isolated. external light can get in through the sides
  // of exposed tubing, also powering the photodiode.

  // however, the chosen ratios are calibrated according to the white LED used in this project.
  // NOT EVERY WHITE LED EMITS AT THE SAME WAVELENGTHS, AND AT THE SAME POWER. This is ESPECIALLY
  // NOT THE CASE if you decide to use a DIFFERENT resistor (67K used in our implementation). While
  // it is likely the device will *work* it may not quite match up with a clinician estimate of what
  // "gross" or "high grade" hematuria should look like. 
  
  if(purple_ratio>=6){
    tft.print(F("Grade 0, no visible hematuria"));
  }
  else if(6>purple_ratio && purple_ratio>=3){
    tft.print(F("Grade 1-2, mild hematuria    "));
  }
  else if(3>purple_ratio && purple_ratio>=1){
    tft.print(F("Grade 3-4: medium hematuria  "));
  }
  else if(1>purple_ratio && purple_ratio>=0.5){
    tft.print(F("Grade 4-5: moderate hematuria"));
  }
  else if(0.5>purple_ratio && purple_ratio>=0.35){
    tft.print(F("grade 6-7, marked hematuria  "));
  }
  else if(0.35>purple_ratio && purple_ratio>=0.0){
    tft.print(F("grade 8+, severe hematuria "));
  }
  else{
    tft.print(F("Hematuria not noted. Error.  "));
  }
}

/*
void sendColorGradeRF(){
   // if changing text outputs, make sure all char outputs are exactly 8 characters. Use spaces to pad out if necessary.
   
   
   if(purple_ratio>=6){
    myData.send_hgbratio=0;
  }
  else if(6>purple_ratio && purple_ratio>=3){
    myData.send_hgbratio=1;
  }
  else if(3>purple_ratio && purple_ratio>=2){
    myData.send_hgbratio=3;
  }
  else if(2>purple_ratio && purple_ratio>=1){
    myData.send_hgbratio=5;
  }
  else if(1>purple_ratio && purple_ratio>=0.40){
    myData.send_hgbratio=7;
  }
  else if(0.45>purple_ratio && purple_ratio>=0.0){
    myData.send_hgbratio=9;
  }
  else{
    myData.send_hgbratio="Err";
  }
  
}
*/

void print_spectrophotometerValues(){
  
  tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.println(F("\n\nRed channel (650nm):"));
    tft.println(calibratedValues[AS726x_RED]);
  tft.setTextColor(ST77XX_ORANGE, ST77XX_BLACK);  
    tft.println(F("Orange channel (600nm):"));
    tft.println(calibratedValues[AS726x_ORANGE]);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
    tft.println(F("Yellow channel (570nm):"));    
    tft.println(calibratedValues[AS726x_YELLOW]);
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);  
    tft.println(F("Green channel (550nm):"));
    tft.println(calibratedValues[AS726x_GREEN]);
  tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK);  
    tft.println(F("Blue channel (500nm):"));;
    tft.println(calibratedValues[AS726x_BLUE]);
  tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);    
    tft.println(F("Violet channel (450nm):"));
    tft.println(calibratedValues[AS726x_VIOLET]);
}
/*
void screenUpdate_weightgraphs()
void screenUpdate_hematuriagraphs()
*/
