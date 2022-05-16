#include <EEPROM.h>
#define tabsize 8
#define maxNoOfOrders 10

int addr=1;
int addrOfsize=0;
int sizeOfEEPROM=0;
int buttonTab[tabsize]={0};
  bool pause=false;
/*
 * 1 - FORWARD 
 * 2 - BACKWARD 
 * 3 - LEFT
 * 4 - RIGHT
 * 5 - START - STARTS THE PROGRAM, reads from orders from EEPROM
 * 6 - RESET  - 
 * 7 - PAUSE 
 */


/*    PINS    */
int inputPin= A5;
int A=9;
int B=13;
int C=8;

int dataPin = 10; //DS or SER ()
int latchPin = 7; //ST_CP or RCLK ()
int clockPin = 5; //SH_CP or SRCLK () 
int clearPin = 4; //MR or SRCLR ()

//Interrupted pins
int interruptPinA= 2;
int interruptPinB = 3;

//MOTOR PINS
//DC  LEFT MOTOR
int breakPinA = 9;
int dirPinA = 12;
int speedPinA = 6;

//DC RIGHT MOTOR
int breakPinB = 8;
int dirPinB = 13;
int speedPinB = 11;

//DEFAULT VALUES
int maxSpeed = 255;
int turnSpeed = 200;
int defaultTimeDuration = 1000;
int defaultTimePause = 1000;
int defaultTimeTurn= 510;

//COUTNERS
volatile int counterMotorA = 0;
volatile int counterMotorB = 0;
int counterTo90Angle;

void setup() {
  
  //Motor pins setup
  pinMode(breakPinA, OUTPUT);
  pinMode(dirPinA, OUTPUT);
  pinMode(speedPinA, OUTPUT);

  pinMode(breakPinB, OUTPUT);
  pinMode(dirPinB, OUTPUT);
  pinMode(speedPinB, OUTPUT);
  
  pinMode(interruptPinA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinA), countMotorA, CHANGE);

  pinMode(interruptPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinB), countMotorB, CHANGE);
  
  Serial.begin(9600);
  
  pinMode(inputPin,INPUT);
  pinMode(A,OUTPUT);
  pinMode(B,OUTPUT);
  pinMode(C,OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clearPin, OUTPUT);

  digitalWrite(clearPin, HIGH);
  digitalWrite(latchPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(dataPin, LOW);

  if (EEPROM.read(addrOfsize)>0) {
      sizeOfEEPROM=EEPROM.read(addrOfsize);
      addr=sizeOfEEPROM;

    }    
}

//    SHIFTREGISTER
//Clears the register
void clearRegister() {
  //Clears the 8-bit register
  digitalWrite(clearPin, HIGH);
  digitalWrite(clearPin, LOW);
  digitalWrite(clearPin, HIGH);

  //Commits the empty register
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
}

//Add a data (0/1) to the 8-bit register
void pushClock(boolean data) {
  //If HIGH, data = 1, if LOW, data = 0
  digitalWrite(dataPin, data);
 
  //Add the data to the 8-bit register 
  digitalWrite(clockPin, LOW);
  digitalWrite(clockPin, HIGH);
  digitalWrite(clockPin, LOW);
}

//Commits the current register
void pushLatch() {
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
}

void writeShiftRegister(int output) {
  //Bring Latch Pin LOW - prepare to commit the register
  digitalWrite(latchPin, LOW);

  //Overwrites the entire register
  shiftOut(dataPin, clockPin, MSBFIRST, output);

  //Bring Latch Pin HIGH - commits the register
  digitalWrite(latchPin, HIGH);
}

//EEPROM 

void clearEEPROM(){

  for(int i=0;i<512;i++){
    EEPROM.write(i,0);
  }
  sizeOfEEPROM=0;
  addr=1;
  EEPROM.write(addrOfsize,0);
}

//LOGIC
int commandNumber(int val[]){

    for(int i=0;i<tabsize;i++){
      if(val[i]==1)
        return i;
    }
    
    return 0;
}

void updateButtonTab(){
  
  for(int i=0;i<tabsize;i++){
    digitalWrite(A,HIGH && (i & B00000001));
    digitalWrite(B,HIGH && (i & B00000010));
    digitalWrite(C,HIGH && (i & B00000100));
    buttonTab[i]=digitalRead(inputPin);

  }
  
}

void turnOnLED(){

  for(int i=0;i<tabsize;i++){
    pushClock(buttonTab[i]);
  }
  pushLatch();
}



void addOrder(int orderNum){

   switch(orderNum){
    case 6:
      clearEEPROM();
      delay(1000);
      break;
    case 5:
       writeShiftRegister(B00000100);
       delay(1000);
       break;
    case 1:
       addOrderToTab(orderNum);
       activeDelay(1000);
       break;
    case 2:
       addOrderToTab(orderNum);
       activeDelay(1000);
       break;
    case 3:
       addOrderToTab(orderNum);
       activeDelay(1000);
       break;
    case 4:
       addOrderToTab(orderNum);
       activeDelay(1000);
       break;
    case 7:
       addOrderToTab(orderNum);
       delay(1000);
       break;

   }
}

void addOrderToTab(int orderNum){
  
  if(orderNum!=0 && sizeOfEEPROM < 10 )
      {
        sizeOfEEPROM+=1;
        EEPROM.write(addrOfsize,sizeOfEEPROM);
        EEPROM.write(addr, orderNum);
        addr+=1;
      }
}

void commitOrders(int orderNum){

     switch(orderNum){
    case 1:
       writeShiftRegister(B11000000);
       goStraightDefault();
       break;
    case 2:
       writeShiftRegister(B10100000);
       goBackwardDefult();
       break;
    case 3:
       writeShiftRegister(B10010000);
       turnLeft90();
       break;
    case 4:
       writeShiftRegister(B10001000);
       turnRight90();
       break;
    case 7:
       writeShiftRegister(B10000001);
       delay(defaultTimePause);
       break;

   }
}

//////////////////////////////////////////////////////////////////////////////////////////
void loop() {
    int orderNum=0;
    int output=0;
    pause=false;
     
    updateButtonTab();
    turnOnLED();
    orderNum=commandNumber(buttonTab);
    
    if(orderNum!=0 && sizeOfEEPROM<maxNoOfOrders){
      addOrder(orderNum);
    }
    else if(orderNum==6){ 
      clearEEPROM();
      delay(1000);
    }
     
     if(orderNum==5){
      int tabLength=10;
      int tempAddr=1;
      activeDelay(1);
    
   
      
      for(int i=0;i<tabLength;i++){ 
 
      orderNum=commandNumber(buttonTab);
       output=EEPROM.read(tempAddr);
        if(output==0)
          break;
      
        if(pause){
          pause_func();
        }
        commitOrders(output);
        activeDelay(1000);
   
        tempAddr+=1;
        
      }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////

//pause 
void pause_func(){
    int orderNum=0;
        do{
            writeShiftRegister(B00000001);
            activeDelay(1);
        } while(pause);

}


void activeDelay(int dt){
      int orderNum=0;
      for(int i=0;i<dt;i++){ 
            updateButtonTab();
            orderNum=commandNumber(buttonTab);
            
            if(orderNum==7){
              pause=true;
               for(int a=i;a<dt;a++){
                delay(1);
               }
              break;
            }
               
            else if(orderNum==5){
              pause=false;
                for(int b=i;b<dt;b++){
                delay(1);
               }
              break;
            }
               
            delay(1);
      }       
}

//MOTOR FUNCTIONS

//PRIVATE
void setDirection(bool dir){
  digitalWrite(dirPinA, dir);
  digitalWrite(dirPinB, dir);
}

void setBreak(bool brk){
  digitalWrite(breakPinA, brk);
  digitalWrite(breakPinB,brk);
}

void setSpd(int spd){
  digitalWrite(speedPinA, spd);
  digitalWrite(speedPinB, spd);
}

void setSpdOneMotor(int spd/*=maxSpeed*/, char channelMotor){
  switch(channelMotor){
    case 'A':
      digitalWrite(speedPinA, spd);
      break;
      
    case 'B':
      digitalWrite(speedPinB, spd);
      break;
    default:
    break;
      
  }
}
//PUBLIC

void goStraight(int spd /*=maxSpeed*/, int timeDuration){
  setBreak(LOW);
  setDirection(HIGH);
  setSpd(spd);
   delay(timeDuration);
  setBreak(HIGH);  
}

void goStraightDefault(){
  setBreak(LOW);
  setDirection(HIGH);
  setSpd(maxSpeed);
   delay(defaultTimeDuration);
  setBreak(HIGH);  
}

void goBackward(int spd/*=maxSpeed*/, int timeDuration){
  setBreak(LOW);
  setDirection(LOW);
  setSpd(spd);
   delay(timeDuration);
  setBreak(HIGH);  
}

void goBackwardDefult(){
  setBreak(LOW);
  setDirection(LOW);
  setSpd(maxSpeed);
   delay(defaultTimeDuration);
  setBreak(HIGH);  
  setDirection(HIGH);
}

void turnLeft90(){

  int curentValCounter = counterMotorA;
  //counterMotorA = 0;
  setDirection(HIGH);
  setBreak(LOW); 
  setSpd(turnSpeed);
  digitalWrite(breakPinB, HIGH); // breaks turned on (on rgiht wheel)
  digitalWrite(dirPinA, HIGH);
 
   delay(defaultTimeTurn);   
  setBreak(HIGH);
  counterTo90Angle = counterMotorA - curentValCounter;
}

void turnLeft90_2(int spd/*=maxSpeed*/){
  counterMotorA = 0;  // to store the coutner value before the turn
  setDirection(HIGH);
  setBreak(LOW); 
  setSpd(turnSpeed);
  
  while(counterMotorA < counterTo90Angle ){
    digitalWrite(breakPinB, HIGH); // breaks turned on (on rgiht wheel)
    digitalWrite(dirPinA, HIGH);
  }   
  setBreak(HIGH);
}

void turnLeftFast(){  // obrót wokół własnej osi
  setDirection(HIGH);
  setBreak(LOW);
  setSpd(maxSpeed);
  digitalWrite(dirPinA, HIGH);
  digitalWrite(dirPinB, LOW);
   delay(100);
  setBreak(HIGH);
}

void turnRight90(){
  //counterMotorB = 0;
  int curentValCounter = counterMotorB;
  setDirection(HIGH);
  setBreak(LOW); 
  setSpd(turnSpeed);
  digitalWrite(breakPinA, HIGH); // breaks turned on (on rgiht wheel)
  digitalWrite(dirPinB, HIGH);
   delay(defaultTimeTurn);   
  setBreak(HIGH);
  counterTo90Angle = counterMotorB - curentValCounter;
}

void turnRight90_2(int spd/*=maxSpeed*/){
  counterMotorB = 0;  // to store the coutner value before the turn
  setDirection(HIGH);
  setBreak(LOW); 
  setSpd(turnSpeed);
  while(counterMotorB < counterTo90Angle ){
    digitalWrite(breakPinA, HIGH); // breaks turned on (on rgiht wheel)
    digitalWrite(dirPinB, HIGH);
  }   
  setBreak(HIGH);
}

void turnRightFast(){ 
  // obrót wokół własnej osi
  setDirection(HIGH);
  setBreak(LOW); 
  setSpd(turnSpeed);
  digitalWrite(dirPinB, HIGH);
  digitalWrite(dirPinA, LOW);
   delay(100);
  setBreak(HIGH);
}

//PRIVATE INTERRUPTED FUNCTIONS

void countMotorA() {
  counterMotorA++;
}

void countMotorB() {
  counterMotorB++;
}
 
