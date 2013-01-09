/*
 * Sifteo SDK Example.
 */

#include "game.hpp"

using namespace Sifteo;

static AssetSlot MainSlot = AssetSlot::allocate()
  .bootstrap(GameAssets).bootstrap(Fonts);

static Metadata M = Metadata()
    .title("Snowboard!")
    .package("com.Matthew.Clark.Snowboard", "0.1")
    .icon(Icon)
    .cubeRange(0, CUBE_ALLOCATION);


void main()
{
    static Game game;

    game.install();
    game.run();
}
