/****************************************************/
/* This is only one example of code structure       */
/* OFFCOURSE this code can be optimized, but        */
/* the idea is let it so simple to be easy catch    */
/* where can do changes and look to the results     */
/****************************************************/

// Standard Input/Output functions 1284
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/atomic.h>
#include <stdbool.h>
#include "stdint.h"
#include <Arduino.h>

#define UCN5821A_in 8   // If 0 write LCD, if 1 read of LCD
#define UCN5821A_clk 9  // if 0 is a command, if 1 is a data0
#define UCN5821A_stb 10  // Must be pulsed to LCD fetch data of bus

#define AdjustPins    PIND // Use all Port D, Is possible use only some pins! Look inside of function adjustHMS() to redefine it!

/*Global Variables Declarations*/
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char minute = 0;
unsigned char secs=0;
unsigned char seconds=0;
unsigned char milisec = 0;

uint8_t digitSu = 0x00;
uint8_t digitSd = 0x00;
uint8_t digitMu = 0x00;
uint8_t digitMd = 0x00;
uint8_t digitHu = 0x00;
uint8_t digitHd = 0x00;

uint8_t numSecsU;
uint8_t numSecsD;
uint8_t numMinuU;
uint8_t numMinuD;
uint8_t numHourU;
uint8_t numHourD;

unsigned char digit=0;
unsigned char grid=0;

boolean flag=true;
boolean flagSecs=false; // Used in case of using buttons to setting clock, set buttons to do it!

unsigned int wd0 =0x00;
unsigned int wd1 =0x00;

//Arrays of bits to Digits They respect standard order of 7 segments display HGFEDCBA
uint8_t numbers[10]={0x5F,0x06,0x3B,0x2F,0x66,0x6D,0x7D,0x07,0x7F,0x6F}; 

void setup() {
    // initialize digital pin LED_BUILTIN as an output.
      pinMode(LED_BUILTIN, OUTPUT);
      Serial.begin(115200);
      seconds = 0x00;
      minutes =0x00;
      hours = 0x00;

      /*CS12  CS11 CS10 DESCRIPTION
      0        0     0  Timer/Counter1 Disabled 
      0        0     1  No Prescaling
      0        1     0  Clock / 8
      0        1     1  Clock / 64
      1        0     0  Clock / 256
      1        0     1  Clock / 1024
      1        1     0  External clock source on T1 pin, Clock on Falling edge
      1        1     1  External clock source on T1 pin, Clock on rising edge
    */
      // initialize timer1 
      cli();           // disable all interrupts
      //initialize timer1 
      //noInterrupts();    // disable all interrupts, same as CLI();
      TCCR1A = 0;
      TCCR1B = 0;// This initialisations is very important, to have sure the trigger take place!!!
      
      TCNT1  = 0;
      
      // Use 62499 to generate a cycle of 1 sex 2 X 0.5 Secs (16MHz / (2*256*(1+62449) = 0.5
      //Comment next line and uncomment the other line follow to get a count of seconds more fast to effect of test!
      OCR1A = 62498;            // compare match register 16MHz/256/2Hz
      //OCR1A = 1200; // only to use in test, increment seconds more fast!
      TCCR1B |= (1 << WGM12);   // CTC mode
      TCCR1B |= ((1 << CS12) | (0 << CS11) | (0 << CS10));    // 256 prescaler 
      TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

    // Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
    // a little the value 62499 upper or lower if the clock have a delay or advance on hours.
      
    //  a=0x33;
    //  b=0x01;

    CLKPR=(0x80);
    //Set PORT
    DDRD = 0xFF;  // IMPORTANT: from pin 0 to 7 is port D, from pin 8 to 13 is port B
    PORTD=0x00;
    DDRB =0xFF;
    PORTB =0x00;
    //only here I active the enable of interrupts to allow run the test of UCN5821A
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);

    //interrupts();             // enable all interrupts, is same as sei();
    sei();
}
void cmd_without_stb(unsigned char a){
  // send without stb
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  data=a;
  //This don't send the strobe signal, to be used in burst data send
         for (mask = 0b10000000; mask>0; mask >>= 1) { //iterate through bit mask
           digitalWrite(UCN5821A_clk, LOW);
                 if (data & mask){ // if bitwise AND resolves to true
                    digitalWrite(UCN5821A_in, HIGH);
                 }
                 else{ //if bitwise and resolves to false
                   digitalWrite(UCN5821A_in, LOW);
                 }
          delayMicroseconds(5);
          digitalWrite(UCN5821A_clk, HIGH);
          delayMicroseconds(5);
         }
   digitalWrite(UCN5821A_clk, LOW);
}
void test_AllOff(void){
   digitalWrite(UCN5821A_stb, LOW);
                          
  cmd_without_stb(0b11111111); // 
  cmd_without_stb(0b11111111); // 
                            
  digitalWrite(UCN5821A_stb, HIGH);                          
}
void test_AllOn(void){
   digitalWrite(UCN5821A_stb, LOW);
                          
  cmd_without_stb(0b00000000); // 
  cmd_without_stb(0b00000000); // 
                            
  digitalWrite(UCN5821A_stb, HIGH);
                            
}
void number0(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b01011111; //0x5F

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number1(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b00000110; //0x06

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number2(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b00111011; //0x3B

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number3(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b00101111; //0x2F

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number4(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b01100110; //0x66

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number5(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b01101101; //0x6D

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number6(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b01111101; //0x7D

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number7(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b00000111; //0x07

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number8(void){
  uint8_t byteL = 0x00;
  uint8_t byteH = 0x00;
  //........76543210
  byteL = 0b00000001;
  //........hfgedcba 
  byteH = 0b01111111; //0x7F

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void number9(void){
 uint8_t byteL = 0x00;
 uint8_t byteH = 0x00;
 //........76543210
 byteL = 0b00000001;
 //........hfgedcba 
 byteH = 0b01100111; //0x67

  byteL = ~byteL;
  byteH = ~byteH;
  Serial.println(byteL, BIN);
  Serial.println(byteH, BIN);

  digitalWrite(UCN5821A_stb, LOW);        
  cmd_without_stb(byteH);   
  cmd_without_stb(byteL);                        
  digitalWrite(UCN5821A_stb, HIGH);                         
}
void showDigits(uint8_t grid, uint8_t digit){
  uint8_t byteL = 0x00;
  uint8_t byteH = 0x00;
  //.........76543210
  byteL = (0b00000000 | grid);
  //The following two commented lines are only to make the code easier to understand!
  //...........hfgedcba.........................//
  //byteH = (0b00000000 | digit);...............//
  byteH = digit;
  byteL = ~byteL;
  byteH = ~byteH;
  digitalWrite(UCN5821A_stb, LOW);
  cmd_without_stb(byteH); // 
  cmd_without_stb(byteL); //                        
  digitalWrite(UCN5821A_stb, HIGH);                          
}
void showNumbers(void){
    number0();
    delay(250);
    number1();
    delay(250);
    number2();
    delay(250);
    number3();
    delay(250);
    number4();
    delay(250);
    number5();
    delay(250);
    number6();
    delay(250);
    number7();
    delay(250);
    number8();
    delay(250);
    number9();
    delay(250);
}
void send_update_clock(void){
  if (secs >=60){
    secs =0;
    minutes++;
  }
  if (minutes >=60){
    minutes =0;
    hours++;
  }
  if (hours >=24){
    hours =0;
  }
    //*************************************************************
    digitSu = (secs%10);
    numSecsU=numbers[digitSu];
    digitSd = (secs/10);
    numSecsD=numbers[digitSd];
    //*************************************************************
    digitMu = (minutes%10);
    numMinuU=numbers[digitMu];
    digitMd = (minutes/10);
    numMinuD=numbers[digitMd];
    //**************************************************************
    digitHu = (hours%10);
    numHourU=numbers[digitHu];
    digitHd = (hours/10);
    numHourD=numbers[digitHd];
    //**************************************************************
    SegTo32Bits(); // This is to send the total of digits to VFD
}
void SegTo32Bits(){
          //Secondes Block
          showDigits(0x01, numSecsU);
          showDigits(0x02, numSecsD);
          //Minuts Block:
          showDigits(0x08, numMinuU);
          showDigits(0x10, numMinuD); 
          //Hours Block:
          showDigits(0x40, numHourU); 
          showDigits(0x80, numHourD); 
          // Serial.print(numSecsD, DEC); Serial.print(", ");   Serial.println(numSecsU, DEC); 
          // Serial.print(numMinuD, DEC); Serial.print(", ");   Serial.println(numMinuU, DEC); 
          // Serial.print(numHourD, DEC); Serial.print(", ");   Serial.println(numHourU, DEC); 
          // Serial.println(".....");
}
void adjustHMS(){
  //This function implement buttons to set the Clock, case the panel you are
  //using don't have buttons!
  //Case the position of buttons belongs to panel are with different positions
  //bit inside of the byte you can modify it on the function readButtons();
  // Important is necessary put a pull-up resistor to the VCC(+5VDC) to this pins (3, 4, 5)
  //pinMode(3, INPUT_PULLUP); //This line must be placed inside of initial SETUP
  //pinMode(4, INPUT_PULLUP); //This line must be placed inside of initial SETUP
  //pinMode(5, INPUT_PULLUP); //This line must be placed inside of initial SETUP
  // if dont want adjust of the time comment the call of function on the loop
  /* Reset Seconds to 00 Pin number 3 Switch to GND*/
    if((AdjustPins & 0x08) == 0 ){  // Pay attention to the weigth of bit, and position at word of 4 bit: 1 Byte = 0x84218421
      _delay_ms(100);
      secs=00;
    }
    
    /* Set Minutes when SegCntrl Pin 4 Switch is Pressed*/
    if((AdjustPins & 0x10) == 0 ){  // Pay attention to the weigth of bit, and position at word of 4 bit: 1 Byte = 0x84218421
      _delay_ms(100);
      if(minutes < 59)
      minutes++;
      else
      minutes = 0;
    }
    /* Set Hours when SegCntrl Pin 5 Switch is Pressed*/
    if((AdjustPins & 0x20) == 0 ){  // Pay attention to the weigth of bit, and position at word of 4 bit: 1 Byte = 0x84218421
      _delay_ms(100);
      if(hours < 23)
      hours++;
      else
      hours = 0;
    }
}
void loop() {
  uint8_t n = 0x00;
  for(uint8_t v = 0x00; v < 1; v++){
    test_AllOn();
    delay(500);
    test_AllOff();
    delay(500);
    showNumbers();
  }
    
    
    //
        for(uint8_t d = 0x80; d > 0; d = d >> 1){
          showDigits(d, numbers[n=n+1]); //Here I use a "n", because the array numbers[] is constructed with hte swap of segments as mirror of VFD: hfgedcba 
          delay(800);  //Set to value of 2 will not showed the blink!
        }
   // The code will be nailed on this cycle while to allow the dynamic representation of digits.
    while(1){
      send_update_clock();
      adjustHMS();   
    } 
 }
ISR(TIMER1_COMPA_vect)   {  //This is the interrupt request
// https://avr-guide.github.io/timers-on-the-atmega328/
      secs++;
      flag = !flag;
} 
