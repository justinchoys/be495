/*******************************************
 * HMS Sorry Game (SLAVE v4)
 ********************************************/

#include <Sparki.h>
const int threshold = 500; //line sensors thereshold.
const int linewidth = 1.27; //width of line on gameboard
const int sensorsDistance = 4.3+linewidth/2; //distance [cm] between the line sensor and the wheels centers plus half the width of the black line.
const int turnSpeed = 40; //used to turn the robot over an external center of rotation.
const int outOfTheLineAngle = 60; //used to go outside the line and then find the branches.

bool  edgeLeft = false,
lineLeft = false,
lineCenter = false,
lineRight = false,
edgeRight = false;

String state = "undefined";

void setup() {
  Serial1.begin(9600);
  //indicates to the user that the program started:
  sparki.beep(440, 300);
  delay(300);
  sparki.beep(880, 500);
  readSensors();
}

void loop() {

  //receives command from remote computer
  if(Serial1.available()) {
    int inByte = Serial1.read();
    sparki.print((char)inByte);
    sparki.updateLCD();
    switch((char)inByte) {
    case 'a':
      move_fwd(3);
      delay(1000);
      break;
    case 'b':
      move_out();
      delay(1000);
      break;
    case 'c':
      move_in();
      delay(1000);
      break;
    case 'd':
      move_corner();
      delay(1000);
      break;
    case 'e':
      dead_go_home();
      delay(1000);
      break;
    case 'f':
      dead_at_home();
      delay(1000);
      break;
    case 'g':
      leave_castle_takeover();
      delay(1000);
      break;
    case 'h':
      leave_castle();
      delay(1000);
      break;
    case 'i':
      win_at_home();
      delay(1000);
      break;
    case 'j':
      win_go_home();
      delay(1000);
      break;
    }
    sparki.clearLCD();
    sparki.updateLCD();
  }
}

void readSensors() {
  //each sensor is 1 if reading white, and 0 if reading black:
  edgeLeft =  sparki.edgeLeft() > threshold;
  lineLeft =  sparki.lineLeft() > threshold;
  lineCenter = sparki.lineCenter() > threshold;
  lineRight =  sparki.lineRight() > threshold;
  edgeRight = sparki.edgeRight() > threshold;
}

void showSensorsAndState() {
  sparki.clearLCD();

  sparki.print("eL = ");
  sparki.println(edgeLeft);
  sparki.print("lL = ");
  sparki.println(lineLeft);
  sparki.print("lC = ");
  sparki.println(lineCenter);
  sparki.print("lR = ");
  sparki.println(lineRight);
  sparki.print("eR = ");
  sparki.println(edgeRight);
  sparki.println();
  sparki.println(String("state = ") + state);

  sparki.updateLCD();
}

bool robotIsNotCentered() {
  //when centered, lineLeft reads white, lineCenter reads black, lineRight reads white;
  return !(lineLeft && !lineCenter && lineRight);
}

bool isThereALeftBranch() {
  //edgeLeft reads black:
  return !edgeLeft;
}

bool isThereARightBranch() {
  //edgeRight reads black:
  return !edgeRight;
}

void centerRobot() {
  while(robotIsNotCentered()) 
    readSensors();
    showSensorsAndState();
  }
  sparki.beep(); //beep when centered
  state = "centered";
  showSensorsAndState();
}

//void turnLeft() {
//  state = "turn left";
//  sparki.moveForward(sensorsDistance);
//  sparki.moveLeft(outOfTheLineAngle);
//  sparki.moveLeft(); //turn left until the robot is centered.
//  centerRobot();
//  sparki.moveStop();
//}

//void turnRight() {
//  state = "turn right";
//  sparki.moveForward(sensorsDistance);
//  sparki.moveRight(outOfTheLineAngle);
//  sparki.moveRight(); //turn right until the robot is centered.
//  centerRobot();
//  sparki.moveStop();
//}

void turnHardLeft() {
  state = "turn h_left";
  showSensorsAndState();
  sparki.moveLeft(outOfTheLineAngle);
  readSensors();
  sparki.moveLeft(); //turn left until the robot is centered.
  centerRobot();
  sparki.moveStop();
}

void turnHardRight() {
  state = "turn h_right";
  sparki.moveRight(outOfTheLineAngle);
  sparki.moveRight(); //turn right until the robot is centered.
  centerRobot();
  sparki.moveStop();
}

void followLine() {
  sparki.moveStop();
  state = "follow line";
  readSensors();
  showSensorsAndState();
  if (lineLeft && lineCenter) { //white white
    sparki.motorRotate(MOTOR_LEFT, DIR_CCW, 100);
  } else if (!lineLeft) {//black black
    sparki.motorRotate(MOTOR_RIGHT, DIR_CW, 100);
  } else if (lineLeft && !lineCenter) {// white black
    sparki.moveForward();
  }

  delay(200);
}

void move_fwd(int i) {
  state = "move_fwd";
  showSensorsAndState();
  int counter = 0;
  while(counter<i) {
    readSensors();
    while(edgeRight) {
      readSensors();
      followLine();
    }
    sparki.moveStop();
    sparki.beep(700,300);
    sparki.moveForward(linewidth*.5);
    while(!edgeRight) {
      readSensors();
      followLine();
    }
    counter++;
    state = String(counter);
    showSensorsAndState();
  }
  sparki.moveStop();
  sparki.moveForward(sensorsDistance-linewidth);
}

void move_out() {
  state = "move_out";
  showSensorsAndState();
  turnHardLeft();
  centerTurnLeft();
  move_fwd(1);
  turnHardRight();
  centerTurnRight();
}

void move_in() {
  state = "move in";
  showSensorsAndState();
  turnHardRight();
  centerTurnRight();
  move_fwd(1);
  turnHardLeft();
  sparki.moveStop();
  centerTurnLeft();
}

void move_corner() {
  state = "move_corner";
  showSensorsAndState();
  move_fwd(3);
  readSensors();
  turnHardRight();
  while(edgeRight) {
    followLine();
  }
  sparki.moveStop();
  sparki.moveForward(sensorsDistance);
}

void dead_go_home() {
  state = "dead_go_home";
  showSensorsAndState();
  move_fwd(3);
  turnHardLeft();
  move_fwd(1);
  sparki.moveRight(180);

}

void dead_at_home() {
  state = "dead_at_home";
  showSensorsAndState();
  turnHardLeft();
  move_fwd(1);
  turnHardLeft();
  move_fwd(2);
  sparki.moveRight(180);
}

void win_go_home() {
  state = "win_go_home";
  showSensorsAndState();
  move_fwd(2);
  turnHardLeft();
  move_fwd(1);
  sparki.moveRight(180);
}

void win_at_home() {
  state = "win_at_home";
  showSensorsAndState();
  turnHardLeft();
  turnHardLeft();
  move_fwd(1);
  turnHardRight();
  turnHardLeft();
  move_fwd(1);
  sparki.moveRight(180);
}

void leave_castle_takeover() {
  state = "leave_castle_to";
  showSensorsAndState();
  move_fwd(1);
  turnHardRight();
  move_fwd(1);
  turnHardLeft();
  move_fwd(1);
}

void leave_castle() {
  state = "leave_castle";
  showSensorsAndState();
  move_fwd(2);
  move_in();
}


void centerTurnLeft() {
  state = "turn left center";
  showSensorsAndState();
  sparki.moveLeft(); //turn right until the robot is centered.
  centerRobot();
  sparki.moveStop();
}

void centerTurnRight() {
  state = "turn right center";
  showSensorsAndState();
  sparki.moveRight(); //turn right until the robot is centered.
  centerRobot();
  sparki.moveStop();
}