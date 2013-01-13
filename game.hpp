/*
 * Snowboard! by Matthew Clark(mclark4386 @github)
 * game.hpp
 * Core Game Object (with help from sensors sdk example)
 */

#ifndef _GAME_H_
#define _GAME_H_

#include <sifteo.h>
#include "assets.gen.h"

#include "physics.hpp"
#include "cube.hpp"

using namespace Sifteo;

static Cube cubes[CUBE_ALLOCATION];

enum spriteRotation{
  TOP_SPRITE = 0,
  BOTTOM_SPRITE = 8,
  RIGHT_SPRITE = 4,
  LEFT_SPRITE =12
};
class Game {
public:
  Game():physicsClock(60),running(true),playerCubeID(0),playerPos(vec(48,1)),playerVel(vec(0,0)),gameover(false),currentSprite(1),playerRotation(TOP_SPRITE){}

  void install()
  {
    Events::neighborAdd.set(&Game::onNeighborAdd, this);
    Events::neighborRemove.set(&Game::onNeighborRemove, this);
    Events::cubeTouch.set(&Game::onTouch, this);
    Events::cubeConnect.set(&Game::onConnect, this);
    
    // Handle already-connected cubes
    for (CubeID cube : CubeSet::connected()){
      cubes[cube].setup(cube);
    }
    
    AudioTracker::play(Music);
  }
  
  void run(){
    startTime = SystemTime::now();
    TimeStep ts;

    LOG("players: %d\n",Player.numFrames());

    while(running){
      ts.next();
      
      for(int i = physicsClock.tick(ts.delta()); i;i--)
	doPhysics(physicsClock.getPeriod());
      
      draw();
      System::paint();
    }
  }
  
private:
  TimeTicker physicsClock;
  SystemTime startTime;
  bool running;
  unsigned playerCubeID;
  Float2 playerPos;
  Float2 playerVel;
  bool gameover;
  unsigned currentSprite;
  spriteRotation playerRotation;

  void onConnect(unsigned id){
    cubes[id].setup(id);
  }
  
  void onTouch(unsigned id)
  {
    CubeID cube(id);
    if(gameover&&id == playerCubeID){
      LOG("reset cube/start over");
      for(Cube& cube:cubes){
	cube.resetLayout();
      }
      startTime = SystemTime::now();
    }
    LOG("Touch event on cube #%d, state=%d\n", id, cube.isTouching());
  }

  void onNeighborRemove(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
  {
    LOG("Neighbor Remove: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

    if (firstID < arraysize(cubes)&&cubes[firstID].isValid()) {
      cubes[firstID].removeNeighborBySide(firstSide);
    }
    if (secondID < arraysize(cubes)&&cubes[secondID].isValid()) {
      cubes[secondID].removeNeighborBySide(secondSide);
    }
  }
  
  void onNeighborAdd(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
  {
    LOG("Neighbor Add: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

    if (firstID < arraysize(cubes)&&cubes[firstID].isValid()&&secondID < arraysize(cubes)&&cubes[secondID].isValid()) {
      cubes[firstID].addNeighbor(secondID,firstSide);
      cubes[secondID].addNeighbor(firstID,secondSide);
    }
  }
  
  void drawNeighbors(CubeID cube)
  {
    Neighborhood nb(cube);
    
    String<64> str;
    str << "nb "
	<< Hex(nb.neighborAt(TOP), 2) << " "
	<< Hex(nb.neighborAt(LEFT), 2) << " "
	<< Hex(nb.neighborAt(BOTTOM), 2) << " "
	<< Hex(nb.neighborAt(RIGHT), 2) << "\n";
    
  }
  
  void draw(){
    for(Cube cube:cubes){
      cube.draw();
    }
    cubes[playerCubeID].vbuf().sprites[0].setImage(Player,playerRotation+currentSprite);
    cubes[playerCubeID].vbuf().sprites[0].move(playerPos);
  }
  
  void doPhysics(float dt){
    CubeID cube(playerCubeID);

    playerVel = calcNewVelocity(playerVel,cube.accel(),dt);
    playerPos = playerPos+ playerVel;

    int side = sideOfWorldCollision(playerPos,vec(32,32));
    if(side != NO_SIDE){//we have a collision, so handle it
      LOG("collided with wall %d\n",side);
      unsigned id = cubes[playerCubeID].neighborsID(side);
      if(id == Cube::MAX_UINT){
	  GAMEOVER(false);
      }else{
	LOG("current:%d id of neighbor:%d\n",playerCubeID,id);
	//move player to new screen
	int newCubesSide = cubes[id].neighborsSide(playerCubeID);
	if(newCubesSide == NO_SIDE){
	  GAMEOVER(false);
	}
	//TODO: Check if side of new cube is open
	unsigned oldCube = playerCubeID;
	float vy = playerVel.y;
	float vx = playerVel.x;
	if(newCubesSide == TOP){;
	  switch(side){
	  case BOTTOM:
	    break;
	  case TOP:
	    playerPos.x = 128 - 32 - playerPos.x;
	    playerVel.y = -playerVel.y;
	    break;
	  case LEFT:
	    playerPos.x = playerPos.y;
	    playerVel.y = -vx;
	    playerVel.x = vy;
	    break;
	  case RIGHT:
	    playerPos.x = 128 - 32 - playerPos.y;
	    playerVel.y = vx;
	    playerVel.x = vy;
	    break;
	  }
	  playerPos.y = -10;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	if(newCubesSide == BOTTOM){
	  switch(side){
	  case BOTTOM:
	    playerPos.x = 128 - 32 - playerPos.x;
	    playerVel.y = -playerVel.y;
	    break;
	  case TOP:
	    break;
	  case LEFT:
	    playerPos.x = 128 - 32 - playerPos.y;
	    playerVel.y = vx;
	    playerVel.x = -vy;
	    break;
	  case RIGHT:
	    playerPos.x = playerPos.y;
	    playerVel.y = -vx;
	    playerVel.x = vy;
	    break;
	  }
	  playerPos.y = 100;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	if(newCubesSide == LEFT){
	  switch(side){
	  case BOTTOM:
	    playerVel.x = vy;
	    playerVel.y = -vx;
	    playerPos.y = 128 - 32 - playerPos.x;
	    break;
	  case TOP:
	    playerPos.y = 128 - 32 - playerPos.x;
	    playerVel.x = -vy;
	    playerVel.y = vx;
	    break;
	  case LEFT:
	    playerPos.y = 128 - 32 - playerPos.y;
	    playerVel.x = -vx;
	    playerVel.y = -vy;
	    break;
	  case RIGHT:
	    break;
	  }
	  playerPos.x = -10;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	if(newCubesSide == RIGHT){
	  switch(side){
	  case BOTTOM:
	    playerPos.y = playerPos.x;
	    playerVel.x = -vy;
	    playerVel.y = vx;
	    break;
	  case TOP:
	    playerVel.x = vy;
	    playerVel.y = -vx;
	    playerPos.y = 128 - 32 -playerPos.x;
	    break;
	  case LEFT:
	    break;
	  case RIGHT:
	    playerPos.y = 128 - 32 - playerPos.y;
	    playerVel.x = -vx;
	    playerVel.y = -vy;
	    break;
	  }
	  playerPos.x = 100;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	//reprepose oldCube
	cubes[oldCube].resetLayout(startTime-SystemTime::now());
      }
    }
    
    // LOG("PHYSICS: %s",str.c_str());
  }


  void GAMEOVER(bool win){
    //TODO: fill this out a bit more
    if(win){
      LOG("WIN!");
    }else{
      LOG("Game Over!");
    }
  }
  
};

#endif /* _GAME_H_ */
