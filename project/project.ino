#include "LedControl.h"
#define REV 100
#define TIMES 10
#define FRAMES_NUMBER 4
#define VELOCITY_STEP 10

/*--- Zmienne odpowiedzialne za kontrolę obrotów i wyświetlania ---*/

/**
 * Piny: DIN,CLK,CS
 */
LedControl lc = LedControl(12,11,10,1); 

/**
 * Początkowe opóźnienie pomiędzy klatkami
 */
unsigned short delayTime = 1000;

/**
 * Oznacza stan wiatraka. True gdy powinien się kręcić, false w
 * przeciwnym wypadku 
 */
boolean running = true; 

/**
 * Oznacza kierunek kręcenia
 */
boolean right = false;

/**
 * Aktualna prędkość w obrotach na dziesięć sekund
 */
unsigned short velocity = 0;

/**
 * Ilość obrotów ze stałą prędkością pozostałych do wykonania w 
 * obecnym cyklu
 */
unsigned short rounds = TIMES;

/**
 * Ilość obrotów ze stałą prędkością, która ma zostać wykonana w 
 * każdym cyklu
 */
unsigned short roundsInput = TIMES;

/**
 * Prędkość w obrotach na dziesięć sekund, którą wiatrak powinien
 * aktualnie osiągnąć
 */
unsigned short velocityChange = REV;

/**
 * Docelowa wartość prędkości obrotowej
 */
unsigned short velocityTarget = REV;

/**
 * Przechowuje ilość obrotów zadaną przez użytkownika komendą REV
 */
unsigned short velocityToFlush = REV;

/**
 * Przechowuje wartość prędkości zadaną przez użytkownika komendą N
 */
unsigned short roundsToFlush = TIMES;



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
 * Sprawdzenie czy napis w argumencie reprezentuje poprawnie 
 * sformatowaną liczbę.
 */
boolean checkIfNumber(String lineToCheck) {
   for(byte i = 0; i < lineToCheck.length(); ++i)
     if(lineToCheck.charAt(i) < '0' || lineToCheck.charAt(i) > '9')
      return false;
   return true;
}

/**
 * Funkcja ma za zadanie sprawdzić poprawność podanego w 
 * argumencie napisu. Jeżeli był on poprawny, wykonuje odpowiednią
 * akcję. Jeżeli jego początek był poprawny, informuje o niepoprawnej 
 * części końcowej. W przypadku podania napisu z niepoprawnym 
 * pierwszym znakiem, informuje o dozwolonych komendach.
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
 * Wyświetla wiersz po wierszu wartości zapamiętane w tablicy 
 * zmiennych byte podanej w argumencie.
 */
void setDisplay(byte img[]) {
  for(int i = 0; i < 8; i++) {
    lc.setRow(0,i,img[i]);
  }
}

/**
 * Funkcja odpowiada za wyświetlenie klatki animacji, której numer
 * został podany w argumencie, wraz z wykonaniem aktualnie
 * ustalonego opóźnienia.
 */
void dispFrame(short i) {
  setDisplay(frames[i]);
  delay(delayTime);
}

/**
 * Odpowiada za zmianę wartości opóźnienia na podstawie aktualnej 
 * wartości prędkości. W celu podtrzymania płynności animacji, przy 
 * zbyt niskiej wartości prędkości ustalane jest opóźnienie 3000.
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
 * Funkcja przeprowadza jedną animację pełnego obrotu wyświetlając
 * wszystkie klatki animacji w kolejności zależnej od zmiennej right,
 * która określa kierunek obrotu.
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
 * Funkcja odpowiada za zmianę wartości prędkości w sytuacji, 
 * gdy aktualna prędkość nie jest równa prędkości docelowej.
 * W zależności od tego czy animacja ma przyśpieszyć czy 
 * zwolnić, odpowiednia stała wartość jest dodawana lub 
 * odejmowana od aktualnej prędkości. W przypadku otrzymania
 * wartości ujemnej, jest ona korygowana do 0, natomiast 
 * wartość większa od docelowej skutkuje ustaleniem prędkości
 * docelowej jako aktualna.
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
 * Funkcja sprawdza ilość obrotów, które powinny zostać wykonane.
 * Po wykonaniu założonej ilości obrotów, ustala prędkość 
 * docelową na 0, co oznacza rozpoczęcie zwalniania animacji.
 * W przypadku, gdy animacja się zatrzyma, ale użytkownik nie 
 * zdecydował o jej zakończeniu, zmienia kierunek, ustala docelową
 * prędkość na przechowywaną w pamięci wartość maksymalną obrotów
 * oraz ustawia nową wartość obrotów do wykonania, jako wartość 
 * ilości obrotów przechowywaną w pamięci.
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
 * Funkcja inicjująca odpowiada za ustalenie parametrów połączenia
 * serial, ustalanie jasności LED, a także przypisanie początkowych 
 * wartości animacji.
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
 * Główna pętla programu.
 * Na początku sprawdza czy użytkownik nie przesłał danych przez 
 * serial, jeżeli tak to sprawdza ich poprawność i wywołuje żądaną 
 * akcję lub informuje o błędzie. Gdy prędkość wiatraczka nie jest 
 * zerowa, przeprowadza animację pełnego obrotu, po czym uruchamia 
 * funkcje odpowiedzialne za zmianę parametrów animacji.
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
