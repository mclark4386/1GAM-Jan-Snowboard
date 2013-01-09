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

using namespace Sifteo;

static VideoBuffer vid[CUBE_ALLOCATION];
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
    for (CubeID cube : CubeSet::connected())
      onConnect(cube);
    
    AudioTracker::play(musicTrack);
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
  char playerCubeID;
  Float2 playerPos;
  Float2 playerVel;

  void onConnect(unsigned id)
  {
    CubeID cube(id);
    uint64_t hwid = cube.hwID();
    
    bzero(cubeStats[id]);
    LOG("Cube %d connected\n", id);
    
    vid[id].initMode(BG0_SPR_BG1);
    vid[id].attach(id);
    motion[id].attach(id);
    
    //try to set up the bg
    vid[id].bg0.image(vec(0,0),Snow);//not going to work in bg0rom mode
    
    // Draw the cube's identity
    // String<128> str;
    // str << "I am cube #" << cube << "\n";
    // str << "hwid " << Hex(hwid >> 32) << "\n     " << Hex(hwid) << "\n\n";
    // vid[cube].bg0rom.text(vec(1,2), str);

    
    // Draw initial state for all sensors
    onAccelChange(cube);
    onTouch(cube);
    drawNeighbors(cube);
  }
  
  void onTouch(unsigned id)
  {
    CubeID cube(id);
    cubeStats[id].touch++;
    LOG("Touch event on cube #%d, state=%d\n", id, cube.isTouching());
    
    // String<32> str;
    // str << "touch: " << cube.isTouching() <<
    //   " (" << cubeStats[cube].touch << ")\n";
    //  vid[cube].bg0rom.text(vec(1,9), str);
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
    
    //  vid[cube].bg0rom.text(vec(1,10), str);
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
    
    BG0ROMDrawable &draw = vid[cube].bg0rom;
    //  draw.text(vec(1,6), str);
    
    drawSideIndicator(draw, nb, vec( 1,  0), vec(14,  1), TOP);
    drawSideIndicator(draw, nb, vec( 0,  1), vec( 1, 14), LEFT);
    drawSideIndicator(draw, nb, vec( 1, 15), vec(14,  1), BOTTOM);
    drawSideIndicator(draw, nb, vec(15,  1), vec( 1, 14), RIGHT);
  }
  
  static void drawSideIndicator(BG0ROMDrawable &draw, Neighborhood &nb,
				Int2 topLeft, Int2 size, Side s)
  {
    unsigned nbColor = draw.ORANGE;
    draw.fill(topLeft, size,
	      nbColor | (nb.hasNeighborAt(s) ? draw.SOLID_FG : draw.SOLID_BG));
  }
  
  void draw(){
    
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
