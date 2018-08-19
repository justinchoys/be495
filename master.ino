
/*******************************************
 * HMS Sorry Game  (MASTER v5)
 ********************************************/

#include <SoftwareSerial.h>
SoftwareSerial BTSerial(8, 9); // RX | TX

int currentPlayer = -1; //indicator for current player’s number (0,1,2,3)
int currentRoll = 0; //current dice roll
int homePosition[] = {24, 6, 12, 18}; //array of home positions, will never change
float currentPosition[] = { -0.5, 5.5, 11.5, 17.5}; //array of current positions, will be updated
bool checkTarget = false; // 1 if next box has target in it, 0 if not
int checkTargetNumber = 0; //sparki number of target (0-3)
int targetMovesLeft = 0; //moves left to get home for target
bool almostDone[] = {false, false, false, false};
bool firstRoll[] = {true, true, true, true};

//delay times in milliseconds
int movefwd3 = 11533;
int moveout = 12128;
int movein = 11690;
int movecorner = 23247;
int deadgohome = 12500;
int deadathome = 23500;
int leavecastletakeover = 12533;
int leavecastle = 23223;
int win_at_home = 23500;
int win_go_home = 23500;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  DDRC = DDRC | B00111111;  // Sets A0-A5 as outputs
  pinMode(4, INPUT_PULLUP); //input #1
  pinMode(5, INPUT_PULLUP); //input #2
  pinMode(6, INPUT_PULLUP); //input #3
  pinMode(7, INPUT_PULLUP); //input #4
  for (int i = 1; i < 7; i++) {
    sendChar(i, 'p');
    delay(500);
  }
  Serial.println("ready to start game");
}

void loop() {
  delay(100);
  currentPlayer = (currentPlayer + 1) % 4; //update current player number
  Serial.println("ready to roll dice");
  //while (digitalread(currentPlayer + 4) == 1) {} //currentPlayer is 0-3, digitalRead is 4-7; prompt button
  currentRoll = random(1, 5); //roll dice

  String stringOne = "Player ";//Serial check to see if player pressed to roll dice
  String stringTwo = stringOne + (currentPlayer + 1) + " rolled a " + currentRoll;
  Serial.println(stringTwo);

  updateDisplay();

  if (firstRoll[currentPlayer]) {
    if (currentRoll > 2) { //first roll has to be 3 or higher to get out of castle
      Serial.println("entered initial move test");
      homeCheckTargetAvailable(); //check if target next to home (@ceiling check)
      Serial.println("finished homechecktargetavailable");
      //while (digitalread(currentPlayer + 4) == 1) {} //prompt button to move
      Serial.println("button is pressed");
      if (checkTarget) { //if target is next to home
        Serial.println("sending target home");
        sendTargetHome();
      } else {
        sendChar(currentPlayer + 1, 'h'); //if no target is next to home send leave castle
        delay(leavecastle);
      }
      currentPosition[currentPlayer] = currentPosition[currentPlayer] + 0.5; //update position
      firstRoll[currentPlayer] = false; //update current player first roll
      Serial.println("exit initial move test");
    }
  } else if (almostDone[currentPlayer]) {
    Serial.println("entered almostDone");
    if (currentPosition[currentPlayer] + currentRoll < homePosition[currentPlayer] + 1) { //if roll is not finishing move
      move();
    } else if (currentRoll == 1 && currentPosition[currentPlayer] == homePosition[currentPlayer]) { //if currentplayer is at home and player rolled 1
      //while (digitalread(currentPlayer + 4) == 1) {} //prompt button for winning
      sendChar(currentPlayer + 1, 'i'); //send win at home
      Serial.println("win_at_home sent");

      firstRoll[currentPlayer] = true; //reset target firstroll
      currentPosition[currentPlayer] = homePosition[currentPlayer] % 24 - 0.5; //update currentplayer position
      almostDone[currentPlayer] = false; //update almostDone to false
      currentRoll = 100; //adjust display to report WIN if currentRoll = 100
      updateDisplay();
      Serial.println("blueberry");

    } else if (currentPosition[currentPlayer] + currentRoll == homePosition[currentPlayer] + 1) { //going to win after moving currentRoll
      //player wins (code needs fleshing out: win_castle when it gets to homePosition)
      for (int movesLeft =  homePosition[currentPlayer] - currentPosition[currentPlayer] - 1; movesLeft > 0; movesLeft--) {
        //while (digitalread(currentPlayer + 4) == 1) {}//prompt button for each move_fwd
        moveOne(currentPlayer);
        updatePosition(currentPlayer);
      }
      //while (digitalread(currentPlayer + 4) == 1) {}//prompt button for win_go_home (last move)
      Serial.println("sending win go home");
      sendChar(currentPlayer + 1, 'j'); //send win_go_home

      firstRoll[currentPlayer] = true; //reset target firstroll
      currentPosition[currentPlayer] = homePosition[currentPlayer] % 24 - 0.5; //update target position
      almostDone[currentPlayer] = false; //update almostDone to false
      currentRoll = 100; //adjust display to report WIN if currentRoll = 100
      updateDisplay();
      Serial.println("blueberry");
    }
  } else {
    Serial.println("normal move");
    move();
  }

  Serial.println("end of turn");
  
  String stringA = "Current positions:";

  for (int i = 0; i < 4; i = i + 1) {
    stringA = stringA + "  "+ currentPosition[i] +"|";
  }

  Serial.println(stringA);
  Serial.println("-------------------");  
}

void sendChar(int sparkiNum, char letter) {
  PORTC = PORTC & B11000000; // Clear A0-A5. Direct write to PORTC.
  PORTC = PORTC | (1 << (sparkiNum - 1)); // Put a "1" in a single position, A0-A5, to enable that bluetooth module.
  BTSerial.write(letter); // Send the character.
}

void move() {
  checkTargetAvailable();// checks if there is a target, if there is, extracts target number
  Serial.println("checkTargetavailable done");
  //while (digitalread(currentPlayer + 4) == 1) {} //button prompt to move out
  Serial.println("button pressed");
  sendChar(currentPlayer + 1, 'b'); //send move_out to currentPlayer
  Serial.println("move out sent");
  delay(moveout);//delay by move_out duration

  for (int movesLeft = currentRoll - 1; movesLeft > 0; movesLeft--) { //currentRoll - 1 because last move can be takeover or normal moveOne
    //while (digitalread(currentPlayer + 4) == 1) {}  //button prompt to move
    Serial.println("button pressed");
    moveOne(currentPlayer);
    updatePosition(currentPlayer);
  }

  //while (digitalread(currentPlayer + 4) == 1) {}  //button prompt for last move
  if (checkTarget) { //if there is target, send target home (final move and movein for currentPlayer embedded in sendTargetHome)
    Serial.println("sending target home");
    sendTargetHome();
    Serial.println("target arrived at home");
  } else { //if no target, moveOne normally
    moveOne(currentPlayer);
    updatePosition(currentPlayer);
    sendChar(currentPlayer + 1, 'c'); //send move_in
    Serial.println("movein sent");
    delay(movein);//delay by move_in duration
  }

  if (homePosition[currentPlayer] - currentPosition[currentPlayer] <= 4 && homePosition[currentPlayer] - currentPosition[currentPlayer] >= 0) {
    almostDone[currentPlayer] = true; //set almostDone to true if near home
  }
}

void updateDisplay() {
  sendChar(5, (char)(((int)'0') + currentPlayer + 1)); //update LED display #1 with player #
  while (currentRoll == 100) {
    sendChar(6, 'W'); //update LED display #2 with ‘W’ (infinite loop to denote end of game)
    delay(3000);
  }
  sendChar(6, (char)(((int)'0') + currentRoll)); //update LED display #2 with currentRoll
  Serial.println("updated scoreboard");
}

void checkTargetAvailable() {
  for (int x = 0; x < 4; x++) {
    if (currentPosition[currentPlayer] + currentRoll == currentPosition[x]) {
      checkTarget = true;
      checkTargetNumber = x;
    }
  }
}

void homeCheckTargetAvailable() {
  for (int x = 0; x < 4; x++) {
    if (ceil(currentPosition[currentPlayer]) == currentPosition[x] || ceil(currentPosition[currentPlayer]) == currentPosition[x] - 24) {
      checkTarget = true; //target exists in square next to home
      checkTargetNumber = x; //target sparki number 0-3
    }
  }
}

void sendTargetHome() {
  if (homePosition[checkTargetNumber] - currentPosition[checkTargetNumber] > 1 || homePosition[checkTargetNumber] - currentPosition[checkTargetNumber] < 0) { //if target is not already at home
    sendChar(checkTargetNumber + 1, 'b'); //send move out to target sparki
    delay(moveout); //delay for move_out duration
    moveOne(checkTargetNumber);
    updatePosition(checkTargetNumber);
    if (homePosition[checkTargetNumber] - currentPosition[checkTargetNumber] > 0) { //calculate moves to get home (-1 bc last move is dead_go_home)
      targetMovesLeft = homePosition[checkTargetNumber] - currentPosition[checkTargetNumber] - 1;
    } else {
      targetMovesLeft = homePosition[checkTargetNumber] + 24 - currentPosition[checkTargetNumber] - 1;
    }

    if (firstRoll[currentPlayer]) { //currentPlayer moves
      Serial.println("sending leave castle takeover");
      sendChar(currentPlayer + 1, 'g'); //send leave_castle_takeover
    } else {
      moveOne(currentPlayer);
      updatePosition(currentPlayer);
      sendChar(currentPlayer + 1, 'c'); //send move_in
      Serial.println("currentPlayer finished takeover");
    }

    for (int tMovesLeft = targetMovesLeft; tMovesLeft > 0; tMovesLeft--) { //move target back to home
      moveOne(checkTargetNumber);
      updatePosition(checkTargetNumber);
    }

    sendChar(checkTargetNumber + 1, 'e'); //send dead_go_home before final home square
    Serial.println("dead_go_home sent to target");

  } else if (homePosition[checkTargetNumber] - currentPosition[checkTargetNumber] == 1) {
    
    sendChar(checkTargetNumber + 1, 'b'); //send move out to target sparki
    delay(moveout); //delay for move_out duration
    sendChar(checkTargetNumber + 1, 'e');
    Serial.println("dead_go_home sent to target");
    
    moveOne(currentPlayer);
    updatePosition(currentPlayer);
    sendChar(currentPlayer + 1, 'c'); //send move_in
    delay(movein);//delay by move_in duration
    Serial.println("currentPlayer finished takeover");
    
  } else if (homePosition[checkTargetNumber] == currentPosition[checkTargetNumber]) {
    sendChar(checkTargetNumber + 1, 'f');
    Serial.println("dead_at_home sent to target");
    delay(deadathome);
    moveOne(currentPlayer);
    updatePosition(currentPlayer);
    sendChar(currentPlayer + 1, 'c'); //send move_in
    delay(movein);//delay by move_in duration
    Serial.println("currentPlayer finished takeover");
  }

  firstRoll[checkTargetNumber] = true; //reset target firstroll
  currentPosition[checkTargetNumber] = (int)homePosition[checkTargetNumber] % 24 - 0.5; //update target position to home
  almostDone[checkTargetNumber] = false; //update almostDone to false
  checkTarget = false; //update checkTarget to false;
}

void moveOne(int player) {
  if ((int)(currentPosition[player] + 1) % 6 == 0) {
    sendChar(player + 1, 'd'); //send move_corner
    Serial.println("move corner sent");
    delay(movecorner); //delay move_corner duration
  } else {
    sendChar(player + 1, 'a'); //send move_fwd
    Serial.println("move fwd sent");
    delay(movefwd3); //delay move_fwd(3) duration
  }

}

void updatePosition(int player) {
  if ((int)(currentPosition[player] + 1) % 24 == 0) {
    currentPosition[player] = 24; //if next pos is 24, state 24 (not 0)
  } else {
    currentPosition[player] = (int)(currentPosition[player] + 1) % 24; //otherwise, give number 1-23
  }
}

