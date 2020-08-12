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
#include <Adafruit_ST7735.h>            // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h>            // Hardware-specific library for ST7789.
#include <SPI.h>                        // Serial Peripheral Interface (SPI) library.

#include "RunningAverage.h"           // running average

                                      // library for the wifi module


//-------------------------
// PINOUTS AND DECLARATIONS
//-------------------------

// Required pins for the HX711 load calibrated circuit
// -----------------------------------------------
const byte LOADCELL_DOUT_PIN=2; // green; use "byte" not "int," although memory isn't scarce just yet
const byte LOADCELL_SCK_PIN=3;  // white; if you change "const byte" to #define you may save a couple dozen bytes of RAM, although scope control is lost
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
uint16_t sensorValues[AS726x_NUM_CHANNELS];     //buffer to hold raw values
float calibratedValues[AS726x_NUM_CHANNELS];  //buffer to hold calibrated values
// List of functions in the AS726x library: https://adafruit.github.io/Adafruit_AS726x/html/class_adafruit___a_s726x.html#abe1784ea3362da5ed976c5bb1637be79


// SETUP AND INIT SCREEN
// ---------------------

// storage of "temporal window" of 10 data points:
long time_0;
long time_1;
float scale_1;
float loadSlope;
float loadSlopeHourly;

bool proDebug = 0;
uint16_t graphColor = ST77XX_WHITE;
uint16_t pointColor = ST77XX_WHITE;
uint16_t lineColor = ST77XX_WHITE;
String graphName = F("Rate of change in load sensor");
int graphRange = 100;
int markSize = 3;

int valueBlock[500];
int timeBlock[500];
int locationBlock[500];
int valuePos;
int blockPos;
float temp;

//calculate values for graphing
const int numberOfMarks = 8;
const int originX = 45;
const int originY = 200;
const int sizeX = 270;
const int sizeY = 150;
const int deviation = 30;

int boxSize = (sizeX / numberOfMarks);
int mark[] = {(boxSize + deviation), ((boxSize * 2) + deviation), ((boxSize * 3) + deviation), ((boxSize * 4) + deviation), ((boxSize * 5) + deviation), ((boxSize * 6) + deviation), ((boxSize * 7) + deviation), ((boxSize * 8) + deviation)};
const int minorSizeY = (originY + 10);
const int minorSizeX = (originX - 10);

int numberSize = (sizeY / 6);
int number[] = {numberSize, (numberSize * 2), (numberSize * 3), (numberSize * 4), (numberSize * 5), (numberSize * 6)};

int numberValue = (graphRange / 6);
int val[] = {graphRange, (numberValue * 5), (numberValue * 4), (numberValue * 3), (numberValue * 2), numberValue};


void setup(void) {

  Serial.begin(38400);   // set baud rate

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
    tft.print("The device could not connect to the spectral sensor! Please check your wiring.");
    while(1);
  }

  

  // wait four seconds, then flush screen before going into loop function
  delay(3000);
  tft.fillScreen(ST77XX_BLACK);

  //testing graphing screen
  if(proDebug)
    {
    Serial.begin(9600);
    while(!Serial) {};
    }
  
    tft.reset();
    uint16_t identifier = tft.readID();
    identifier=0x9341;

    tft.begin(identifier);
    tft.setRotation(1);

    drawHome();
    drawGraph();
  
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
  /*
  time_0 = millis();
  
  screenUpdate_rawtextdata();
  
  time_1=millis();
  scale_1=scale.get_units(20);
  loadSlope=10*round(((scale_1-trueWeight)/(time_1-time_0)*(1000)*(60))/10);
  loadSlopeHourly=loadSlope/1000*60;
  
  tft.print("\n\n");
  tft.println((time_1-time_0));
*/
  graph();
}






// "DISPLAYING" FUNCTIONS
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
  
  ams.startMeasurement(); //begin a measurement
  bool rdy = false;
  while(!rdy){
    delay(5);
    rdy = ams.dataReady();
  }
    
  ams.readCalibratedValues(calibratedValues); // alternatively: ams.readRawValues(sensorValues);

  tft.println("\n");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(F("SPECTROMETER DATA"));
  // figure out how to make an underline lol
  tft.print(F("Temperature: "));
  tft.print(temp);
  tft.print(F(" degrees C"));

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
  

  // Display current time
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(F("\n\nDevice has been running for:"));
  calcDisplayElapsedTime();
 
}

/*
void showLoadGraph(){
  
 // draw title
 tft.setCursor(10, 10); // set the cursor
 tft.setTextColor(ST77XX_WHITE); // set the colour of the text
 tft.setTextSize(4); // set the size of the text
 tft.println("graphName");
 // draw outline
 tft.drawLine(originX, originY, (originX + sizeX), originY, graphColor);
 tft.drawLine(originX, originY, originX, (originY - sizeY), graphColor);
 // draw lables
 for(int i = 0; i < numberOfMarks; i++)
 {
   tft.drawLine(mark[i], originY, mark[i], minorSizeY, graphColor);
 }
 // draw numbers
 for(int i = 0; i < 6; i++)
 {
   tft.drawLine(originX, (originY - number[i]), minorSizeX, (originY - number[i]), graphColor);
 }
 // draw number values
 for(int i = 0; i < 6; i++)
 {
   tft.setCursor((minorSizeX - 30), (number[i] + numberSize));
   tft.setTextColor(graphColor);
   tft.setTextSize(1);
   tft.println(val[i]);
 }
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


void drawHome()
{
  tft.fillScreen(BLACK);
  delay(500);
  
  tft.setCursor(10, 10); // set the cursor
  tft.setTextColor(BLUE); // set the colour of the text
  tft.setTextSize(5); // set the size of the text
  tft.println("Universum");
  
  tft.setCursor(10, 80); // set the cursor
  tft.setTextColor(CYAN); // set the colour of the text
  tft.setTextSize(3); // set the size of the text
  tft.println("Graphing");

  tft.setCursor(30, 110); // set the cursor
  tft.setTextColor(CYAN); // set the colour of the text
  tft.setTextSize(2); // set the size of the text
  tft.println("History Graphs");
  
  tft.setCursor(10, 160); // set the cursor
  tft.setTextColor(WHITE); // set the colour of the text
  tft.setTextSize(2); // set the size of the text
  tft.println("Andrei Florian");
  delay(4000);

  tft.fillScreen(WHITE);
  delay(500);
}

void drawGraph()
{
  // draw title
  tft.setCursor(10, 10); // set the cursor
  tft.setTextColor(BLUE); // set the colour of the text
  tft.setTextSize(4); // set the size of the text
  tft.println(graphName);
  
  // draw outline
  tft.drawLine(originX, originY, (originX + sizeX), originY, graphColor);
  tft.drawLine(originX, originY, originX, (originY - sizeY), graphColor);

  // draw lables
  for(int i = 0; i < numberOfMarks; i++)
  {
    tft.drawLine(mark[i], originY, mark[i], minorSizeY, graphColor);
  }

  // draw numbers
  for(int i = 0; i < 6; i++)
  {
    tft.drawLine(originX, (originY - number[i]), minorSizeX, (originY - number[i]), graphColor);
  }

  // draw number values
  for(int i = 0; i < 6; i++)
  {
    tft.setCursor((minorSizeX - 30), (number[i] + numberSize));
    tft.setTextColor(graphColor);
    tft.setTextSize(1);
    tft.println(val[i]);
  }
}

void graph()
{
  chk = DHT.read11(22);
  temp = (DHT.temperature);
  timeBlock[valuePos] = ((millis() - 4500) / 1000);

  valueBlock[valuePos] = temp;
  
  if(proDebug)
  {
    Serial.println(timeBlock[valuePos]);
  }
  
  if(blockPos < 8)
  {
    // print the time
    tft.setCursor((mark[valuePos] - 5), (originY + 16));
    tft.setTextColor(graphColor, WHITE);
    tft.setTextSize(1);
    tft.println(timeBlock[valuePos]);
    
    // map the value
    locationBlock[valuePos] = map(temp, 0, graphRange, originY, (originY - sizeY));

    // draw point
    tft.fillRect((mark[valuePos] - 1), (locationBlock[valuePos] - 1), markSize, markSize, pointColor);

    // try connecting to previous point
    if(valuePos != 0)
    {
      tft.drawLine(mark[valuePos], locationBlock[valuePos], mark[(valuePos - 1)], locationBlock[(valuePos - 1)], lineColor);
    }

    blockPos++;
  }
  else
  {
    // clear the graph's canvas
    tft.fillRect((originX + 2), (originY - sizeY), sizeX, sizeY, WHITE);

    // map the value - current point
    locationBlock[valuePos] = map(temp, 0, graphRange, originY, (originY - sizeY));

    // draw point - current point
    tft.fillRect((mark[7]), (locationBlock[valuePos] - 1), markSize, markSize, pointColor);

    // draw all points
    for(int i = 0; i < 8; i++)
    {
      tft.fillRect((mark[(blockPos - (i + 1))] - 1), (locationBlock[(valuePos - i)] - 1), markSize, markSize, pointColor);
    }

    // draw all the lines
    for(int i = 0; i < 7; i++)
    {
      tft.drawLine(mark[blockPos - (i + 1)], locationBlock[valuePos - i], mark[blockPos - (i + 2)], locationBlock[valuePos - (i + 1)], lineColor);
    }
    
    // change time lables
    for(int i = 0; i <= 7; i++)
    {
      tft.setCursor((mark[(7 - i)] - 5), (originY + 16));
      tft.setTextColor(graphColor, WHITE);
      tft.setTextSize(1);
      tft.println(timeBlock[valuePos - i]);
    }
  }

  valuePos++;
}



/*
void calcRateOfChangeLoad(){
  // include code here to calculate rate of change of load on load calibrated in "real-time"
  // should include some security mechanisms if load is jostled somehow--possibly take ten measurements per second, and exclude the top five

  if time_diff = 
  time_0 = millis();
  //scale_0 = scale.get_units(10);
  time_1 = millis();
  scale_1=scale.get_units(10);
  loadSlope=(scale_1-trueWeight)/(time_1-time_0)*(1000)*(60);
  tft.print("\n\n");
  tft.println((time_1-time_0)/1000);
  
}
*/

/*
void screenUpdate_weightgraphs()
void screenUpdate_hematuriagraphs()
*/
