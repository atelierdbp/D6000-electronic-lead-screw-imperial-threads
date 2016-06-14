 /* Plewes Electronic Lead Screw (PELS) threading code version PELS1.0. This code drives an Arduino electronic lead screw and permits turning and imperial threads. 
The imperial threads are based on a 635 pulse/spindle-rev encoder with a quad boast of 4 to give a total of 2540 pulses per revolution specific to a Wabeco D6000 lathe.
This is achieved by using a 1024 quadrature encoder with a speed reduction through the use of Wabeco pulleys which are supplied with the lathe for normal spindle speed changes.  
In order to achieve this reduction one places pulleys in the various positions 'C1','A' and 'B' as defined in the Wabeco user manual.  To get the correct reduction
one uses a 14 tooth pulley in position 'C1', a 16 tooth pulley in position 'A', a 34 tooth pulley in position 'B' and a 36 tooth pulley on the optical encoder. 
This allows simple division by integers for all imperial threads of interest. The divisor used equals the thread per inch value desired.  For example, a 24 tpi thread
will use a divisor of 24 and so forth. Selecting the parameters for turning or threading is achieved by a push-button on the knob rotary encoder switch.  
In 'prog' mode turning the knob changes the parameters for the thread or turning parameters. Pushing the knob button again loads these into the divisor algorithm for lathe operation.
This ensures that the parameters cannot change during lathe operation. All the thread parameters are put into one big menu file which includes: Turning and Imperial threads.
The inputs to the Arduino which are needed are as follows:
  - knob rotary encoder - Q to Arduino pin 4, I to Arduino pin 5, knob push-button to Arduino pin 6. 
  - spindle rotary encoder I and Q pulses goes to a LS7184 chip. 
  - Output from pin 7 of LS7184 goes to Arduino pin 2. 
  - The direction level from pin 8 of LS7184 goes directly to the stepper motor control direction input.
  - THe step input of the stepper motor controller comes from Arduino pin 13. 
  - Output from Arduino pin 7 goes to a voltage divider which is connected to pin 6 of the LS7184 chip to changes it "mode". 
  
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
details. You can read a copy of the GNU General Public License at <http://www.gnu.org/licenses/>
  */  

#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
 Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
 int stepper_pin=13;                    //the output pin for the stepper pin for the motor driver
 int encoder0PinA = 4;                  //the input pin for knob rotary encoder 'I' input
 int encoder0PinB = 5;                  //the input pin for knob rotary encoder 'Q' input
 int buttonPin=6;                       // the button for the knob rotary encoder push button line
 int range_select_pin=7;                // the pin for the output to drive the LS7184 from mode 0 to mode 2
 int menu = 1;                          // the parameter for the menu select
 int tpi=128;                           //a parameter to indicate the number of threads/inch
 int encoder0PinALast = LOW;            //a dummy parameter for the rotary encoder quad select algorithm
 int n = HIGH;                          //another dummy parameter for the rotary encoder quad select algorithm
 int state=HIGH;
 int previous=LOW;
 int reading=HIGH;
 int newButtonState = 0;              //a parameter for the button push algorithm
 int oldButtonState=0;                //a parameter for the button push algorithm
 int mode_select=1;                   // a parameter to define the programming versus operation settings
 volatile long input_counter=0;       //a parameter for the interrupt to count input pulses
 volatile long output_counter=0;
 volatile int divisor=128;       //defines the divisor for the initial condition which is turning-normal (divisor 128)


 void setup() { 
   pinMode (encoder0PinA,INPUT);        //input for the Q channel of the switch rotary encoder
   pinMode (encoder0PinB,INPUT);        //input for the I channel of the switch rotary encoder
   pinMode (buttonPin,INPUT_PULLUP);    //input for the button of the switch rotary encoder
   pinMode (range_select_pin,OUTPUT);   // set up digital pin "range_select_pin" to digital output to change the state of the LS7184 quad chip with HIGH giving 4x pulse number and LOW giving 1x pulse number
   lcd.begin(16,2);
   pinMode(stepper_pin, OUTPUT);        
   attachInterrupt(0, count, RISING);  // enable the interrupt for Arduino pin 2 which is interrupt "0"
  //This next section starts the system to Nnormal Imperial Turning 
   lcd.setCursor(0,0);
   lcd.print("Imperial Turning");
   lcd.setCursor(0,1);
   lcd.print("Normal ");
        } 
    
   
void button()          //detects when the rotary switch button is pushed and toggles between either programming or locked states
 {
    newButtonState = digitalRead(buttonPin);       // Get the current state of the button
      if (newButtonState == HIGH && oldButtonState == LOW) {    // Has the button gone high since we last read it?
        mode_select=1-mode_select;}
        oldButtonState = newButtonState;
         if (mode_select == 0)         {       //mode_select==0 for lathe operatoin
         lcd.setCursor(11,1);
         lcd.print("lathe");
            }
          else {
          lcd.setCursor(11,1);                // mode_select==1 for parameter programming
          lcd.print(" prog");
           }
      oldButtonState = newButtonState;          
         }
    
 void count()                            //this is the interrupt routine for the division algorithm
  {
    input_counter++;
    if(input_counter%divisor==0)        //if true deliver a pulse every 'divisor' pulses
   {
    digitalWrite(stepper_pin,HIGH);
    delayMicroseconds(10);                                      // gives a 10 microsecond pulse  on the stepper_pin
    digitalWrite(stepper_pin,LOW);
    output_counter++;                                          // incremented the output_counter for each output pulse.  Useful for trouble shooting but not actually needed
    }
    }  

 void thread_parameters()                                           //this defines the parameters for the thread and turning for both metric and imperial threads
 { 
  if(mode_select==1)
          {
           n = digitalRead(encoder0PinA);                                        //Selecting the Thread and Turning Parameters
                 if ((encoder0PinALast == LOW) && (n == HIGH)) {                 //true if button got pushed?
                       if (digitalRead(encoder0PinB) == LOW) {                   //this is the quadrature routine for the rotary encoder
                       menu++;
                 } else {
                       menu--;
                 }
                 
                  if(menu>17){                                      //the next four lines allows the rotary select to go around the menu as a loop in either direction
                             menu=1;
                             }
                  if(menu<1){
                             menu=17;
                             }
                                 switch(menu) {
                                             case(1):     divisor=128;       tpi=64;      break;  // Normal Turning
                                             case(2):     divisor=166;       tpi=116;     break;  // Fine Turning
                                             case(3):     divisor=64;        tpi=32;      break;  // Course Turning
                                             case(4):     divisor=10;        tpi=10;      break;  
                                             case(5):     divisor=11;        tpi=11;      break;
                                             case(6):     divisor=12;        tpi=12;      break;
                                             case(7):     divisor=13;        tpi=13;      break;
                                             case(8):     divisor=16;        tpi=16;      break;
                                             case(9):     divisor=20;        tpi=20;      break;
                                             case(10):    divisor=24;        tpi=24;      break;
                                             case(11):    divisor=28;        tpi=28;      break;
                                             case(12):    divisor=32;        tpi=32;      break;
                                             case(13):    divisor=36;        tpi=36;      break;
                                             case(14):    divisor=42;        tpi=42;      break;
                                             case(15):    divisor=44;        tpi=44;      break;
                                             case(16):    divisor=48;        tpi=48;      break;
                                             case(17):    divisor=52;        tpi=52;      break;
                                             
                                             }
                                                     if(menu<4){
                                                           digitalWrite(range_select_pin,LOW);
                                                     switch(menu) {
                                                           case(1):     lcd.setCursor(0,0);     lcd.print("Turning         ");     lcd.setCursor(0,1);     lcd.print("Normal ");     break;
                                                           case(2):     lcd.setCursor(0,0);     lcd.print("Turning         ");     lcd.setCursor(0,1);     lcd.print("Fine   ");     break;
                                                           case(3):     lcd.setCursor(0,0);     lcd.print("Turning         ");     lcd.setCursor(0,1);     lcd.print("Coarse ");     break;
                                                                   }}
                                                     else {
                                                           digitalWrite(range_select_pin,HIGH);
                                                      {
                                                   lcd.setCursor(0,0);     lcd.print("Imperial thread ");     lcd.setCursor(0,1);     lcd.print(tpi);    lcd.print(" tpi ");
                                            
                                             }}} 
                  encoder0PinALast = n; 
             
 }
 }
 
  
   void loop()
   {
   button();
   thread_parameters();
 //  lcd.setCursor(9,0);                //next three lines checks the output of the divisor against the input delivered running the lathe for a brief period
 // lcd.print(input_counter);           //we measure the number of input pulses as the parameter input_counter 
 //  lcd.setCursor(9,1);
 //  lcd.print(output_counter);        //we measure the number of output pulses from the parameter output_counter.  The ratio input/output should equal 'divisor' chosen
   }
   

 
