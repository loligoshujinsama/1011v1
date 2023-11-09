////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Copyright 2015 TheYoHanSolo

#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>

#define	BLACK           0x00
#define	BLUE            0xE0
#define	RED             0x03
#define	GREEN           0x1C
#define WHITE           0xFF
#define	GREY            0x6D

// Constant variables for the paddles
//
int kPaddleStep = 1;
int kLoopDelay = 0;
int kPaddleWidth = 2;
int kPaddleHeight = 15;
int kStickSensitivity = 100;

int kScreenTop = 0;
int kScreenLeft = 0;
int kScreenWidth = 96;
int kScreenHeight = 64;
int kScreenRight = kScreenWidth-1;
int kScreenBottom = kScreenHeight-1;

//Paddle Location
//
int kLPaddleX = 10;
int kRPaddleX = kScreenRight - kLPaddleX;

TinyScreen display = TinyScreen(0);

//Button Variables (Top 4 are x and y axis of joysticks)
int RX=0;
int RY=0;
int LX=0;
int LY=0;

byte leftButton=0;
byte rightButton=0;

//Paddle Variable Values
//
int paddleX = 10;
int lPaddleY = 10;
int rPaddleY = 10;


// Constant variables for the ball.
//
int kBallWidth = 4;
int kBallHeight = 4;
int ballX = 48;
int ballY = 32;
int bMovementX = 1;
int bMovementY = 1;
int kRight = 1;
int kLeft = -1;

//Score Values
int leftPlayerScore  = 0;
int rightPlayerScore = 0;

//Goal Start and end positions for the 3 modes of pong
int kTennisGoalUpper   = 0;
int kTennisGoalLower   = 64;

int kHandballGoalUpper = 0;
int kHandballGoalLower = 64;

int kHockeyGoalUpper   = 22;
int kHockeyGoalLower   = 42;


//Three modes of pong
enum { kTennisMode = 0, kHandballMode, kHockeyMode , kMaxModes };
int mode = kTennisMode;

//Initial value of button before being pressed
int preLeftButton  = 0;
int preRightButton = 0;

//Pausing booleans
 bool prevPaused = false;
 bool paused     = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup(void) 
{
    // Initilize the WIRE interface used to talk to the graphics chip.
    Wire.begin();
  
    // Initialize the serial interface for output of debug strings.
    Serial.begin(9600);
  
    // Initialize the display library so we can draw.
    display.begin();
  
    // Select the font we will use for scoring and other messages.
    display.setFont(liberationSans_8ptFontInfo);
  
    // Default to displaying white text on a black background.
    display.fontColor(WHITE,BLACK);
  
} // setup(void)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() 
{
  getJoysticks();
  bool moveLeftPaddleUp = digitalRead(display.getButtons(TSButtonUpperLeft)) == LOW; // Replace with the actual pin number
  bool moveLeftPaddleDown = digitalRead(display.getButtons(TSButtonLowerLeft)) == LOW; // Replace with the actual pin number
  bool moveRightPaddleUp = digitalRead(display.getButtons(TSButtonUpperRight)) == LOW; // Replace with the actual pin number
  bool moveRightPaddleDown = digitalRead(display.getButtons(TSButtonLowerRight)) == LOW; // Replace with the actual pin number
 // Pausing function of the left button(Is left button equal to the previous value?
   if( leftButton && !preLeftButton)
   {
       paused = !paused;
       if( paused && !prevPaused )
       {
           // Sets color and location of the "PAUSED" label and resets the color back to default
           display.fontColor(RED,BLACK);
           display.setCursor(31, 25);
           display.print("PAUSED");   
           display.fontColor(WHITE, BLACK);
       }
       //Resets when pushed again
       if( !paused && prevPaused )
     {
         //Calls the clear screen function to wipe the screen of the "PAUSED"
         clearScreen();
     }  
   }
   
   prevPaused = paused;
   preLeftButton = leftButton;
 //Tells program to halt 
   if( paused)
  {
    return;
  }
   
  //Asks if the right button has been pressed before and if it's something else 
  if( rightButton && !preRightButton)
  {
    //If the ball hits the goal, it stops and resets back to the center of the field and gets tossed to start the game
    if(bMovementX == 0)
   {
     drawBall(ballX, ballY, kBallWidth, kBallHeight, BLACK); 
     ballX = 48;
     ballY = 32;
     bMovementX = -1;
     bMovementY = 1;
     
     //When the mode is handball, make sure the right score is reset every time it's selected
     if( mode == kHandballMode)
     {
       rightPlayerScore = 0;
     }
   }
   
   else
   {
    mode = mode+1;
    //If the mode is called and the number is higher than the 3 modes, reset it back to tennis and start again
    if( mode == kMaxModes)
    {
      mode = kTennisMode;
    }
    
    //Full reset, screen clear, score reset, ball reset, and movement reset
    clearScreen();
    leftPlayerScore = 0;
    rightPlayerScore = 0;
    ballX = 48;
    ballY = 32;
    bMovementX = 1;
    bMovementY = 1;
   }
  }
  preRightButton = rightButton;
  
 
   
  // Update the Left Paddle with the new position.
  //
  if( mode != kHandballMode)
  {
    paddleUpdate(moveLeftPaddleUp, moveLeftPaddleDown, kLPaddleX, lPaddleY);
  }
  // Update the Right Paddle with the new position.
  //
  paddleUpdate(moveRightPaddleUp, moveRightPaddleDown, kRPaddleX, rPaddleY);
 
 
  ballUpdate( ballX, ballY);
  
  //Score Counter display on the screen
  //
  display.setCursor(60, 3);
  display.print(rightPlayerScore);
  
  if( mode != kHandballMode)
  {
    display.setCursor(36, 3);
    display.print(leftPlayerScore);
  }
  
  displayField();
  // Limit the animation speed.
  //
  delay(kLoopDelay);
  
} // loop()


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Function that clears the screen when called without and weird lines that show up without a delay
void clearScreen()
{
  display.clearWindow(kScreenLeft, kScreenTop, kScreenWidth, kScreenHeight );
  delayMicroseconds(50);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//This function draws the playing field based on which mode is selected
void displayField()
{
  //Is it handball?
  if( mode == kHandballMode)
  {
    //It's handball, so draw the a line on the top, bottom, and left sides of the screen
    display.drawLine(kScreenLeft,kScreenTop,kScreenLeft,kScreenBottom,WHITE);
    display.drawLine(kScreenLeft,kScreenTop,kScreenRight,kScreenTop,WHITE);
    display.drawLine(kScreenLeft,kScreenBottom,kScreenRight,kScreenBottom,WHITE);
  }
  
  //Is it tennis?
  if( mode == kTennisMode)
  {
    //Yes, so draw a top and bottom line and make a center field line
    display.drawLine(kScreenLeft,kScreenTop,kScreenRight,kScreenTop,WHITE);
    display.drawLine(kScreenLeft,kScreenBottom,kScreenRight,kScreenBottom,WHITE);
    display.drawLine(kScreenWidth/2, kScreenTop, kScreenWidth/2, kScreenBottom, GREY);
  }
  
   //Is it Hockey?
   if( mode == kHockeyMode)
  {
    //Yes so draw the goal lines, the center divide, and the top and bottom lines
    display.drawLine(kScreenLeft,kScreenTop,kScreenRight,kScreenTop,WHITE);
    display.drawLine(kScreenLeft,kScreenBottom,kScreenRight,kScreenBottom,WHITE);
    display.drawLine(kScreenWidth/2, kScreenTop, kScreenWidth/2, kScreenBottom, GREY);
    display.drawLine(kScreenLeft,kScreenTop,kScreenLeft,kScreenBottom/2 - 10,WHITE);
    display.drawLine(kScreenLeft,kScreenBottom,kScreenLeft,kScreenBottom/2 + 10,WHITE);
    display.drawLine(kScreenRight,kScreenTop,kScreenRight,kScreenBottom/2 - 10,WHITE);
    display.drawLine(kScreenRight,kScreenBottom,kScreenRight,kScreenBottom/2 + 10,WHITE);
  }
  
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//This function determines the properties of the paddle and creates its movement
void paddleUpdate(bool moveUp, bool moveDown, int positionX, int& positionY)
{
  // Move the paddle up if the "move up" button is pressed
  if(moveUp)
  {
    positionY = positionY - kPaddleStep;

    // Check if the paddle has moved off the top of the screen
    if(positionY < kScreenTop)
    {
      positionY = kScreenTop;
    }
  }

  // Move the paddle down if the "move down" button is pressed
  if(moveDown)
  {
    // Check if the paddle is at the bottom of the screen
    if(positionY >= kScreenBottom - kPaddleHeight)
    {
      positionY = kScreenBottom - kPaddleHeight;
    }
    else
    {
      // Draw over the old position of the paddle
      drawPaddle(positionX, positionY, kPaddleWidth, kPaddleHeight, BLACK);
      positionY = positionY + kPaddleStep;
    }
  }
  // Redraw the paddle at its new position
  drawPaddle(positionX, positionY, kPaddleWidth, kPaddleHeight, WHITE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Code for ball movement
//
void ballUpdate(int& positionX, int& positionY)
{
  int upperGoal;
  int lowerGoal;
  //Draw the ball black
    drawBall(positionX, positionY, kBallWidth, kBallHeight, BLACK);
  
  //The movement on the screen
    positionX = positionX + bMovementX;
    positionY = positionY + bMovementY;
    
    //The ball being drawn in white
    drawBall(positionX, positionY, kBallWidth, kBallHeight, WHITE);
    
    //Delay of one millisecond in the drawing
    
    
    /*if( positionX = positionX - bMovementX + 1)
    {
      drawBall(positionX, positionY, kBallWidth, kBallHeight, BLACK);
    }
    
    if( positionX = positionX - bMovementX - 1)
    {
      drawBall(positionX, positionY, kBallWidth, kBallHeight, BLACK);
    }
    */
    //Is the ball at the right side of the screen?
   
   //Will switch between what mode it is and determines the properties of the right wall
    switch(mode)
    {
      //Tennis goal boundaries
      case kTennisMode: 
        upperGoal = kTennisGoalUpper;
        lowerGoal = kTennisGoalLower;
      break;
      //Handball goal boundaries
      case kHandballMode:
        upperGoal = kHandballGoalUpper;
        lowerGoal = kHandballGoalLower;
      break;
      //Hockey goal boundaries
      case kHockeyMode:
        upperGoal = kHockeyGoalUpper;
        lowerGoal = kHockeyGoalLower;
      break;
    }
    
    //Is the ball at the right side of the screen?
    if( positionX >= kScreenRight - kBallWidth )
    {
      //Is the ball in the goal area?
      if( positionY > upperGoal && positionY + kBallHeight < lowerGoal )
      {
        //Is the mode handball and the ball is in the same position as the paddle?
        if( mode != kHandballMode && bMovementX != 0)
        {
          //Yes, so increase the score
          leftPlayerScore++;
        }
        //Stops the ball from moving after missing the paddle
        bMovementX = 0;
        bMovementY = 0;
      }      
     //Reverses the movement
      bMovementX = -bMovementX;
      
    }
   //Goal boundaries changes between games
   switch(mode)
    {
      //Tennis goal boundaries
      case kTennisMode: 
        upperGoal = kTennisGoalUpper;
        lowerGoal = kTennisGoalLower;
      break;
      //Handball Goal boundaries 
      case kHandballMode:
        upperGoal = kHandballGoalUpper;
        lowerGoal = kHandballGoalLower;
      break;
      //Hockey goal boundaries
      case kHockeyMode:
        upperGoal = kHockeyGoalUpper;
        lowerGoal = kHockeyGoalLower;
      break;
    }
     
    //Is the ball at the left side of the screen?
    if( positionX <= kScreenLeft)
    {
      //Is the ball within the goal boundaries?
      if( positionY > upperGoal && positionY + kBallHeight < lowerGoal )
      {
        //Is the mode not handball?
        if( mode != kHandballMode && bMovementX != 0)
        {
          //Then increase right player score
          rightPlayerScore++;
        }
        //Is the mode not handball?      
        if( mode != kHandballMode)
        {
          //Then stop the ball from moving
          bMovementX = 0;
          bMovementY = 0;
        }
      }
      //Yes, reverse direction and increase right's score.
      bMovementX = -bMovementX;
      
    }
    
    // Collisions for the bottom
    
    if( positionY >= kScreenBottom - kBallWidth)
    {
      bMovementY = -bMovementY;
    }
    
    //Collision detection for the top
    
    if( positionY <= kScreenTop)
    {
      bMovementY = -bMovementY;
    }
    
    //Is the ball at the same row as the paddle?
   if( mode != kHandballMode){
    if( positionX == paddleX)
    {
      //Is it at the same position as the paddle?
      if( positionY >= lPaddleY - kBallHeight/2 && positionY <= lPaddleY + kPaddleHeight - kBallHeight/2)
      {
        //Yes so change direction
        bMovementX = -bMovementX;
      }
    }
   }
    //Is the ball at the same row as the paddle?
     if( positionX == kScreenRight - paddleX)
    {
      
      //Is it at the same position as the paddle?
      if( positionY >= rPaddleY - kBallHeight/2 && positionY <= rPaddleY + kPaddleHeight - kBallHeight/2)
      {
        
         //Yes so change direction
        bMovementX = -bMovementX;
        if(mode == kHandballMode)
        {
          rightPlayerScore++;
        }
      }
    }
    
    
    
   //Ball Drawing code for use on the screen
    
    drawBall( ballX, ballY, kBallWidth, kBallHeight, WHITE);
} //void ballUpdate


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Code for drawing the basic shape of the paddle

void drawPaddle(int x, int y, int width, int height, byte color)
{
  for(int i=0;i<=height;i++)
  {
    display.drawLine(x,y+i,x+width,y+i,color);
  }
  //delayMicroseconds(50);
} //void drawPaddle


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Code for drawing the basic shape of the ball
void drawBall(int x, int y, int width, int height, byte color)
{
  for(int i=0;i<width;i++)
  {
    display.drawLine(x,y+i,x+width,y+i,color);
    
  }
  //delayMicroseconds(50);
}//void drawBall


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Joystick code for use by the paddles
void getJoysticks()
  {
    leftButton = display.getButtons(TSButtonUpperLeft);
    rightButton = display.getButtons(TSButtonUpperRight);

  }