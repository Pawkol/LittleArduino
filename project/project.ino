#include "LedControl.h"

LedControl lc=LedControl(12,11,10,1);  // Pins: DIN,CLK,CS, # of Display connected

/*-------------- Spinning control data ---------------*/

unsigned long delayTime=100;  // Delay between Frames
boolean running=true; // If windmill should stop
boolean right=false; // Spinning in right, when false spinning in left
//boolean velocityChanging=true; // If velocity is changing to input read
unsigned short velocity=0;  // Rotation velocity (rounds per 10s)
unsigned short rounds=40; // Number of rounds left
unsigned short roundsInput=4;
unsigned short velocityChange=100;
unsigned short velocityTarget=100;
unsigned short velocityStep=1;
unsigned short framesNumber=12;
unsigned short frame=0;
unsigned short velocityToFlush=0;
unsigned short roundsToFlush=0;

/*--------- Windmill animation frames data ----------- */

byte a1[]=
{
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00011000
};

byte a2[]=
{
    B00100000,
    B00010000,
    B00010000,
    B00011000,
    B00011000,
    B00001000,
    B00001000,
    B00000100
};

byte a3[]=
{
    B01000000,
    B00100000,
    B00100000,
    B00011000,
    B00011000,
    B00000100,
    B00000100,
    B00000010
};

byte a4[]=
{
    B10000000,
    B01000000,
    B00100000,
    B00011000,
    B00011000,
    B00000100,
    B00000010,
    B00000001
};

byte a5[]=
{
    B00000000,
    B10000000,
    B01100000,
    B00011000,
    B00011000,
    B00000110,
    B00000001,
    B00000000
};

byte a6[]=
{
    B00000000,
    B00000000,
    B10000000,
    B01111000,
    B00011110,
    B00000001,
    B00000000,
    B00000000
};

byte a7[]=
{
    B00000000,
    B00000000,
    B00000000,
    B11111111,
    B11111111,
    B00000000,
    B00000000,
    B00000000
};

byte a8[]=
{
    B00000000,
    B00000000,
    B00000001,
    B00011110,
    B01111000,
    B10000000,
    B00000000,
    B00000000
};

byte a9[]=
{
    B00000000,
    B00000001,
    B00000110,
    B00011000,
    B00011000,
    B01100000,
    B10000000,
    B00000000
};

byte a10[]=
{
    B00000001,
    B00000010,
    B00000100,
    B00011000,
    B00011000,
    B00100000,
    B01000000,
    B10000000
};

byte a11[]=
{
    B00000010,
    B00000100,
    B00000100,
    B00011000,
    B00011000,
    B00100000,
    B00100000,
    B01000000
};

byte a12[]=
{
    B00000100,
    B00001000,
    B00001000,
    B00011000,
    B00011000,
    B00010000,
    B00010000,
    B00100000
};

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
/*
byte* framesR1[]=
{
  windmill0,
  windmill1,
  windmill2,
  windmill3
};

byte* framesL1[]=
{
  windmill0,
  windmill4,
  windmill2,
  windmill5
};
*/
byte* framesR[]=
{
  a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12
};

byte* framesL[]=
{
  a1,a12,a11,a10,a9,a8,a7,a6,a5,a4,a3,a2
};

/*-----------Functions for parsing----------------*/

/**
 * Check if sended string is correct formated integer value representation.
 */
boolean checkIfNumber(String lineToCheck) {
   for(byte i = 0; i < lineToCheck.length(); ++i)
   {
     Serial.print("Dostałem ");
     Serial.print(lineToCheck.charAt(i));
     if(lineToCheck.charAt(i) < '0' || lineToCheck.charAt(i) > '9')
      return false;
   }
   return true;
}

/**
 * Sets changes derivered earlier by serial input.
 */
void flushNumbers() {
  velocityTarget=velocityToFlush;
  roundsInput=roundsToFlush;
}

/**
 * Function parse serial String line derived in argument and introduce sended
 * changes in case of wrong input format informs user using serial. Serial line
 * must be send without line ending character.
*/
void parseInput(String input) {
  if(input[0]=='P')
  {
    if(input[1]=='1')
    {
      Serial.print("Starting spinning...\n");
      velocityChange=velocityTarget;
      running=true;
    }
    if(input[1]=='0')
    {
      Serial.print("Stopping spinning...\n");
      velocityChange=0;
      running=false;
    }
    else
    {
      Serial.print("Wrong input, you can send P0 or P1\n");
    }
  }
  else if(input[0]=='R'&&input[1]=='E'&&input[2]=='V')
  {
    if(input.length() >= 6 && checkIfNumber(input.substring(3)))
    {
      velocityToFlush=input.substring(3).toInt();
      Serial.print("Input velocity ");
      Serial.println(input.substring(3));
    }
    else
    {
      Serial.println("Wrong input format, must be REV..., where ... are all digits");
    }
  }
  else if(input[0]=='N')
  {
    if(input.length() == 3 && checkIfNumber(input.substring(1)))
    {
      roundsToFlush=input.substring(1).toInt();
      Serial.print("Input rounds ");
      Serial.println(input.substring(1));
    }
    else
    {
      Serial.println("Wrong input format, must be N.., where .. are all digits");
    }
  }
  else if(input[0]<='S')
  {
    flushNumbers();
    Serial.println("Flushing...");
  }
  else
  {
    Serial.print("Incorrect command:");
    Serial.println(  roundsToFlush);
    Serial.println("  Correct command are:");
    Serial.println("  REV..., where ... - digits");
    Serial.println("  N.., where .. - digits");
    Serial.println("  P., where . is 0 or 1");
    Serial.println("  S");
  }
}

/*-----------Functions for animation and loop----------------*/

//  Take values in Arrays and Display them
void setDisplay(byte img[]){
  for (int i = 0; i < 8; i++)
  {
    lc.setRow(0,i,img[i]);
  }
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
  lc.setIntensity(0,2);  // Set intensity levels
  lc.clearDisplay(0);  // Clear Displays
}

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
