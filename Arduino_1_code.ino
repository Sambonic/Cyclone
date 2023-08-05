#include <Adafruit_NeoPixel.h> // for the light ring

#define PIN 6 		// Pin that the Neopixel Ring is connected to
#define NUM_LEDS 24 // Number of LEDs in the Neopixel Ring

Adafruit_NeoPixel ring(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);// Creates a new instance of the Adafruit_NeoPixel class and sets the LED count and neopixel pin.

// Defines the pins for the buttons, buzzer and 2 LED lights.
#define PLAYER1_BTN 2
#define PLAYER2_BTN 3
#define BUZZER 5
#define LEFT_BTN 8
#define ENTER_BTN 9
#define RESET_BTN 10
#define PLAYER2_LED 11
#define PLAYER1_LED 12

//defining the gamemode number and if there is a gamemode selected
int gameMode = 1;
bool gameModeSelected = false;

// Defines the initial positions for the players and the level
volatile int i = 0; //Player 1/Single player position
volatile int j = 12; //Player 2 position
volatile int level = 0;

//Defines the speeds and increments for the game
int arraySpeeds[4]= {500,400,300,200};
int levelIncrement=0;

//Gamemode 1 led positions for the 4 levels 
int arrayDesired[4] = {random(0,NUM_LEDS),random(0,NUM_LEDS),random(0,NUM_LEDS),random(0,NUM_LEDS)};
volatile int desiredStatus[4] = {255,255,255,255};

//Gamemode 2
bool firstSequence = true; //shows the LED sequence once in memory mode

//Gamemode 3 led positions for the 2 fixed red leds, players' score's, players' bool for defining which player is which
int arrayDesiredLeds[2]={11,23};
volatile int desiredStatusLeds[2] = {255,255};
volatile int player1Pts = 0;
volatile int player2Pts = 0;
volatile bool a=true;
volatile bool b=true;

//PIR sensor 
int PIRSensor = 13;
int senval = 0; //PIR read
bool powerOn = false;//defining the intial value for the power state of the game

//Buzzer notes
#define NOTE_F5  698
#define NOTE_D5  587
#define NOTE_A5  880
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_AS5 932
//for the buzzer sounds
volatile int thisNote=0;// to iterate between the notes in melody[]
volatile int duration=0;// duration of each note
volatile int pause=0;
int melody[] = 
{
  NOTE_F5,0, NOTE_D5,0, NOTE_A5,0, NOTE_C6,0, NOTE_A5,0, NOTE_D6,0, 
  NOTE_A5,0, NOTE_C6,0, NOTE_AS5,0, NOTE_A5,0};
int durations[] = {
  400,20, 600,20, 210,20, 210,20, 210,200};
int songLength = sizeof(melody)/sizeof(melody[0]);


//Documentation below//
void buzzerwin();
void buzzerlose();
void resetGame();
void levelCompleted();
void gameCompleted();


void classicMode();
void memoryMode();
void multiplayerMode();

void buttonPressedClassic();
void buttonPressedMemory();
void buttonPressedMultiplayer(int pos);

void gameDisplay();

//interupts
void isr1();
void isr2();

void setup() {
  Serial.begin(9600);
  //defining pinmodes
  pinMode(PLAYER1_BTN,INPUT_PULLUP);
  pinMode(PLAYER2_BTN,INPUT_PULLUP);
  pinMode(PLAYER1_LED, OUTPUT);
  pinMode(PLAYER2_LED, OUTPUT);
  
  pinMode(PIRSensor, INPUT); 
  
  pinMode(BUZZER,OUTPUT);
  
  pinMode(LEFT_BTN,INPUT_PULLUP);
  pinMode(ENTER_BTN,INPUT_PULLUP);
  pinMode(RESET_BTN,INPUT_PULLUP);
  
  ring.begin();
  ring.show(); // Initialize all pixels to 'off'

  attachInterrupt(0,isr1,FALLING);
  attachInterrupt(1,isr2,FALLING);
}
void loop() {
  
  //Check if coin inserted
  senval = digitalRead(PIRSensor);
  // if the power is off and the pir senses a motion, the power will be turned onn
  if(!powerOn) {
    if(senval == HIGH){
      powerOn=true;
     
    }

    else{
      Serial.write('A');
    }
    delay(100);
  }
  //if the power is on, the system is turned on
  else{
    
    //OLED menu screen
    gameDisplay();

    //Enter game mode
    //for each respective gameMode 1,2,3 their respective display functions are called and the player's buttons led is on
    if (gameModeSelected){
      if (gameMode == 1) {
          classicMode();
          digitalWrite(PLAYER1_LED,HIGH);
       }
      else if (gameMode == 2) {
          memoryMode();
          digitalWrite(PLAYER1_LED,HIGH);
       }
      else if (gameMode == 3) {
          attachInterrupt(1,isr2,FALLING);
          multiplayerMode();
          digitalWrite(PLAYER1_LED,HIGH);
          digitalWrite(PLAYER2_LED,HIGH);
      }
    }

     //Exit game mode when push reset button(back)
    if( digitalRead(RESET_BTN) == LOW){
      
      resetGame();
      ring.fill(ring.Color(0, 0, 0));
      ring.show();
      Serial.println("Exiting Game. . .");
      detachInterrupt(1);
      digitalWrite(PLAYER1_LED,LOW);
      digitalWrite(PLAYER2_LED,LOW);
      Serial.write("R");
      gameModeSelected = false;
      powerOn = false;
    }
  }
}

//Gamemodes//
void classicMode(){
  
  Serial.write('L');//display level
  //Check if player completed a level or the game
  levelCompleted();
  gameCompleted();

  //Desired LED(s)
  for (int z = 0; z <= level; z++) {
    ring.setPixelColor(arrayDesired[z], desiredStatus[z], 0, 0);
  }
  ring.show();
  
  // Set the color of the i-th LED to (red, green, blue) 
  ring.setPixelColor(i, 0, 255, 0);
  ring.show();
  ring.setPixelColor(i, 0, 0, 0);
  delay(arraySpeeds[level]);
  
  //Increment to next LED
  i++;
  (i==NUM_LEDS)?(i=0):0;
}
void memoryMode(){
  Serial.write('L');//display level
  //Check if player completed a level or the game
  levelCompleted();
  gameCompleted();
  delayMicroseconds(50000);
  //Memory Sequence displayed once
  if(firstSequence){
    for (int z = 0; z <= level; z++) {
      ring.setPixelColor(arrayDesired[z], desiredStatus[z], 0, 0);
      delay(arraySpeeds[level]*5);
      ring.show();
    }
 
    delay(arraySpeeds[level]);
    ring.fill(ring.Color(0, 0, 0));   //hide the desired LED
    ring.show();
    delay(arraySpeeds[level]*5);
    firstSequence = false;
  }
  
  // Set the color of the i-th LED to (red, green, blue) 
  ring.setPixelColor(i, 0, 255, 0);
  ring.show();
  ring.setPixelColor(i, 0, 0, 0);
  delay(arraySpeeds[level]);
  
  //Increment to next LED
  i++;
  (i==NUM_LEDS)?(i=0):0;
}
void multiplayerMode(){
  Serial.write('H'); //display players' scores
  
  //Desired LED(s)
  ring.setPixelColor(arrayDesiredLeds[0], desiredStatusLeds[0], 0, 0);
  ring.setPixelColor(arrayDesiredLeds[1], desiredStatusLeds[1], 0, 0);
  ring.show();
  
  //Player 1 LED green, Player 2 LED blue
  // Set the color of the i-th LED to (red, green, blue) 
  if(a){
  	ring.setPixelColor(i, 0, 255, 0);//light green player 1
  }
  if(b){
  	ring.setPixelColor(j, 0, 0, 255);//light green player 1
  }
  ring.show();
  ring.setPixelColor(i, 0, 0, 0);
  ring.setPixelColor(j, 0, 0, 0);
  delay(arraySpeeds[level]);
  
  //Increment to next LED
  i++;
  j++;
  (i==24)?(i=0):0;
  (j==24)?(j=0):0;
}

//OLED Menu Display//
void gameDisplay(){
  //if no gameMode is selected 
  if (!gameModeSelected ){
      delayMicroseconds(100000);
      Serial.write('B'); //,display menu
      if (Serial.available() > 0) {//check if arduino 2 sent a complete signal
        int incomingByte = Serial.read();
        Serial.flush();
        if (incomingByte == '1') {//classical
          gameMode = 1;
          gameModeSelected = true;
          Serial.println("Classic mode has been selected");
        }
        else if (incomingByte == '2') {//memory
          gameMode = 2;
          gameModeSelected = true;
          Serial.println("Memory mode has been selected");
        }
        else if (incomingByte == '3') {//multiplayer 
          gameMode = 3;
          gameModeSelected = true;
          Serial.println("Multiplayer has been selected");
        }
      }
  } 
}


 //Interrupts' conditions//
void isr1(){
  //call respective game mode logic functions
  if (gameMode == 1) {
    buttonPressedClassic();
  }
  else if (gameMode == 2) {
    buttonPressedMemory();
  }
  else if (gameMode == 3) {
    buttonPressedMultiplayer(i);
  }
}
void isr2(){
  //for player 2
  if (gameMode == 3) {
    buttonPressedMultiplayer(j);
  }
}

//Buzzer sound Functions//
void buzzerwin(){

  thisNote=3;//start from 4rth note

  duration = durations[thisNote]; //duration of note
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++; //move to next note

  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;

  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;

  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;

  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;

  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;

  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;

  }

void buzzerlose(){
  //start from note 1
  duration = durations[thisNote];
  tone(BUZZER, melody[thisNote], duration);
  pause = duration * 1.3;
  delay(pause);
  noTone(BUZZER);
  thisNote++;
}
//Game Mechanics//
void resetGame(){
  for(int i =0;i<10;i++){
      Serial.write('D'); //sends signal to display gameover
    }
    //for gamemode 1 and 2 when they lose
  if(level != 4 && gameMode !=3){
    delayMicroseconds(50000);
  	Serial.println("Game Over");
  }
  //Indicate Fail
    ring.fill(ring.Color(255, 0, 0));
    ring.show();
    buzzerlose();
    delayMicroseconds(10000);
    ring.clear();
    
  //multiplayer
  if(gameMode==3){
    //reattach the players' button interupts and reset scores
    attachInterrupt(0,isr1,FALLING);
    attachInterrupt(1,isr2,FALLING);
    player1Pts = 0;
    player2Pts = 0;
    a=true;
    b=true;
  }
  //Reset game
  i= 0;
  j=12;
  level=0;
  levelIncrement=0;

  desiredStatus[0]=255;
  desiredStatus[1]=255;
  desiredStatus[2]=255;
  desiredStatus[3]=255;
  
  desiredStatusLeds[0]=255;
  desiredStatusLeds[1]=255;
  
  firstSequence = true;
  return;
}
void levelCompleted(){
  //When all LEDs pressed, Move onto next Level
    if(levelIncrement==(level+1)){
      Serial.println("Moving to the next Level!");

      //Indicate Success
      Serial.write('C');
      ring.rainbow();
      ring.show();
      delayMicroseconds(200000);
      buzzerwin();
      noTone(BUZZER);
      ring.clear();
      
      //Reset positions
      i=0;
      levelIncrement=0;
      thisNote=0;
      
      if(gameMode == 1 || gameMode == 2){
        desiredStatus[0]=255;
        desiredStatus[1]=255;
        desiredStatus[2]=255;
        desiredStatus[3]=255;
      }
      //gameMode 2
      if(gameMode == 2){     
        firstSequence = true;
      }
      
      if(gameMode == 3){     
        j=12;
        desiredStatusLeds[0]=255;
        desiredStatusLeds[1]=255;
      }
      
      
      //Next Level all game modes
      level++;
    }
    return;
}
void gameCompleted(){
  //Winner Msg
  if(level==4){
    Serial.println("You win!");
    ring.rainbow();
    delayMicroseconds(100000);
    ring.clear();
    delayMicroseconds(100000);
    ring.rainbow();
    delayMicroseconds(100000);
    ring.clear();
    buzzerwin();

    //Reset game
    resetGame();
    return;    
  }    
}


//Buttons' logic conditions//
void buttonPressedClassic(){
  delayMicroseconds(50000);
  //Light change to indicate press
  ring.setPixelColor(i, 127, 0, 255);//purple
  ring.show();
  
   //Pressed A Desired LED for all levels
  if(i==arrayDesired[0] || i==arrayDesired[1] ||
    	i==arrayDesired[2] || i==arrayDesired[3]){
        if(level==0){
          if(i==arrayDesired[0] && desiredStatus[0]==255){//if click on desired and its also red
            desiredStatus[0]=0;//hide red
            levelIncrement++;//increase level
            thisNote=0;//reset the buzzer
            return;
          }
          else{//lose
            Serial.println("Lost to level 1");
            resetGame();
          }
        }
        //repeat for 4 levels
        if(level==1){
          if(i==arrayDesired[0] && desiredStatus[0]==255){
            desiredStatus[0]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else if(i==arrayDesired[1] && desiredStatus[1]==255){
            desiredStatus[1]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else{//lose
            Serial.println("Lost to level 2");
            resetGame();
          }
        }
        if(level==2){
          if(i==arrayDesired[0] && desiredStatus[0]==255){
            desiredStatus[0]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else if(i==arrayDesired[1] && desiredStatus[1]==255){
            desiredStatus[1]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else if(i==arrayDesired[2] && desiredStatus[2]==255){
            desiredStatus[2]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else{
            Serial.println("Lost to level 3");
            resetGame();
          }
        }
        if(level==3){
          if(i==arrayDesired[0] && desiredStatus[0]==255){
          desiredStatus[0]=0;
          levelIncrement++;
            thisNote=0;
            return;
        }
        else if(i==arrayDesired[1] && desiredStatus[1]==255){
          desiredStatus[1]=0;
          levelIncrement++;
          thisNote=0;
          return;
        }
        else if(i==arrayDesired[2] && desiredStatus[2]==255){
          desiredStatus[2]=0;
          levelIncrement++;
          thisNote=0;
          return;
        }
        else if(i==arrayDesired[3] && desiredStatus[3]==255){
          desiredStatus[3]=0;
          levelIncrement++;
          thisNote=0;
          return;
        }
        else{
          Serial.println("Lost to level 4");
          resetGame();
        }
      }
    }
      //didnt press any of the desired     
    else{
      //Reset game
      resetGame();
    }
  
}
void buttonPressedMemory(){
  //Light change to indicate press
  ring.setPixelColor(i, 127, 0, 255);//purple
  ring.show();
  
   //Pressed A Desired LED for all levels
  if(i==arrayDesired[0] || i==arrayDesired[1] ||
    	i==arrayDesired[2] || i==arrayDesired[3]){
        if(level==0){
          if(i==arrayDesired[0] && desiredStatus[0]==255){//if click on desired and its also red
            desiredStatus[0]=0;//hide red
            levelIncrement++;//increase level
            thisNote=0;//reset the buzzer
            return;
          }
          else{
            Serial.println("Lost to level 1");
            resetGame();
          }
        }
        //repeat for the 4 levels
        if(level==1){
          if(i==arrayDesired[0] && desiredStatus[0]==255){
            desiredStatus[0]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else if(i==arrayDesired[1] && desiredStatus[1]==255){
            desiredStatus[1]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else{
            Serial.println("Lost to level 2");
            resetGame();
          }
        }
        if(level==2){
          if(i==arrayDesired[0] && desiredStatus[0]==255){
            desiredStatus[0]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else if(i==arrayDesired[1] && desiredStatus[1]==255){
            desiredStatus[1]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else if(i==arrayDesired[2] && desiredStatus[2]==255){
            desiredStatus[2]=0;
            levelIncrement++;
            thisNote=0;
            return;
          }
          else{
            Serial.println("Lost to level 3");
            resetGame();
          }
        }
        if(level==3){
          if(i==arrayDesired[0] && desiredStatus[0]==255){
          desiredStatus[0]=0;
          levelIncrement++;
            thisNote=0;
            return;
        }
        else if(i==arrayDesired[1] && desiredStatus[1]==255){
          desiredStatus[1]=0;
          levelIncrement++;
          thisNote=0;
          return;
        }
        else if(i==arrayDesired[2] && desiredStatus[2]==255){
          desiredStatus[2]=0;
          levelIncrement++;
          thisNote=0;
          return;
        }
        else if(i==arrayDesired[3] && desiredStatus[3]==255){
          desiredStatus[3]=0;
          levelIncrement++;
          thisNote=0;
          return;
        }
        else{
          Serial.println("Lost to level 4");
          resetGame();
        }
      }
    }
    //didnt press any of the desired 
    else{
      //Reset game
      resetGame();
    }
}
void buttonPressedMultiplayer(int pos){//position is the generalised location of the player either i or j
  //Light change to indicate press
  ring.setPixelColor(pos, 127, 0, 255);//purple
  ring.show();
  
  //Pressed A Desired LED
//position of 1st red led
  if(pos==arrayDesiredLeds[0] && desiredStatusLeds[0]==255){//if one of the players click on desired and its also red
    desiredStatusLeds[0]=0;//hide red
    levelIncrement++;//increase level
    //if player 1 
    if(pos==i){
      player1Pts++;
      Serial.write('I');
      Serial.println(player1Pts);
    }
    //if player 2
    if(pos==j){
      player2Pts++;
      Serial.write('J');
      Serial.println(player2Pts);
    }
    thisNote=0;//reset the buzzer
  }
  //position of 2nd red led
  else if(pos==arrayDesiredLeds[1] && desiredStatusLeds[1]==255){//if one of the players click on desired and its also red
    desiredStatusLeds[1]=0;//hide red
    levelIncrement++;//increase level
    //if player 1 
    if(pos==i){
      player1Pts++;
      Serial.write('I');
      Serial.println(player1Pts);
    }
    //if player 2
    if(pos==j){
      player2Pts++;
      Serial.write('J');
      Serial.println(player2Pts);
    }
    thisNote=0;//reset the buzzer
  }
  //click wrong
  else{
    if(pos==i && a){//if player 1
      a=false;//turn off the light for the round for the mulitplayer function 
      ring.setPixelColor(i, 0, 0, 0);
      ring.show();
      detachInterrupt(0); //disable button
    }
    if(pos==j && b){//if player 1
      b=false; //turn off the light for the round for the mulitplayer function 
      ring.setPixelColor(j, 0, 0, 0); 
      ring.show();
      detachInterrupt(1); //disable button
    }
  }
    
    
    //When all LEDs pressed, Move onto next Level
    if(levelIncrement==1 && level!=4){
      Serial.println("Moving to the next Level!");
      a=true;
      b=true;
      attachInterrupt(0,isr1,FALLING); //reattach player 1 button
  	  attachInterrupt(1,isr2,FALLING);//reattach player 2 button
      
      //Indicate Success
     
      ring.rainbow();
      ring.show();
      delayMicroseconds(250000);
      buzzerwin();
      noTone(BUZZER);
      ring.clear();
      
      //Reset positions
      i=0;
      j=12;
      levelIncrement=0;
      thisNote=0;
      desiredStatusLeds[0]=255;
      desiredStatusLeds[1]=255;
      
      //Next Level
      level++;
    }
  
    //Winner Msg
    if(gameMode == 3 && (level==4||(a==false&&b==false))){//either both lose or one of the players win
      if(player1Pts>player2Pts){//player 1 scores higher
        Serial.println("Player 1 wins!");
        Serial.write('E');
      }
      else if(player1Pts<player2Pts){//player 2 scores higher
        Serial.println("Player 2 wins!");
        Serial.write('F');
      }
      //Reset game
      resetGame();//both lost
    }
}

