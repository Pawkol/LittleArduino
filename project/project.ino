#include "LedControl.h"
#define REV 100
#define TIMES 10
#define FRAMES_NUMBER 4
#define VELOCITY_STEP 10

/*-------------- Spinning control data ---------------*/

LedControl lc = LedControl(12,11,10,1);  // Pins: DIN,CLK,CS of Display connected
unsigned short delayTime = 1000;  // Initial delay between Frames
boolean running = true; // If windmill should stop
boolean right = false; // Spinning in right, when false spinning in left
unsigned short velocity = 0;  // Rotation velocity (rounds per 10s)
unsigned short rounds = TIMES; // Initial number of rounds
unsigned short roundsInput = TIMES; // Initial number of rounds from input
unsigned short velocityChange = REV; // Initial value of variable for velocity windmill current is going to reach
unsigned short velocityTarget = REV; // Initial value of top value of velocity
unsigned short velocityToFlush = REV; // Initial value of variable holds user decision about top velocity
unsigned short roundsToFlush = TIMES; // Initial value of variable holds user decision about number of rounds

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
/*
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
*/
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
/*
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
*/
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
/*
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
*/
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
/*
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
*/
byte* frames[]=
{
  a1,a4,a7,a10 //a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12
};

/*-----------Functions for parsing----------------*/

/**
 * Check if sended string is correct formated integer value representation.
 */
boolean checkIfNumber(String lineToCheck) {
   for(byte i = 0; i < lineToCheck.length(); ++i)
     if(lineToCheck.charAt(i) < '0' || lineToCheck.charAt(i) > '9')
      return false;
   return true;
}

/**
 * Function parse serial String line derived in argument and introduce sended
 * changes in case of wrong input format informs user using serial. Serial line
 * must be send without line ending character.
*/
void parseInput(String input) {
  if(input[0]=='P') {
    if(input[1]=='1') {
      Serial.print("Starting spinning...\n");
      rounds = roundsInput;
      velocityChange=velocityTarget;
      running=true;
    }
    else if(input[1]=='0') {
      Serial.print("Stopping spinning...\n");
      velocityChange=0;
      running=false;
    }
    else {
      Serial.print("Wrong input, you can send P0 or P1\n");
    }
  }
  else if(input[0]=='R'&&input[1]=='E'&&input[2]=='V') {
    if(input.length() >= 6 && checkIfNumber(input.substring(3,6))) {
      velocityToFlush=input.substring(3).toInt();
      Serial.print("Input velocity ");
      Serial.println(input.substring(3,6));
    }
    else {
      Serial.println("Wrong input format, must be REV..., where ... are all digits");
    }
  }
  else if(input[0]=='N') {
    if(input.length() >= 3 && checkIfNumber(input.substring(1,4))) {
      roundsToFlush=input.substring(1).toInt();
      Serial.print("Input rounds ");
      Serial.println(input.substring(1,4));
    }
    else {
      Serial.println("Wrong input format, must be N.., where .. are all digits");
    }
  }
  else if(input.length() == 1 && input[0]=='S') {
    if(velocityToFlush > 990)
      velocityToFlush = 990;
    velocityTarget=velocityToFlush;
    roundsInput=roundsToFlush;
    setup();
  }
  else {
    Serial.print("Incorrect command:");
    Serial.println(  roundsToFlush);
    Serial.println("  Correct commands are:");
    Serial.println("  REV..., where ... - digits");
    Serial.println("  N.., where .. - digits");
    Serial.println("  P., where . is 0 or 1");
    Serial.println("  S");
  }
}

/*-----------Functions for animation and loop----------------*/

/**
 * Take values in Arrays and Display them.
 */
void setDisplay(byte img[]) {
  for(int i = 0; i < 8; i++) {
    lc.setRow(0,i,img[i]);
  }
}

/**
 * Proceed one frame of animation using direction variable
 * to specify good table of LED's states, after sets delay
 * time using variable delayTime.
 */
void dispFrame(short i) {
  setDisplay(frames[i]);
  delay(delayTime);
}

/**
 * During spinning set new value of delayTime variable according to
 * actual value of willmill velocity.
 */
void changeDelay() {
  if(velocity!=0) {
    unsigned short newDelay;
    newDelay=(unsigned short)(10000)/(velocity);
    if(newDelay > 3000) {
      delayTime = 3000;
    }
    else {
      delayTime=newDelay;
    }
  }
}

/**
 * Proceed one round in constans speed, the direction depends
 * on value of variable right.
 */
void dispRound() {
  
  if(right) {
    for(short i = 0; i < FRAMES_NUMBER; i++)
      dispFrame(i);
  }
  else {
    for(short i = FRAMES_NUMBER-1; i >= 0; i--)
      dispFrame(i);
  }
}

/**
 * Compare values of actual velocity and velocityChange, with holds value
 * that will is going to reach. Velocity raise of fall in constans acceleration
 * value. After changing value, there is also check if velocity does not get out
 * of the borders if so, the velocityChange is set.
 */
void changeVelocity() {
  if(velocity>velocityChange) {
    velocity=velocity-VELOCITY_STEP;
    if(velocity < 0)
      velocity = 0;
  }
  else if(velocity<velocityChange) {
    velocity=velocity+VELOCITY_STEP;
    if(velocity > velocityChange)
      velocity = velocityChange;
  }
}

/**
 * In case of all rounds done, change velocityChange value to so,
 * willmill will going to stop spinning after this action, also if
 * velocity is already 0 and willmill is still running, function
 * changes direction to opposite boolean value, set number of rounds
 * to number from input and velocityChange to number from input.
 */
void changeDirection() {
  if(rounds == 0) {
    velocityChange=0;
    if(velocity == 0 && running) {
      velocityChange=velocityTarget;
      rounds=roundsInput;
      right=!right;
    }
  }
}

/**
 * Initialize serial connection with 9600 baud, then wake up displays,
 * sets intensity levels for LED and clear displays for start position.
 */
void setup()
{
  Serial.begin(9600);
  lc.shutdown(0,false);
  lc.setIntensity(0,2);
  lc.clearDisplay(0);

  right = false;
  rounds = roundsInput;
  velocityChange = velocityTarget;
  velocity = 0;
  delayTime=(unsigned short)(10000)/(velocityChange);
  if(delayTime > 3000)
    delayTime = 3000;
}

/**
 * Main loop of the program. First it parse data if user sended commands through
 * serial and tries to parse them if connect change some global variables. Then
 * starts to display frames and set changes during spinning.
 */
void loop()
{
  if (Serial.available() > 0) {
    String input=Serial.readString();
    parseInput(input);
  }
  
  if(velocity == 0) {
    setDisplay(frames[0]);
    delay(2000);
  }
    
  if(velocity != 0)
    dispRound();
  
  if(velocity==velocityChange)
    rounds--;
  
  changeVelocity();
  changeDelay();
  changeDirection();
}
