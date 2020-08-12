// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>
// Include dependant SPI Library 
#include <SPI.h> 
 // include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 10, 5, 4, 3, 2);

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

struct dataStruct{
  float send_hgbratio ;
  float send_loadrate;
  float send_load;
}myData;


unsigned long previousMillis=0;
const long interval=5000;
bool showRateNotLoad = true;

void setup()
{
    // Initialize ASK Object
    rf_driver.init();
    // Setup Serial Monitor
    Serial.begin(38400);

    // set up the LCD's number of columns and rows:
  lcd.begin(8, 2);
  // Print a message to the LCD.
  lcd.print("UMonitor");
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  lcd.print("Starting");
  pinMode(6, OUTPUT);    // for yellow led
  pinMode(7, OUTPUT);    // for orange led
  pinMode(8, OUTPUT);    // for red led

  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  
  delay(1500);
}
 
void loop()
{
    
    unsigned long currentMillis=millis();
    if (currentMillis - previousMillis>=interval){
      previousMillis=currentMillis;
    
        if (showRateNotLoad==true){
          showRateNotLoad=false;
        } else {
          showRateNotLoad=true;
        }
    }

    


    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (rf_driver.recv(buf, &buflen)) // Non-blocking
    {
      int i;

      // Message with a good checksum received, dump it.
      
      rf_driver.printBuffer("Got:", buf, buflen);
      memcpy(&myData, buf, sizeof(myData));
      //Serial.println("");
       
      //Serial.print("press_norm: ");
      //Serial.print(myData.press_norm);
      lcd.setCursor(0, 0);

      if(myData.send_hgbratio>=0 && myData.send_hgbratio<=2){
        digitalWrite(6, LOW);
        digitalWrite(7, LOW);
        digitalWrite(8, HIGH);
      }
      else if(myData.send_hgbratio>2 && myData.send_hgbratio<=4){
        digitalWrite(6, LOW);
        digitalWrite(7, HIGH);
        digitalWrite(8, LOW);
      } 
      else {
        digitalWrite(6, HIGH);
        digitalWrite(7, LOW);
        digitalWrite(8, LOW);
      }
      
      if(myData.send_hgbratio>0){
          lcd.print("HgB:");
          lcd.print(myData.send_hgbratio); 
        }
      else{
          lcd.print("Hg:0W/0Y");
        }
       
      
      lcd.setCursor(0, 1);

      if (showRateNotLoad==true){
        if(myData.send_loadrate<0.00){
          lcd.print("0.00");
          lcd.print("L/hr");  
        } else {
          lcd.print(myData.send_loadrate); 
          lcd.print("L/hr");
        }
      } else {
        if(myData.send_load<0.00){
          lcd.print("0.00");
          lcd.print("mL  ");  
        } else {
          lcd.print(myData.send_load); 
          lcd.print("mL  ");
      }
      }
      
      
       
      //Serial.print("  temp: ");
      //Serial.print(myData.temp);
       
      //Serial.print("  counter: ");
      //Serial.println(myData.counter);
    }
    delay(1000);
    
    
}
