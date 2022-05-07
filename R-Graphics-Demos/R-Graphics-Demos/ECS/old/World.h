#pragma once
#include <cstdint>
#include <cassert>
#include "WorldConsts.h"
#include "Archetype.h"
namespace R
{
	namespace ECS
	{
		class World
		{
		public:
			World();
		private:
			std::unordered_map<uint64_t, uintptr_t> archetypeHashes;
		public:
			template<typename... T>
			Archetype<T...>* RegisterArchetype()
			{
				uint64_t signature = Signature<T...>();
				assert(archetypeHashes.find(signature) == archetypeHashes.end());
				Archetype<T...>* archetype = new Archetype<T...>();
				archetypeHashes[signature] = reinterpret_cast<uintptr_t>(archetype);
			}

			template<typename... T>
			uint32_t CreateEntity(const T&... t)
			{
				uint64_t signature = Signature<T...>();
				auto archeTypePtr = archetypeHashes.find(signature);
				assert(archeTypePtr != archetypeHashes.end());
				return  { (reinterpret_cast<Archetype<T...>*>(archeTypePtr->second))->AddEntity(t...) };
			}

			template<typename... T>
			void MarkEntityDead(uint32_t)
			{
				uint64_t signature = Signature<T...>();
				auto archeTypePtr = archetypeHashes.find(signature);
				assert(archeTypePtr != archetypeHashes.end());
				(reinterpret_cast<Archetype<T...>*>(archeTypePtr->second))->MarkEntityDead();
			}



		private:
		public:

			template<typename... T>
			uint64_t Signature()
			{
				uint64_t signature;
				auto l = [&]<typename U>{ signature |= U::uid; };
				(l<T>(), ...);
			}

			template<typename UpdateFunc>
			void ProcessArchetypesWithSignature(std::uint64_t signature, const UpdateFunc& update)
			{

			}
		};
	}
}

