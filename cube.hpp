#ifndef _CUBE_H_
#define _CUBE_H_


#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

class Cube{
public:
  class hitBox{
  public:
    float originX,originY;
    float sizeX,sizeY;
    bool win;
    hitBox():originX(NAN),originY(NAN),sizeX(NAN),sizeY(NAN),win(false){};
    //hitBox(const hitBox& other):origin(other.origin),size(other.size),win(other.win){}
    hitBox(Float2 origin, Float2 size, bool win):originX(origin.x),originY(origin.y),sizeX(size.x),sizeY(size.y),win(win){}
  };
  static const unsigned MAX_UINT = -1;

  Cube():_id(MAX_UINT),isSetup(false){for(int i=0;i<4;i++)neighbors[i]=MAX_UINT;};
  ~Cube(){};
  
  void setup(unsigned id){
    CubeID cube(id);
    _id = id;
    LOG("Cube %d connected\n", id);
    
    vid.initMode(BG0_SPR_BG1);
    vid.attach(id);
    
    resetLayout();
    isSetup = true;
  }
  
  void draw(){
    if(isSetup&&_id != MAX_UINT){
      //      vid.bg0.image(vec(0,0),Snow);//may not be needed
    }
  }

  //TODO: fill this out
  void resetLayout(float time = 0){
    LOG("resetLayout\n");
    hitBoxes.clear();
    static float timeToWaitBeforeFinish = 5000;
    static float percentChanceOfFinish = 90/100;
    if(time > timeToWaitBeforeFinish){//been playing for longer then 5seconds
      if(rand.random() >= percentChanceOfFinish){//10% chance to spawn the finish line
	//spawn finishline
	vid.bg0.image(vec(0,0),Snow,0);
	hitBox win(vec(0,64),vec(128,32),true);
	hitBoxes.append(win);
	hitBox leftWall(vec(0,0),vec(14,128),false);
	hitBoxes.append(leftWall);
	hitBox rightWall(vec(114,0),vec(14,128),false);
	hitBoxes.append(rightWall);
	System::paint();
	return;
      }
    }

    Float2 tmpVec;

    //not finish, but still good!
    switch(rand.randint(0,4)){
    case 0:
      LOG("curve + obs\n");
      //curve
      tmpVec = vec(rand.randint(0,72),rand.randint(48,69));
      vid.sprites[1].setImage(Obsticles,rand.randint(0,1));
      vid.sprites[1].move(tmpVec);
      hitBoxes.push_back(hitBox(tmpVec, vec(16,16), false));
    case 1:
      LOG("curve\n");
      //curve
      vid.bg0.image(vec(0,0),SnowCurve,0);
      hitBoxes.push_back(hitBox(vec(0,0),vec(16,32),false));
      hitBoxes.push_back(hitBox(vec(114,0),vec(14,128),false));
      hitBoxes.push_back(hitBox(vec(0,114),vec(128,14),false));
      hitBoxes.push_back(hitBox(vec(102,49),vec(12,65),false));
      hitBoxes.push_back(hitBox(vec(83,63),vec(16,51),false));
      hitBoxes.push_back(hitBox(vec(53,88),vec(33,26),false));
      hitBoxes.push_back(hitBox(vec(27,103),vec(26,11),false));
      break;
    case 2:
      LOG("straight + 2Xobs\n");
      //straight
      tmpVec = vec(32,rand.randint(64,112));
      vid.sprites[1].setImage(Obsticles,rand.randint(0,1));
      vid.sprites[1].move(tmpVec);
      hitBoxes.push_back(hitBox(tmpVec, vec(16,16), false));
    case 3:
      LOG("straight + obs\n");
      //straight
      tmpVec = vec(86,rand.randint(64,112));
      vid.sprites[2].setImage(Obsticles,rand.randint(0,1));
      vid.sprites[2].move(tmpVec);
      hitBoxes.push_back(hitBox(tmpVec,vec(16,16),false));
    default:
      LOG("straight\n");
      //straight
      vid.bg0.image(vec(0,0),Snow,0);
      hitBoxes.push_back(hitBox(vec(0,0),vec(14,128),false));
      hitBoxes.push_back(hitBox(vec(114,0),vec(14,128),false));
      break;
    }
    System::paint();
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

  unsigned neighborsID(int side) const{
    if(side <= NO_SIDE||side >= NUM_SIDES) return MAX_UINT;
    return neighbors[side];
  }

  int neighborsSide(unsigned id) const{
    for(int i = 0; i<4; i++){
      if(neighbors[i] == id){
	return i;
      }
    }
    return NO_SIDE;
  }
  
  unsigned id() const{
    return _id;
  }
  
  VideoBuffer& vbuf(){//can't be const because it may result in change even if it doesn't itself
    return vid;
  }

  Array<hitBox,8>& physicsBoxes(){
    return hitBoxes;
  }
  
  bool isValid() const{
    return isSetup&&_id!=MAX_UINT;
  }
  
private:
  unsigned _id;
  VideoBuffer vid;
  bool isSetup;
  unsigned neighbors[4];
  Random rand;
  Array<hitBox,8> hitBoxes;
};

#endif /* _CUBE_H_ */
