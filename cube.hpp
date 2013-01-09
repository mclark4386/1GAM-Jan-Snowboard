#ifndef _CUBE_H_
#define _CUBE_H_


#include <sifteo.h>
#include "assets.gen.h"

using namespace Sifteo;

class Cube{
  static const unsigned MAX_UINT = -1;

public:
  Cube():_id(MAX_UINT),isSetup(false){};
  ~Cube(){};

  void setup(unsigned id){
    CubeID cube(id);
    _id = id;
    LOG("Cube %d connected\n", id);
    
    vid.initMode(BG0_SPR_BG1);
    vid.attach(id);
    
    //try to set up the bg
    vid.bg0.image(vec(0,0),Snow);
    isSetup = true;
  }

  void draw(){
    if(isSetup&&_id != MAX_UINT){
      vid.bg0.image(vec(0,0),Snow);//may not be needed
    }
  }

  unsigned id(){
    return _id;
  }

private:
  unsigned _id;
  VideoBuffer vid;
  bool isSetup;

};

#endif /* _CUBE_H_ */
