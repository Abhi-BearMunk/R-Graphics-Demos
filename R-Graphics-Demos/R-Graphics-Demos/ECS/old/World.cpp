#include "World.h"

R::ECS::World::World()
	:root{ 0, 0, {}, {}}
{
	archetypeHashes.insert(0);
}
