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
static TiltShakeRecognizer motion[CUBE_ALLOCATION];

class Game {
public:
  Game():physicsClock(60),running(true),playerCubeID(0),playerPos(vec(48,1)),playerVel(vec(0,0)){}
  
  struct CubeStats {
    Vector3<float> acceleration;
    unsigned touch;
    unsigned neighborAdd;
    unsigned neighborRemove;
  } cubeStats[CUBE_ALLOCATION];

  void install()
  {
    Events::neighborAdd.set(&Game::onNeighborAdd, this);
    Events::neighborRemove.set(&Game::onNeighborRemove, this);
    Events::cubeAccelChange.set(&Game::onAccelChange, this);
    Events::cubeTouch.set(&Game::onTouch, this);
    Events::cubeConnect.set(&Game::onConnect, this);
    
    // Handle already-connected cubes
    for (CubeID cube : CubeSet::connected()){
      cubes[cube].setup(cube);
    }
    
    AudioTracker::play(Music);
  }
  
  void run(){
    TimeStep ts;
    
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
  bool running;
  unsigned playerCubeID;
  Float2 playerPos;
  Float2 playerVel;

  void onConnect(unsigned id){
    cubes[id].setup(id);
  }
  
  void onTouch(unsigned id)
  {
    CubeID cube(id);
    cubeStats[id].touch++;
    LOG("Touch event on cube #%d, state=%d\n", id, cube.isTouching());
  }
  
  void onAccelChange(unsigned id)
  {
    CubeID cube(id);
    auto accel = cube.accel();
    
    String<64> str;
    str << "acc: "
	<< Fixed(accel.x, 3)
	<< Fixed(accel.y, 3)
	<< Fixed(accel.z, 3) << "\n";
    
    str << "pos: "
	<< Fixed(playerPos.x, 3)
	<< Fixed(playerPos.y, 3) << "\n";
    
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
    
    switch(firstSide){
    case TOP:
      LOG("first top\n");
      break;
    case BOTTOM:
      LOG("first Bottom\n");
      break;
    case LEFT:
      LOG("first left\n");
      break;
    case RIGHT:
      LOG("first right\n");
      break;
    }
    
    switch(secondSide){
    case TOP:
      LOG("2 top\n");
      break;
    case BOTTOM:
      LOG("2 Bottom\n");
      break;
    case LEFT:
      LOG("2 left\n");
      break;
    case RIGHT:
      LOG("2 right\n");
      break;
    }

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
    cubes[playerCubeID].vbuf().sprites[0].setImage(Player,1);
    cubes[playerCubeID].vbuf().sprites[0].move(playerPos);
  }
  
  void doPhysics(float dt){
    CubeID cube(playerCubeID);

    // String<128> str;

    // str <<"\n\tv("
    // 	<<FixedFP(playerVel.x,1,3)<<", "<<FixedFP(playerVel.y,1,3)<<")\n\ta("
    // 	<<FixedFP(cube.accel().x,1,3)<<", "<<FixedFP(cube.accel().y,1,3)<<", "<<FixedFP(cube.accel().z,1,3)<<")\n\tdt:"
    // 	<<FixedFP(dt,1,3)<<", p("
    // 	<<FixedFP(playerPos.x,1,3)<<", "<<FixedFP(playerPos.y,1,3)<<")\n";

    // auto acc = static_cast<Float3>(cube.accel()).normalize();
    // str<<"na("
    //    <<FixedFP(acc.x,1, 3)<<", "<<FixedFP(acc.y,1, 3)<<", "<<FixedFP(acc.z,1, 3)<<")\n";

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
	if(newCubesSide == TOP){
	  playerPos.y = -10;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	if(newCubesSide == BOTTOM){
	  playerPos.y = 100;
	  playerVel.y = -playerVel.y;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	if(newCubesSide == LEFT){
	  playerPos.x = -10;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
	if(newCubesSide == RIGHT){
	  playerPos.x = 100;
	  playerVel.x = -playerVel.y;
	  cubes[playerCubeID].vbuf().sprites.erase();
	  playerCubeID = id;
	  return;
	}
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
