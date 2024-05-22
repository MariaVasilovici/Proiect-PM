#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SD.h>
#include <SPI.h>
#include<ShiftOutX.h>
#include<ShiftPinNo.h>

int serialData = A1;
int shiftClock = A2;
int latchClock = A3;

int buzzer = 9;

#define SERIAL_DATA_PIN PC1
#define SHIFT_CLOCK_PIN PC2
#define LATCH_CLOCK_PIN PC3
#define BUZZER_PIN PB1

// Chip select pin for SD card module
const int chipSelectPin = 10;
const char* answerlist = "answords.txt";
const char* records = "recordsFile.txt";

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  { '1', '2', '3'} ,
  { '4', '5', '6'} ,
  { '7', '8', '9'} ,
  { '*', '0', '#'} 
};

byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {2, 3, 4};

Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int charsOnScreen = 0;
char keyPressed;
int nrOfGuesses;

bool soundOff = false;

shiftOutX reg(latchClock, serialData, shiftClock, LSBFIRST, 2);
unsigned long leds = 0b1111111111000000;

// Mapping of numbers to letters
const char* letterMap[] = {
  "",   // 0
  "",   // 1
  "ABC", // 2
  "DEF", // 3
  "GHI", // 4
  "JKL", // 5
  "MNO", // 6
  "PQRS", // 7
  "TUV", // 8
  "WXYZ" // 9
};


File myFile;
void setup()
{
   // TODO de pus timere ca in lab 3
  DDRC |= (1 << SERIAL_DATA_PIN);   // Set PC1 (A1) as output
  DDRC |= (1 << SHIFT_CLOCK_PIN);   // Set PC2 (A2) as output
  DDRC |= (1 << LATCH_CLOCK_PIN);   // Set PC3 (A3) as output
  DDRB |= (1 << BUZZER_PIN);        // Set PB1 (Digital 9) as output
  PORTB &= ~(1 << BUZZER_PIN);  // Set PB1 (Digital 9) to LOW
  changeLeds(leds);

  lcd.init();         // initialize the lcd
  lcd.backlight();    // Turn on the LCD screen backlight
  // Initialize random seed with a different value each time
  randomSeed(analogRead(0));

  // Configure Timer1
  TCCR1A = 0;  // Clear Timer/Counter Control Registers
  TCCR1B = 0;
  TCNT1 = 0;   // Clear Timer/Counter Register

  // Set Timer1 to CTC mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << WGM12);

  // Set the compare match value for 0.1 second delay
  // With a prescaler of 1024, each tick is 64 microseconds, and 1562 ticks correspond to approximately 0.1 seconds.
  OCR1A = 1562;  

  // Set the prescaler to 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);

  Serial.begin(9600);
  startScreen();
}

void loop()
{
  keyPressed = customKeypad.getKey();

  if (keyPressed == '1') {
    beepPress();    
    guessNumber();
  }

  else if (keyPressed == '2') {
    beepPress();
    guessWord();
  }

  else if (keyPressed == '3') {
    soundOff = !soundOff;
  }

  else if (keyPressed != NO_KEY)
    beepWrong();
}

// Function to create a delay of a specified number of seconds using Timer1
void delayTimer(unsigned int seconds) {
  for (unsigned int i = 0; i < seconds; i++) {
    // Wait for the compare match flag to be set
    while (!(TIFR1 & (1 << OCF1A)));

    // Clear the compare match flag
    TIFR1 |= (1 << OCF1A);

    // Reset the timer counter
    TCNT1 = 0;
  }
}

void changeLeds(unsigned long newLeds) {
  for (int i = 0; i < 10; i++) {
    PORTC |= (1 << LATCH_CLOCK_PIN);
    shiftOut_16(serialData, shiftClock, LSBFIRST, leds);
    PORTC &= ~(1 << LATCH_CLOCK_PIN);
  }
}

void makeGreen(int nrLed) {
  leds &= ~(1UL << (14 - nrLed * 2));
}

void makeRed(int nrLed) {
  leds &= ~(1UL << (15 - nrLed * 2));
}

void makeYellow(int nrLed) {
  makeRed(nrLed);
  makeGreen(nrLed);
}

String numbersToWord(String word = "") {
  char pastPressed = '0';
  unsigned long previousTime = 0;
  unsigned long deltaTime = 2000;
  int timesPressed = 0;
  int crtPos = word.length();
  while (1) {
    keyPressed = customKeypad.getKey();

    if (keyPressed == NO_KEY)
      continue;
  
    if (keyPressed == '#' && crtPos == 5)
      return word;

    if (keyPressed == '*' && crtPos > 0) {
      beepPress();
      crtPos--;
      word.remove(crtPos, 1);
      lcd.setCursor(4, 1);
      lcd.print(word + "      ");
      continue;
    }

    if (keyPressed < '2' || keyPressed > '9') {
      beepWrong();
      continue;
    }
    beepPress();

    deltaTime = millis() - previousTime;
    if (keyPressed == pastPressed && deltaTime <= 800) {
      timesPressed++;
    } else {
      timesPressed = 0;
      deltaTime = 2000;
    }
    pastPressed = keyPressed;
    previousTime = millis();
    
    if (keyPressed == '7' || keyPressed == '9')
      timesPressed = timesPressed % 4;
    else
      timesPressed = timesPressed % 3;

    if (timesPressed == 0 && deltaTime > 800 && crtPos < 5) {
      word += letterMap[keyPressed - '0'][timesPressed];
      crtPos++;
    } else if (deltaTime <= 800) {
      word[crtPos - 1] = letterMap[keyPressed - '0'][timesPressed];
    }

    lcd.setCursor(4, 1);
    lcd.print(word + "     ");
  }
}

void guessWord() {
  lcd.clear();
  lcd.setCursor(0, 0);

  // Initialize SD card
  if (!SD.begin(chipSelectPin)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");
  String wordToGuess = chooseRandomWord();
  wordToGuess.trim();
  wordToGuess.toUpperCase();

  lcd.print("Enter 5 letters");
  nrOfGuesses = 0;
  String word = "";

  while(1) {
    word = numbersToWord(word);
    beepPress();

    if (word == wordToGuess)
      break;
    
    String copy = word;
    copy.toLowerCase();
    if (!checkWord(copy)) {
      unsigned long tempLeds = leds;
      leds = 0b1111111111000000;
      for (int i = 0; i < 5; i++)
        makeRed(i);
  
      changeLeds(leds);
      beepWrong();
      delayTimer(7);
      leds = tempLeds;
      changeLeds(leds);
      continue;
    }

    nrOfGuesses++;
    lcd.clear();
    leds = 0b1111111111000000;
    String tempWord = word;
    String tempWTG = wordToGuess;
    for (int i = 0; i < 5; i++) {
      lcd.setCursor(i * 3, 0);
      lcd.print(word[i]);
      if (word[i] == wordToGuess[i]) {
        makeGreen(i);
        tempWord[i] = ' ';
        tempWTG[i] = ' ';
      }
    }
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        if (tempWord[i] != ' ' && tempWord[i] == tempWTG[j]) {
          makeYellow(i);
          tempWord[i] = ' ';
          tempWTG[j] = ' ';
        }
      }
    }
  
    changeLeds(leds);
    lcd.setCursor(15, 0);
    lcd.print(nrOfGuesses);
    word = "";

    if (nrOfGuesses == 6) {
      loseScreen(wordToGuess);
      return;
    }
  }

  leds = 0b1111111111000000;
  for (int i = 0; i < 5; i++) 
    makeGreen(i);
  changeLeds(leds);

  nrOfGuesses++;
  winScreen(true);
}

void loseScreen(String wordToGuess) {
  delayTimer(30);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("You lost!");
  lcd.setCursor(0, 1);
  lcd.print("Word was: ");
  lcd.setCursor(10, 1);
  lcd.print(wordToGuess);
  delayTimer(50);
  leds = 0b1111111111000000;
  changeLeds(leds);
  startScreen();
}

bool checkWord(String word) {
  String file_name;
  word.toLowerCase();

  if (word[0] == 's') {
    if (word.compareTo("slops") <= 0)
      file_name = "s1.txt";
    else
      file_name = "s2.txt";
  } else {
    file_name = String(word[0]) + ".txt";
  }
    
  File file = SD.open(file_name);

  if (!file) {
    Serial.print("Error opening ");
    Serial.println(file_name);
    return false;
  }
  String crtWord;
  while (file.available()) {
    crtWord = file.readStringUntil('\n');
    crtWord.trim();
    if (crtWord == word) {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

String chooseRandomWord() {
  // Generate a random line number
  int randomLine = random(0, 2310); // 800
  File file = SD.open(answerlist);
  if (!file) {
    Serial.print("Error opening ");
    Serial.println(answerlist);
    return;
  }
  Serial.println(randomLine);

  // Read the word from the random line
  int currentLine = 0;
  String word;

  file.seek(0);
  // Read the file byte by byte until the desired line
  while (currentLine <= randomLine) {
    word = "";
    while (true) {
      char c = file.read();
      if (c == '\n' || c == -1) {// || c == '\r') {
        break; // Reached end of line or end of file
      }
      word += c;
    }
    ++currentLine;
  }

  file.close();

  Serial.println("Random Word: " + word);
  return word;
}

void guessNumber() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Enter 4 digits");

  int nrToGuess = generateNumber();
  Serial.print("Number to guess:");
  Serial.println(nrToGuess);
  int userGuess = 0;
  int digitsEntered = 0;
  nrOfGuesses = 0;

  while(1) {
    keyPressed = customKeypad.getKey();

    // delete a digit
    if (keyPressed == '*' && digitsEntered != 0) {
      beepPress();
      digitsEntered--;
      lcd.setCursor(2 + digitsEntered, 1);
      lcd.print(" ");
      userGuess = userGuess / 10;

    // check number
    } else if (keyPressed == '#' && digitsEntered == 4) {
      beepPress();
      nrOfGuesses++;

      if (checkGuess(nrToGuess, userGuess))
        break;

      digitsEntered = 0;
      userGuess = 0;

    // add a digit
    } else if (isdigit(keyPressed) && digitsEntered < 4) {
      if (digitsEntered == 0) {
        lcd.setCursor(2, 1);
        lcd.print("              ");
        if (keyPressed == '0') {
          beepWrong();
          continue;       
        }
      }
      beepPress();

      if (!checkIfDigitInNumber(keyPressed - '0', userGuess)) {
        lcd.setCursor(2 + digitsEntered, 1);
        lcd.print(keyPressed);
        userGuess = userGuess * 10 + (keyPressed - '0');

        digitsEntered++;
      } 
    } else if (keyPressed != NO_KEY)
      beepWrong();
  }
  winScreen(false);
}

void winScreen(bool wordGame) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Congrats!");
  lcd.setCursor(2, 1);
  lcd.print(nrOfGuesses);
  if (nrOfGuesses > 9)
    lcd.setCursor(5, 1);
  else
    lcd.setCursor(4, 1);
  lcd.print("Guesses");
  delayTimer(50);
  leds = 0b1111111111000000;
  changeLeds(leds);

  startScreen();
}

void startScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1 = Guess number");
  lcd.setCursor(0, 1);
  lcd.print("2 = Guess word");
}

bool checkIfDigitInNumber(int digit, int number) {
  while (number != 0) {
      int digitToCheck = number % 10;
      if (digitToCheck == digit) {
        return true;
      }
      number /= 10;
    }
  return false;
}

bool checkGuess(int nrToGuess, int userGuess) {
  int correctDigits = 0; // Counter for digits in the correct position
  int correctDigitsInGuess = 0; // Counter for digits in the user's guess
  
  // Compare the user's guess with the number to guess
  int copyNrToGuess = nrToGuess; // Make a copy of the number to guess
  for (int i = 0; i < 4; i++) {
    int digitToGuess = nrToGuess % 10; // Extract the digit to guess
    int digitGuessed = userGuess % 10; // Extract the digit guessed by the user

    if (digitToGuess == digitGuessed) {
      correctDigits++; // Increment the counter if the digits match
    }

    if (checkIfDigitInNumber(digitGuessed, copyNrToGuess))
      correctDigitsInGuess++;

    nrToGuess /= 10; // Move to the next digit in the number to guess
    userGuess /= 10; // Move to the next digit in the user's guess
  }

  lcd.setCursor(11, 1);
  lcd.print(correctDigitsInGuess);
  lcd.setCursor(13, 1);
  lcd.print(correctDigits);
  if (correctDigits == 4)
    return true;
  return false;
}

int generateNumber() {
  // Generate first digit
  int digit1 = random(1, 10); // Generate a random number between 1 and 9
  int num = digit1 * 1000; // Place the first digit in the thousands place
  
  // Generate the other three digits, making sure they are different from each other
  int digit2, digit3, digit4;
  do {
    digit2 = random(0, 10); // Generate a random number between 0 and 9
  } while (digit2 == digit1); // Repeat if it's the same as the first digit
  num += digit2 * 100; // Place the second digit in the hundreds place
  
  do {
    digit3 = random(0, 10); // Generate a random number between 0 and 9
  } while (digit3 == digit1 || digit3 == digit2); // Repeat if it's the same as the first two digits
  num += digit3 * 10; // Place the third digit in the tens place
  
  do {
    digit4 = random(0, 10); // Generate a random number between 0 and 9
  } while (digit4 == digit1 || digit4 == digit2 || digit4 == digit3); // Repeat if it's the same as the first three digits
  num += digit4; // Place the fourth digit in the ones place

  return num;
}

void beepPress() {
  if (soundOff)
    return;
  for (int j = 0; j < 40; j++) { 
		PORTB |= (1 << BUZZER_PIN);  // Set PB1 (Digital 9) to HIGH
		delay(1);
		PORTB &= ~(1 << BUZZER_PIN);  // Set PB1 (Digital 9) to LOW
		delay(1);
	}
}

void beepWrong() {
  if (soundOff)
    return;
  for (int j = 0; j < 30; j++) { 
		PORTB |= (1 << BUZZER_PIN);  // Set PB1 (Digital 9) to HIGH
		delay(3);
		PORTB &= ~(1 << BUZZER_PIN);  // Set PB1 (Digital 9) to LOW
		delay(3);
	}
}
