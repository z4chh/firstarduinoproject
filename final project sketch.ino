#include <IRremote.h>       //library for IR remote and receiver
#include <LiquidCrystal.h>  //library for LCD display
#define IR_RECEIVE_PIN 18   //ir sensor data pin should be connected to pin 18

#include <Adafruit_MPU6050.h> //libraries for using the gyro/accel sensor
#include <Adafruit_Sensor.h>
#include <Wire.h>
#define abs(x) ((x)>0?(x):-(x)) //allows for absolute value to be used

float initialXPos = 0;  //defines all the the variables used 
float initialYPos = 0;

Adafruit_MPU6050 mpu;

int delayTime = 300;
int i = 3;
int buzzerPin = 9;
volatile int systemStatus = LOW;
volatile int offButton = LOW;
const int interruptPin = 18;
volatile int beepFrequency = 0;  //end of defined variables used for program

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;   //first 2 are pins 12 and 11 (one space apart on LCD) then four blank spaces, then the next four pins.
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //initialize the LCD display

void interrupt() {  //this is the interrupt function that is called when the interrupt is triggered
  beepFrequency = 0;
  systemStatus = HIGH;
} 

void(* resetFunc) (void) = 0;//declare reset function at address 0  //this defines the reset function that resets the program when called


void clear() {    //this function clears the second line of the LCD display
  lcd.setCursor(0, 1);
  lcd.print("");
}

void activate() {   //this functions shows on the LCD that the alarm is active
  lcd.setCursor(0, 0);
  lcd.print("ALARM ACTIVE     ");
}

void countdown(int i) { //this function contains the countdown on the LCD 
  lcd.setCursor(0, 1);
  lcd.print(i);
  delay(1000);
}

void alarm() {    //this is the function that is called that sounds the alarm until it is turned off with the remote.
  while(true) {
  if(beepFrequency == 0)    //allows the function to be broken when the remote is pressed
  {
    break;
  }

  tone(buzzerPin, beepFrequency);   //these are simply an annoying beeping noise with the buzzer  
  delay(delayTime);
  noTone(buzzerPin);
  delay(delayTime);
  }
}

void setup() {
 // attachInterrupt(digitalPinToInterrupt(21), interrupt, CHANGE);
  IrReceiver.begin(IR_RECEIVE_PIN);   //initializes the ir receiver with the pin it is connected to
  lcd.begin(16, 2);                   //sets the dimensions of the screen for the LCD
  pinMode(8, INPUT);    //this pin is connected to the infared motion sensor
  attachInterrupt(digitalPinToInterrupt(interruptPin), interrupt, FALLING);   //sets the parameters for the interrupt

  delay(10); 
  // Try to initialize!
  if (!mpu.begin()) {   //begins intializing the MPU (aka the gyro sensor)
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");   //confirms that the MPU is connected

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);   //all of these functions are all part of the initlization process for the MPU
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("");  
  delay(100);

  sensors_event_t a, g, temp;   //initializes the ability to pull information from the MPU
  mpu.getEvent(&a, &g, &temp);   //pulls the x and y acceleration from the MPU
  initialXPos = abs(a.acceleration.x);  //calibrates the MPU to the current position
  initialYPos = abs(a.acceleration.y);  //calibrates the MPU to the current position
}
void loop() {
  lcd.setCursor(0, 0);        
  lcd.print("Alarm waiting...");      //shows alarm waiting until it is turned on
  delay(1000);

  if (IrReceiver.decode()) {

    IrReceiver.resume();
    if(IrReceiver.decodedIRData.command==69) {    //if the power button on the remote is pressed, the following will happen
        activate();     //shows on the screen that the alarm is turning on
      for(i = 5 ; i > 0 ; i--) {
        countdown(i);   //counts down from 5 before activating
      }
      lcd.setCursor(0, 1);   
      lcd.print(" ");   //clears the LCD
      systemStatus = LOW;  
      beepFrequency = 100;    //sets the frequency for the buzzer
    }

    while (systemStatus == LOW) {   //while the system is turned on do the following
        sensors_event_t a, g, temp;   
        mpu.getEvent(&a, &g, &temp);  //pulls the x and y acceleration from the MPU
        delay(100);
      if(abs(a.acceleration.x) > (initialXPos + 0.5) || abs(a.acceleration.y) > (initialYPos + 0.5)) {   //if the detected acceleration more than the resting calibrated acceleration do the following
        alarm();  //sounds the alarm

      }
    }
    if (systemStatus == HIGH) {
      lcd.setCursor(0, 0);
      lcd.print("ALARM OFF       ");  //shows that the alarm has been turned off on the screen
      delay(5000);   
      resetFunc();    //resets the whole functions
  }
  }

}