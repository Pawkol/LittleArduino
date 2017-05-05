#include "LedControl.h"

LedControl lc=LedControl(12,11,10,1);  // Pins: DIN,CLK,CS, # of Display connected

unsigned long delayTime=100;  // Delay between Frames
boolean running=true; // If windmill should stop
boolean right=false; // Spinning in right, when false spinning in left
unsigned short velocity=0;  // Rotation velocity (rounds per 10s)
unsigned short rounds=4; // Number of rounds left
unsigned short roundsInput=4;
unsigned short velocityChange=10;
unsigned short velocityTarget=10;
unsigned short velocityStep=1;
unsigned short framesNumber=4;
unsigned short frame=0;
unsigned short velocityToFlush=0;
unsigned short roundsToFlush=0;

// Windmill animation frames

byte windmill0[]=
{
    B00011000,
    B00011000,
    B00011000,
    B11100111,
    B11100111,
    B00011000,
    B00011000,
    B00011000
};

byte windmill1[]=
{
    B00001100,
    B00011100,
    B11011000,
    B11100110,
    B01100111,
    B00011011,
    B00111000,
    B00110000
};

byte windmill2[]=
{
    B11000011,
    B11100111,
    B01111110,
    B00100100,
    B00100100,
    B01111110,
    B11100111,
    B11000011
};

byte windmill3[]=
{
    B01100000,
    B01100111,
    B01111111,
    B00100100,
    B00100100,
    B11111110,
    B11100110,
    B00000110
};

byte windmill4[]=
{
    B00110000,
    B00111000,
    B00011011,
    B01100111,
    B11100110,
    B11011000,
    B00011100,
    B00001100
};

byte windmill5[]=
{
    B00000110,
    B11100110,
    B11111110,
    B00100100,
    B00100100,
    B01111111,
    B01100111,
    B01100000
};

byte* framesR[]=
{
  windmill0,
  windmill1,
  windmill2,
  windmill3
};

byte* framesL[]=
{
  windmill0,
  windmill4,
  windmill2,
  windmill5
};






//  Take values in Arrays and Display them
void setDisplay(byte img[]){
  for (int i = 0; i < 8; i++)
  {
    lc.setRow(0,i,img[i]);
  }
}

void parseInput(String input){
  if(input[0]=='P'){
    if(input[1]=='1'){
      Serial.print("Starting spinning...\n");
      velocityChange=velocityTarget;
      running=true;
    }
    if(input[1]=='0'){
      Serial.print("Stopping spinning...\n");
      velocityChange=0;
      running=false;
    }
  }else if(input[0]=='R'&&input[1]=='E'&&input[2]=='V'){
    velocityToFlush=input.substring(3).toInt();
    Serial.print("Input velocity ");
    Serial.println(velocityToFlush);
  }else if(input[0]=='N'){
    roundsToFlush=input.substring(1).toInt();
    Serial.print("Input rounds ");
    Serial.println(roundsToFlush);
  }else if(input[0]=='S'){
    flushNumbers();
    Serial.println("Flushing...");
  }
}

void flushNumbers(){
  velocityTarget=velocityToFlush;
  roundsInput=roundsToFlush;
}

void nextFrame(){
  if(velocity!=0){
    if(frame==framesNumber-1){
      frame=0;
      if(velocity==velocityChange){
        rounds--;
      }
    }else{
      frame++;
    }
  }
}

void changeDirection(){
  if(rounds==0){
    // Stop if no more rounds to go
    velocityChange=0;
    if(velocity==0&&running){
      // Change direction of spin
      velocityChange=velocityTarget;
      rounds=roundsInput;
      right=!right;
    }
  }
}

void changeDelay(){
  if(velocity!=0){
    unsigned long newDelay;
    newDelay=(unsigned long)1000/velocity*framesNumber;
    delayTime=newDelay;
    
  }
}

void changeVelocity(){
  if(velocity>velocityChange){
    // Slow down
    velocity=velocity-velocityStep;
  }else if(velocity<velocityChange){
    // Speed up
    velocity=velocity+velocityStep;
  }
}

void dispFrame(){
  if(right){
    setDisplay(framesR[frame]);
  }else{
    setDisplay(framesL[frame]);
  }
  
  delay(delayTime);
}

void setup()
{
  Serial.begin(9600); 
  lc.shutdown(0,false);  // Wake up displays
  lc.setIntensity(0,5);  // Set intensity levels
  lc.clearDisplay(0);  // Clear Displays
}

/**
 * Idea for final loop:
 * 1. Parse Input - set integer variables and boolean variable if user 
 *    sended S, or decidef to start or stop spinning, if user decided to
 *    stop spinning, set number of spins to top and boolean variable for stop
 *    spinning if user decided to start spinning, then set variable and set
 *    number of spins to zero
 * 2. Start spinning - if user decided to stop spinning or sended S, then
 *    do nothing, else looking for direction and number of spins, call function 
 *    startSpining(), where program set velocity to top level with acceleration 
 *    effect
 * 3. If number of spins is enough little and user does not sended S or decided 
 *    to stop, call function oneSpin, with change number of done spins
 * 4. If number of spins is equal to top number of spins or user sended
 *    S or decided to stop spinning, then call function stopSpinning(), 
 *    with change number of spins to zero
 */
void loop()
{
  
  //Windmill animation
  if (Serial.available() > 0) {
    String input=Serial.readString();
    parseInput(input);
    
  }
  
  dispFrame();
  

  changeVelocity();

  
  changeDelay();
  
  
  changeDirection();
  

  nextFrame();
  
  /*Serial.print("velo");
  Serial.print(velocity);
  Serial.print("\n");
  
  Serial.print("delayTime");
  Serial.print(delayTime);
  Serial.print("\n");*/


}
