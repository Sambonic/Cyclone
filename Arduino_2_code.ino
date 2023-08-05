//libraries for the oled
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// This code initializes the OLED display using the Adafruit_SSD1306 library.
#define OLED_RESET 4 
Adafruit_SSD1306 display(OLED_RESET);

// Defines the pins for the two menu buttons and the current game mode.
#define NEXT_BTN 2
#define CONFIRM_BTN 3
volatile int gameMode = 0;
// Defines variables to track whether each button has been pressed and the last time the button was debounced. 
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
unsigned long lastDebounceTime1 = 0;
const unsigned long debounceDelay1 = 50; //initialize delay
unsigned long lastDebounceTime2 = 0;
const unsigned long debounceDelay2 = 59;
// Defines the current level and player scores.
int currentLevel = 1;
static int player1Score = 0;
static int player2Score = 0;

// Defines variables to track whether the menu is currently displayed and if the "next" button is attached.
bool menuON = false;
bool nextButtonAttached = true;

void startingSequence();
void displayMenu();
void gameConfirmed();

void displayYouWin();
void displayGameOver();

void displayPlayer1Wins();
void displayPlayer2Wins();
void displayScore();
void displayLevel();

void insertCoin();
void nextChoice();
void confirmChoice();

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //begin displat of oled by sending object and size
  display.clearDisplay();//initalize screen to blank
  //font of text on screen
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.println("Please Insert a Coin"); 
  display.display();
  //define pinmodes for menu buttons
  pinMode(NEXT_BTN,INPUT_PULLUP);
  pinMode(CONFIRM_BTN,INPUT_PULLUP);
  //intialize menu button presses to false
  button1Pressed = false;
  button2Pressed = false;

  //interupts
  attachInterrupt(digitalPinToInterrupt(NEXT_BTN), nextChoice, FALLING);
  attachInterrupt(digitalPinToInterrupt(CONFIRM_BTN), confirmChoice, FALLING);
}

void loop() {
  
  if (Serial.available() > 0) {//check if arduino 2 sent a complete signal
    char incomingByte = Serial.read();
    if (incomingByte >= 'A' && incomingByte <= 'D'){
      if (incomingByte == 'A') {
        startingSequence();//display "Press menu button" on oled
        delayMicroseconds(50000);
      }
      else if (incomingByte == 'B') {
        displayMenu();   //display menu modes on oled
        if(nextButtonAttached){
          menuON = true;
          nextButtonAttached=false;
        }
        Serial.println("Menu screen on");       
      }
      else if (incomingByte == 'C') {
        displayYouWin();// display win on oled
        delayMicroseconds(50000);
      }
      else if (incomingByte == 'D'){
        displayGameOver(); // display gameover on oled
        delayMicroseconds(50000);
      }
    }
    //mulitplayer
    if (incomingByte >= 'E' && incomingByte <= 'J'){
      if (incomingByte == 'E'){
        displayPlayer1Wins(); //display player 1 wins on oled
        delayMicroseconds(100000);
      }
      else if (incomingByte == 'F'){
        displayPlayer2Wins(); //display player 2 wins on oled
        delayMicroseconds(100000);
      }
      else if (incomingByte == 'H'){
        displayScore(); //display score of 2 players on oled
        delayMicroseconds(50000);
      }
      else if (incomingByte == 'I'){//increment player 1 score and level
        player1Score = player1Score +1;
        currentLevel = currentLevel+1;
      }
      else if (incomingByte == 'J'){//increment player 2 score and level
        player2Score = player2Score+1;
        currentLevel = currentLevel+1;
      }
    }
    if (incomingByte >= 'K' && incomingByte <= 'Z'){
      if (incomingByte == 'R'){
        insertCoin(); //display insert coin on oled
        delayMicroseconds(50000);
      }
      else if (incomingByte == 'L'){
        displayLevel();//display current game level on oled
        delayMicroseconds(50000);
      }
    }
  }
  //display menu
  if(menuON){
    displayMenu();
  }
  
  
  gameConfirmed();
  button1Pressed = false;
  button2Pressed = false;
  menuON = false;
}
//OLED DISPLAYS
//A//
void startingSequence(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(15, 10);
    display.println("Press menu button");
    display.setCursor(40, 20);
    display.println("to start");
    display.display();
}
//B//
void displayMenu() {
  if (!nextButtonAttached) {
    attachInterrupt(digitalPinToInterrupt(NEXT_BTN), nextChoice, FALLING);
    nextButtonAttached = true;
  }

  if(button1Pressed == true){
    gameMode = gameMode +1;
    if(gameMode==4){
      gameMode=0;
    }
    button1Pressed = false;
  }
        
  // Print full menu text
  if(gameMode == 1){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(12, 3);
    display.println("Choose game mode:");
    display.setTextSize(2);
    display.setCursor(3, 15);
    display.println("> Classic");
  }
  else if(gameMode == 2){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(12, 3);
    display.println("Choose game mode:");
    display.setTextSize(2);
    display.setCursor(3, 15);
    display.println("> Memory");
  }
  else if(gameMode == 3){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(12, 3);
    display.println("Choose game mode:");
    display.setTextSize(2);
    display.setCursor(3, 15);
    display.println("> M-Player");
  }
  display.display();

  
}
void gameConfirmed(){
  if(button2Pressed && gameMode>0) {
    detachInterrupt(digitalPinToInterrupt(NEXT_BTN));
    nextButtonAttached = true;

    menuON = false;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    // Print full menu text
    display.setCursor(35, 10);
    display.println("Loading...");
    display.display();

    Serial.println("bless");

    if(gameMode==1){
      Serial.write('1');
    }
    else if(gameMode==2){
      Serial.write('2');        
    }
    else if(gameMode==3){
      Serial.write('3');
    }
    button2Pressed = false;
    gameMode = 0;
  }
}
//C//
void displayYouWin() {
  currentLevel = currentLevel+1;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("You win:)");
  display.display();
  
  delayMicroseconds(1000000);
  display.clearDisplay();
}
//D//
void displayGameOver() {
  currentLevel = 1;
  player1Score = 0;
  player2Score = 0;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Game over");
  display.display();
  
}

//E//
void displayPlayer1Wins(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(35, 3);
  display.println("Winner is:");
  display.setTextSize(2);
  display.setCursor(15, 15);
  display.println("Player 1!");
  display.display();

}
//F//
void displayPlayer2Wins(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(35, 3);
  display.println("Winner is:");
  display.setTextSize(2);
  display.setCursor(15, 15);
  display.println("Player 2!");
  display.display();
}
//H//
void displayScore(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(5, 3);
  display.print("Player 1:");
   display.setCursor(70, 3);
  display.println("Player 2:");
  display.setTextSize(2);
  display.setCursor(25, 15);
  display.println(player1Score);
  display.setCursor(90, 15);
  display.println(player2Score);
  display.display();
}

//L//
void displayLevel(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.print("Level ");
  display.print(currentLevel);
  Serial.print(currentLevel);
  display.display();  
}
//R//
void insertCoin(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 10);
  display.println("Please Insert a Coin");
  display.display();  
  currentLevel = 1;
}
// This code defines two functions to detect button presses
void nextChoice() {
  // Records the current time using millis().
  unsigned long currentTime1 = millis();
  // Checks if the elapsed time since the last button press is greater than the debounce delay
  if (currentTime1 - lastDebounceTime1 > debounceDelay1) {
    // Sets the button1Pressed flag to true.
    button1Pressed = true;
     // Updates the last debounce time to the current time
    lastDebounceTime1 = currentTime1;
  }
  // Prints a message to the serial monitor indicating that the "Next" button was pressed.
  Serial.println("Next");
}       
void confirmChoice() {
  // Records the current time using millis().
  unsigned long currentTime2 = millis();
  // Checks if the elapsed time since the last button press is greater than the debounce delay. 
  if (currentTime2 - lastDebounceTime2 > debounceDelay2) {
      button2Pressed = true; // Sets the button2Pressed flag to true
      lastDebounceTime2 = currentTime2;// Updates the last debounce time to the current time
  }
  Serial.println("Confirmed");  // Prints a message to the serial monitor indicating that the "Confirm" button was pressed.      
}


