#include "LedControl.h"
#define REV 100
#define TIMES 10
#define FRAMES_NUMBER 4
#define VELOCITY_STEP 10

/*--- Zmienne odpowiedzialne za kontrolę obrotów i wyświetlania ---*/

LedControl lc = LedControl(12,11,10,1);  // Piny: DIN,CLK,CS
unsigned short delayTime = 1000;  // Początkowe opóźnienie pomiędzy klatkami
boolean running = true; // Oznacza stan, true gdy powinien się kręcić, false w przeciwnym wypadku
boolean right = false; // Oznacza kierunek kręcenie
unsigned short velocity = 0;  // Aktualna prędkość w obrotach na dziesięć sekund
unsigned short rounds = TIMES; // Ilość obrotów, które w aktualnej pracy mają jeszcze zostać wykonane
unsigned short roundsInput = TIMES; // Ilość obrotów, która ma zostać wykonana ze stałą prędkością
unsigned short velocityChange = REV; // Prędkość w obrotach na dziesięć sekund, którą młynek powinien aktualnie osiągnąć
unsigned short velocityTarget = REV; // Poczatkowa wartość maksymalnej wartości prędkości
unsigned short velocityToFlush = REV; // Przechowuje ilość obrotów zadaną przez użytkownika przed samą podmianą
unsigned short roundsToFlush = TIMES; // Przechowuje wartość prędkości zadaną przez użytkownika przed samą podmianą

/*-------- Zmienne odpowiedzialne za wyświetlane klatki ---------- */

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
    B10000000,
    B01000000,
    B00100000,
    B00011000,
    B00011000,
    B00000100,
    B00000010,
    B00000001
};

byte a3[]=
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

byte a4[]=
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

byte* frames[]=
{
  a1,a3,a4,a4
};

/*----- Funkcje odpowiedzialne za interakcje z użytkownikiem -----*/

/**
 * Sprawdzenie czy napis w argumencie reprezentuje poprawnie sformatowaną liczbę.
 */
boolean checkIfNumber(String lineToCheck) {
   for(byte i = 0; i < lineToCheck.length(); ++i)
     if(lineToCheck.charAt(i) < '0' || lineToCheck.charAt(i) > '9')
      return false;
   return true;
}

/**
 * Funkcja ma za zadanie sprawdzić poprawnośc podanego w argumencie napisu,
 * jeżeli napis był poprawny wykonuje odpowiednią akcję, jeżeli jego początek był
 * poprawny informuje o niepoprawnej części końcowej, natomiast w przypadku podania
 * napisu z niepoprawnym pierwszym znakiem, informuje o dozwolonych komendach.
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
  else if(input.length() >= 1 && input[0]=='S') {
    if(velocityToFlush > 990)
      velocityToFlush = 990;
    velocityTarget=velocityToFlush;
    roundsInput=roundsToFlush;
    rounds=0;
    velocityChange=0;
    Serial.print("Applying new settings \n");
    //setup();
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

/*--- Główna funkcja programu oraz funkcje odpowiadające za animację ---*/

/**
 * Wyświetla wiersz po wierszu wartości zapamiętane w tablicy zmiennych byte
 * podanej w argumencie.
 */
void setDisplay(byte img[]) {
  for(int i = 0; i < 8; i++) {
    lc.setRow(0,i,img[i]);
  }
}

/**
 * Funkcja odpowiada za wyświetlenie klatki animacji, której numer został
 * podany w argumencie, wraz z wykonaniem aktualnie ustalonego opóźnienia.
 */
void dispFrame(short i) {
  setDisplay(frames[i]);
  delay(delayTime);
}

/**
 * Odpowiada za zmianę wartości opóźnienia na podstawie aktualnej wartości prędkości.
 * W celu podtrzymania płynności animacji, przy zbyt niskiej wartości prędkości ustalana
 * jest sztywna wartość 3000.
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
 * Funkcja przeprowadza jeden pełny obrót animacji wyświetlając
 * wszystkie klatki animacji, wyświetlając je w kolejności zależnej
 * od zmiennej right, która określa kierunek obrotu.
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
 * Funkcja odpowiada za zmianę wartości prędkości w sytuacji, gdy aktualna prędkość
 * nie jest równa prędkości docelowej, w zależności od tego czy animacja ma przyśpieszyć
 * czy zwolnić, odpowiednia, stała wartość jest dodawana lub odejmowana od aktualnej prędkości
 * przy czym w przypadku otrzymania wartości ujemnej jest ona korygowana do 0, natomiast wartość
 * większa od docelowej skutkuje ustaleniem prędkości docelowej jako aktualna.
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
 * Funkcja sprawdza ilość obrotów, które powinny zostać wykonane, jeżeli to zajdzie
 * ustala prędkość docelową na 0, co oznacza rozpoczęcie zwalniania animacji, a w przypadku
 * gdy animacja się zatrzyma, ale użytkownik nie zdecydował o jej zakończeniu, zmienia kierunek,
 * ustala docelową prędkość na przechowywaną w pamięci wartość maksymalną obrotów oraz ustawia
 * nową wartość obrotów do wykonania, jako wartość ilości obrotów przechowywaną w pamięci.
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
 * Funkcja odpowiada za ustalenie parametrów połączenia serial, ustalanie jasności
 * LED, a także przypisanie początkowych wartości animacji.
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
 * Główna pętla programu, na początku sprawdza czy użytkownik nie przesłał danych
 * poprzez serial, jeżeli tak to w przypadku poprawności tych danych przeprowadza
 * odpowiednią akcję, w przeciwnym wypadku informuje o błędzie poprzez serial. Następnie
 * jeżeli prędkośc wiatraczka nie jest zerowa przeprowadza pełen obrót animacji, po czym
 * uruchamia funkcje odpowiedzialne za zmianę parametrów animacji.
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