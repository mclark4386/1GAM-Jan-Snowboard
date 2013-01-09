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
  Game():physicsClock(60),running(true),playerCubeID(0),playerPos(vec(0,0)),playerVel(vec(0,0)){}
  
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
    
    if (firstID < arraysize(cubeStats)) {
      cubeStats[firstID].neighborRemove++;
      drawNeighbors(firstID);
    }
    if (secondID < arraysize(cubeStats)) {
      cubeStats[secondID].neighborRemove++;
      drawNeighbors(secondID);
    }
  }
  
  void onNeighborAdd(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
  {
    LOG("Neighbor Add: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);
    
    if (firstID < arraysize(cubeStats)) {
            cubeStats[firstID].neighborAdd++;
            drawNeighbors(firstID);
    }
    if (secondID < arraysize(cubeStats)) {
      cubeStats[secondID].neighborAdd++;
      drawNeighbors(secondID);
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
    
    str << "   +" << cubeStats[cube].neighborAdd
	<< ", -" << cubeStats[cube].neighborRemove
	<< "\n\n";

    
  }
  
  void draw(){
    for(Cube cube:cubes){
      cube.draw();
    }
    cubes[playerCubeID].vbuf().sprites[0].setImage(Question,1);
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
    // LOG("PHYSICS: %s",str.c_str());
  }
  
};

#endif /* _GAME_H_ */
