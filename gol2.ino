#include <Wire.h> 
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

/*
 * gameoflife. A Conway's game of life implementation for Arduino and MAX7219 
 * controlled led matrix.
 * 
 * Copyright (C) 2014  Rafael Bailón-Ruiz <rafaelbailon "en" ieee "punto" org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "LedControl.h"



/*
 pin 10 is connected to the DataIn 
 pin 8 is connected to the CLK 
 pin 9 is connected to LOAD 
 One MAX72XX.
 */
LedControl lc=LedControl(10,8,9,1);
long screenupdate = millis();
long looptime = micros();
long elapsedloop = 0;
unsigned long gen = 0;
unsigned long maxGen = 100000;
unsigned long delaytime=200;
boolean resetit = false;
unsigned long deadcells=0;

int NUMROWS = 8;
int NUMCOLS = 8;

int gameBoard[] =  { 2, 4, 7, 0, 0, 0, 0, 0 };
int newGameBoard[] =  { 0, 0, 0, 0, 0, 0, 0, 0 };
int oldGameBoard[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int olderGameBoard[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int oldestGameBoard[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int olderthanoldestGameBoard[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
//LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the default I2C bus address of the backpack-see article


void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,5);
  /* and clear the display */
  lc.clearDisplay(0);
  lcd.begin (20,4); // 16 x 2 LCD module
  lcd.setBacklightPin(3,POSITIVE); // BL, BL_POL
   lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
   lcd.print("BORN TO DIE WORLD IS");
   lcd.setCursor(0,1);
   lcd.print("A FUCK Kill Em All");
   lcd.setCursor(0,2);
   lcd.print("I am MATTYZCAST");
   lcd.setCursor(10,4);
   lcd.print("DEAD CELLS");
   delay(2000);
  //resetMap();
}

void resetMap()
{
  randomSeed(analogRead(0));
  //for (int q=5; q >= 0; q--) {
  //  lc.setIntensity(0,q);
  //  delay(200);
  //}
  delay(3000);
  //lc.setIntensity(0,5);

  for(int x = 0; x < NUMROWS; ++x)
  {
    gameBoard[x] = random(256);
  }
}

void nextGeneration() {
  int up;
  
  for(int x = 0; x < NUMROWS; ++x)
  {

    for(int y = 0; y < NUMCOLS; ++y)
    {
      int sum = sumNeighbours(gameBoard, NUMROWS, x, y);
      if(bitRead(gameBoard[x],y) == 1) //If Cell is alive
      {
        if(sum < 2 || sum > 3) //Cell dies
        {
          bitClear(newGameBoard[x],y);
          deadcells++;
        }
        else
        {
          bitSet(newGameBoard[x],y);
        }
      }
      else //If Cell is dead
      {
        if(sum == 3) //A new Cell is born
        {
          bitSet(newGameBoard[x],y);
        }
      }
    }
  }
  if((compareArray(newGameBoard, gameBoard, NUMROWS) == 0) || (compareArray(newGameBoard, oldGameBoard, NUMROWS) == 0) || (compareArray(newGameBoard, olderGameBoard, NUMROWS) == 0) || (compareArray(newGameBoard, oldestGameBoard, NUMROWS) == 0) || (compareArray(newGameBoard, olderthanoldestGameBoard, NUMROWS) == 0))
  {
        deathcount();
        resetit = true;
    resetMap();
  }
  copyArray(oldestGameBoard, olderthanoldestGameBoard, NUMROWS);
  copyArray(olderGameBoard, oldestGameBoard, NUMROWS);
  copyArray(oldGameBoard, olderGameBoard, NUMROWS);
  copyArray(gameBoard, oldGameBoard, NUMROWS);
  copyArray(newGameBoard, gameBoard, NUMROWS);
  
  
}

int sumNeighbours(int matrix[], int matrixLength, int x, int y)
{
  int sum = 0;
  
  for (int ix = -1; ix <=1; ++ix)
  {
    for (int iy = -1; iy <=1; ++iy)
    {
      if(x + ix < 0 || x + ix > matrixLength - 1 || y + iy < 0 || y + iy > 7 || (ix == 0 && iy == 0))
      {
        continue;
      }
      
      bitRead(matrix[x + ix], y + iy) == 1 ? ++sum : 0;
    }
  }
  
  return sum;
}

void copyArray(int *from, int *to, int length)
{
  for(int i = 0; i < length; ++i)
  {
    to[i] = from[i];
  }
}

/* Return 0 if they are equal, else 1 */
int compareArray(int *first, int *second, int length)
{
  for(int i = 0; i < length; ++i)
  {
    if(first[i] != second[i])
    {
      return 1;
    }
  }
  return 0;
}

void rows(boolean wait) {
  for(int row=0;row<8;row++)
  {
    lc.setRow(0,row,gameBoard[row]);
    
  }
  if(wait == true){
    delay(delaytime);
  }
}

void deathcount() {
  lcd.setCursor(0,3);
  lcd.print("          ");
  lcd.setCursor(0,3);
  lcd.print(deadcells);
  lcd.setCursor(0,2);
  lcd.print("                    ");
  lcd.setCursor(0,2);
  lcd.print(gen);
}
void loop() { 
  if(resetit)
  {
    gen = 0;
    resetMap();
    resetit = false;
    rows(true);
  }else
  {
    rows(true);
  }
  nextGeneration();
 
  gen++;
  if (screenupdate + 1000 <= millis()) {
  deathcount();
  screenupdate = millis();  
  }
  elapsedloop = micros() - looptime;
  looptime = micros();
}
