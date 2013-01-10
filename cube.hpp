#ifndef _CUBE_H_
#define _CUBE_H_


#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

class Cube{
public:
  static const unsigned MAX_UINT = -1;

  Cube():_id(MAX_UINT),isSetup(false){for(int i=0;i<4;i++)neighbors[i]=MAX_UINT;};
  ~Cube(){};
  
  void setup(unsigned id){
    CubeID cube(id);
    _id = id;
    LOG("Cube %d connected\n", id);
    
    vid.initMode(BG0_SPR_BG1);
    vid.attach(id);
    
    //try to set up the bg
    vid.bg0.image(vec(0,0),Snow,0);
    resetLayout();
    System::paint();
    isSetup = true;
  }
  
  void draw(){
    if(isSetup&&_id != MAX_UINT){
      //      vid.bg0.image(vec(0,0),Snow);//may not be needed
    }
  }

  void resetLayout(float time = 0){
    static float timeToWaitBeforeFinish = 30000;
    static float percentChanceOfFinish = 90/100;
    if(time > timeToWaitBeforeFinish){//been playing for longer then 30seconds
      if(rand.random() >= percentChanceOfFinish){//10% chance to spawn the finish line
	//spawn finishline
	return;
      }
    }

    //not finish, but still good!
    switch(rand.randint(0,4)){
    case 0:
      //curve
    case 1:
      //straight
    case 2:
      //straight
    case 3:
      //straight
    default:
      //straight
      break;
    }
  }
  
  void addNeighbor(unsigned id, unsigned side){
    neighbors[side] = id;
  }
  
  void removeNeighborByID(unsigned id){
    for(int i = 0; i<4; i++){
      if(neighbors[i] == id){
	neighbors[i] = MAX_UINT;
	return;
      }
    }
  }
  
  void removeNeighborBySide(unsigned side){
    neighbors[side] = MAX_UINT;
  }

  unsigned neighborsID(int side){
    if(side <= NO_SIDE||side >= NUM_SIDES) return MAX_UINT;
    return neighbors[side];
  }

  int neighborsSide(unsigned id){
    for(int i = 0; i<4; i++){
      if(neighbors[i] == id){
	return i;
      }
    }
    return NO_SIDE;
  }
  
  unsigned id(){
    return _id;
  }
  
  VideoBuffer& vbuf(){
    return vid;
  }
  
  bool isValid(){
    return isSetup&&_id!=MAX_UINT;
  }
  
private:
  unsigned _id;
  VideoBuffer vid;
  bool isSetup;
  unsigned neighbors[4];
  Random rand;
};

#endif /* _CUBE_H_ */
