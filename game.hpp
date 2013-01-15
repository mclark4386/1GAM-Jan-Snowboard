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
enum spriteType{
  TURN_RIGHT_SPRITE = 0,
  SLOW_SPRITE,
  TURN_LEFT_SPRITE,
  FAST_SPRITE
};
class Game {
public:
  Game():physicsClock(60),running(true),playerCubeID(0),playerPos(vec(48,1)),playerVel(vec(0,0)),gameover(false),currentSprite(SLOW_SPRITE),playerRotation(TOP_SPRITE){}

  void install()
  {
    Events::neighborAdd.set(&Game::onNeighborAdd, this);
    Events::neighborRemove.set(&Game::onNeighborRemove, this);
    Events::cubeTouch.set(&Game::onTouch, this);
    Events::cubeConnect.set(&Game::onConnect, this);
    
    // Handle already-connected cubes
    for (CubeID cube : CubeSet::connected()){
      onConnect(cube);
    }
    
    AudioTracker::play(Music);
  }
  
  void run(){
    startTime = SystemTime::now();
    TimeStep ts;

    LOG("players: %d\n",Player.numFrames());

    while(running){
      ts.next();
      
      if(!gameover){
	for(int i = physicsClock.tick(ts.delta()); i;i--)
	  doPhysics(physicsClock.getPeriod());
      
	draw();
      }

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
  spriteType currentSprite;
  spriteRotation playerRotation;
  String<16> timerText;

  void onConnect(unsigned id){
    cubes[id].setup(id);
    cubes[id].vbuf().bg1.setMask(BG1Mask::filled(vec(0,12),vec(16,4)));
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
      playerPos = vec(48,1);
      playerVel = vec(0,0);
      cubes[playerCubeID].vbuf().bg1.text(vec(0,12),Font,"               ");
      cubes[playerCubeID].vbuf().bg1.text(vec(0,14),Font,"               ");
      currentSprite = SLOW_SPRITE;
      System::paint();
      gameover = false;
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
	float timertime = SystemTime::now()-startTime;
	// LOG("timertime: %f",timertime);

	int hours = timertime/60/60;
	int minutes = (timertime-(hours*60*60))/60;

	float seconds = timertime-(hours*60*60)-(minutes*60);

	timerText.clear();//make sure it's ready for next time
	timerText<<Fixed(hours,3)<<":"<<Fixed(minutes,2)<<":"<<FixedFP(seconds,2,3);

	for(Cube cube:cubes){
	  cube.draw();
	  if(cube.id() != playerCubeID && cube.id() != Cube::MAX_UINT){  
	    cubes[cube.id()].vbuf().bg1.text(vec(0,14),Font,timerText);
	    cubes[cube.id()].vbuf().bg1.setPanning(vec(-15,56));	
	  }
	}

	cubes[playerCubeID].vbuf().sprites[0].setImage(Player,playerRotation+currentSprite);
	cubes[playerCubeID].vbuf().sprites[0].move(playerPos);
  }
  
  void doPhysics(float dt){
    CubeID cube(playerCubeID);

    playerVel = calcNewVelocity(playerVel,cube.accel(),dt);
    playerPos = playerPos+ playerVel;

    static const float kSpriteTypeWeight = 2.0f; 

    if(playerRotation == TOP_SPRITE){// down is (0,1)
      if(playerVel.x >= kSpriteTypeWeight/2){//half the weights on turning, so that they show sooner
	currentSprite = TURN_RIGHT_SPRITE;
      }else if(playerVel.x <= -kSpriteTypeWeight/2){
	currentSprite = TURN_LEFT_SPRITE;
      }else if(playerVel.y >= kSpriteTypeWeight){
	currentSprite = FAST_SPRITE;
      }else{
	currentSprite = SLOW_SPRITE;
      }
    }else if(playerRotation == BOTTOM_SPRITE){//down is (0,-1);
      if(playerVel.x >= -kSpriteTypeWeight/2){
	currentSprite = TURN_RIGHT_SPRITE;
      }else if(playerVel.x <= kSpriteTypeWeight/2){
	currentSprite = TURN_LEFT_SPRITE;
      }else if(playerVel.y <= -kSpriteTypeWeight){
	currentSprite = FAST_SPRITE;
      }else{
	currentSprite = SLOW_SPRITE;
      }
    }else if(playerRotation == LEFT_SPRITE){//down is (1,0);
      if(playerVel.y >= kSpriteTypeWeight/2){
	currentSprite = TURN_RIGHT_SPRITE;
      }else if(playerVel.y <= -kSpriteTypeWeight/2){
	currentSprite = TURN_LEFT_SPRITE;
      }else if(playerVel.x >= kSpriteTypeWeight){
	currentSprite = FAST_SPRITE;
      }else{
	currentSprite = SLOW_SPRITE;
      }
    }else if(playerRotation == RIGHT_SPRITE){//down is (-1,0);
      if(playerVel.y >= -kSpriteTypeWeight/2){
	currentSprite = TURN_RIGHT_SPRITE;
      }else if(playerVel.y <= kSpriteTypeWeight/2){
	currentSprite = TURN_LEFT_SPRITE;
      }else if(playerVel.x <= kSpriteTypeWeight){
	currentSprite = FAST_SPRITE;
      }else{
	currentSprite = SLOW_SPRITE;
      }
    }

    //check for obsticle collision
    auto hitBoxes = cubes[playerCubeID].physicsBoxes();

    for(auto it = hitBoxes.begin();it != hitBoxes.end();it++){
      if(collisionDetect(vec(playerPos.x+4,playerPos.y+4),vec(24.0f,24.0f),it->origin(),it->size())){//reduce the hit box on the player since the sprite isn't the full 32pix
	GAMEOVER(it->win);
      }
    }

    //check for world collision
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

	unsigned oldCube = playerCubeID;
	float vy = playerVel.y;
	float vx = playerVel.x;
	if(newCubesSide == TOP){;
	  playerRotation = TOP_SPRITE;
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
	}else
	if(newCubesSide == BOTTOM){
	  playerRotation = BOTTOM_SPRITE;
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
	}else
	if(newCubesSide == LEFT){
	  playerRotation = LEFT_SPRITE;
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
	}else
	if(newCubesSide == RIGHT){
	  playerRotation = RIGHT_SPRITE;
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
	}
	//reprepose oldCube
	cubes[id].vbuf().bg1.text(vec(0,14),Font,"                ");
	cubes[oldCube].resetLayout(SystemTime::now()-startTime);
      }
    }
    
    // LOG("PHYSICS: %s",str.c_str());
  }


  void GAMEOVER(bool win){
    if(!gameover){//if not already in a gameover state
      gameover = true;
      cubes[playerCubeID].vbuf().sprites.erase();
      if(win){
	//      LOG("WIN!");
	cubes[playerCubeID].vbuf().bg1.text(vec(0,12),Font,"WIN!");
	cubes[playerCubeID].vbuf().bg1.text(vec(0,14),Font,timerText);
	cubes[playerCubeID].vbuf().bg1.setPanning(vec(-24,80));
      }else{
	//      LOG("Game Over!");
	cubes[playerCubeID].vbuf().bg1.text(vec(0,12),Font,"Game Over!");
	cubes[playerCubeID].vbuf().bg1.text(vec(0,14),Font,timerText);
	cubes[playerCubeID].vbuf().bg1.setPanning(vec(-24,56));
      }
      System::paint();
    }
  }
  
};

#endif /* _GAME_H_ */
