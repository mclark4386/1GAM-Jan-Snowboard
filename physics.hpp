/*
 * Snowboard! by Matthew Clark(mclark4386 @github)
 * physics.hpp
 * Basic 2.5D physics for snowboarding and collision detection
 */

#include <sifteo.h>

using namespace Sifteo;


static const float kGravity = 9.806f;
static const float kDampen = 0.95f;

template<class T, class F> 
Vector2<T> calcNewVelocity(const Vector2<T> oldVelocity, const Vector3<F> accel, const float dt){
  if(accel.x <= 1&&accel.x >= -1&&accel.y <= 1&&accel.y >=-1){//hopefully reduce jittering
    return oldVelocity*kDampen;
  }

  auto Accel = vec(static_cast<T>(accel.x),static_cast<T>(accel.y),static_cast<T>(accel.z));
    Accel = Accel.normalize() * kGravity;
    return ((Accel*dt).xy() + oldVelocity) * kDampen;
};

bool collisionDetect(Int2 originA, Int2 sizeA,Int2 originB, Int2 sizeB){
  return false;
};

template<class T, class F>
int sideOfWorldCollision(const Vector2<T> pos, const Vector2<F> size){
  if(pos.x+(size.x/2) < 0){
    return LEFT;
  }
  if(pos.y+(size.y/2) < 0){
    return TOP;
  }
  if(pos.x+(size.x/2) > 128){
    return RIGHT;
  }
  if(pos.y+(size.y/2) > 128){
    return BOTTOM;
  }
  return NO_SIDE;
}
