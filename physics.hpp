/*
 * Snowboard! by Matthew Clark(mclark4386 @github)
 * physics.hpp
 * Basic 2.5D physics for snowboarding and collision detection
 */

#include <sifteo.h>

using namespace Sifteo;


static const float kGravity = 9.806f;
static const float kDampen = 0.80f;

template<class T, class F> 
Vector2<T> calcNewVelocity(Vector2<T> oldVelocity, Vector3<F> accel, float dt){
  auto Accel = vec(static_cast<T>(accel.x),static_cast<T>(accel.y),static_cast<T>(accel.z));
    Accel = Accel.normalize() * kGravity;
    return ((Accel*dt).xy() + oldVelocity) * kDampen;
};

bool collisionDetect(Int2 originA, Int2 sizeA,Int2 originB, Int2 sizeB);