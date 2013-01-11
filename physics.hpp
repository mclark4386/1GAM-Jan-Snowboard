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

template<class T>
bool collisionDetect(const Vector2<T> originA, const Vector2<T> sizeA, const Vector2<T> originB, const Vector2<T> sizeB){
  T x1A = originA.x;
  T x2A = originA.x+sizeA.x;
  T y1A = originA.y;
  T y2A = originA.y+sizeA.y;

  T x1B = originB.x;
  T x2B = originB.x+sizeB.x;
  T y1B = originB.y;
  T y2B = originB.y+sizeB.y;

  if(y2A < y1B) return false;
  if(y1A > y2B) return false;

  if(x2A < x1B) return false;
  if(x1A > x2B) return false;

  return true;
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
