#pragma once
#include "pch.h"
namespace R
{
	namespace ECS
	{
		constexpr std::uint32_t MAX_ENTITIES_PER_ARCHETYPE = 1 << 24; // A little over 15 million
		constexpr std::uint32_t MAX_COMPONENTS = 64;

		struct Entity
		{
		public:
			std::uint64_t signature;
			std::uint32_t index;
		};

		template<size_t numComps>
		struct Interest
		{
			void* ppComps[numComps];
			std::uint32_t entityCount;
		};

		/// <summary>
		/// Wrapper for all things ECS
		/// Stores Arrays for each component type (on the heap) by UID and signature
		/// example:
		/// component1 = { ... data, uid = 0b1 }, component2 = { ... data, uid = 0b10 }
		/// Archetype1 = [component1] (signature 0b1) , Archetype2 = [component1, component2] (signature 0b11)
		/// Now add 2 entities of Archetype1 and 3 of Archetype2
		/// Generated table looks like this
		/// ------------------------------------------------------------
		/// ComponentID			  SOA					 SOA        .....
		/// ------------------------------------------------------------
		/// [0b1]		| { 0b1, component1[2] }	| { 0b11, component1[3]	|
		/// [0b10]		| { 0b11, component2[3] }	|						|
		///		...
		/// </summary>
		class World
		{
		private:
			struct SOAGeneric
			{
			public:
				size_t size;
				void* ptr;
			};

		public:
			~World()
			{
				Cleanup();
			}

			template<typename... Components>
			void RegisterArchetype()
			{
				std::uint64_t signature = GenerateSignature<Components...>();
				// Register Archetype should be called only once per component combination
				assert(m_signatureCounts.find(signature) == m_signatureCounts.end());
				m_signatureCounts[signature] = 0;
				(InitializeComponentArrayForSignature<Components>(signature), ...);
			}

			template<typename... Components>
			Entity CreateEntity(const Components&... t)
			{
				std::uint64_t signature = GenerateSignature<Components...>();
				auto it = m_signatureCounts.find(signature);
				// Archetype should already be registered
				assert(it != m_signatureCounts.end());
				std::uint32_t index = it->second;
				// Add each component to the appropriate array
				(SetComponentAtIndex<Components>(signature, index, t), ...);
				// Increment count
				it->second++;
				return { signature, index };
			}

			// TODO !!!
			void KillEntity(Entity& e)
			{

			}

			template<typename Component>
			Component& GetComponent(const Entity& e)
			{
				assert((e.signature & Component::uid) != 0);
				return reinterpret_cast<Component*>(m_componentArrays[Component::uid][e.signature].ptr)[e.index];
			}

			template<typename... Components>
			void InterestedIn(std::vector<Interest<sizeof...(Components)>>& interests)
			{
				assert(interests.size() == 0);
				std::uint64_t signature = GenerateSignature<Components...>();
				std::uint32_t numComponents = sizeof...(Components);
				// TODO: Could be improved with a tree
				for (auto& it : m_signatureCounts)
				{
					if ((it.first & signature) == signature)
					{
						interests.push_back(Interest<sizeof...(Components)>());
						interests.back().entityCount = it.second;
						int i = 0;
						(InterestedInHelper<Components>(interests.back().ppComps, i, it.first), ...);
					}
				}
			}

		private:
			template<typename T>
			void InterestedInHelper(void** ppComps, int& i, std::uint64_t signature)
			{
				ppComps[i] = m_componentArrays[T::uid][signature].ptr;
				i++;
			}

			template<typename... T>
			std::uint64_t GenerateSignature()
			{
				std::uint64_t signature = 0;
				(GenerateSignatureHelper<T>(signature), ...);
				return signature;
			}

			template<typename T>
			void GenerateSignatureHelper(std::uint64_t& signature)
			{
				signature |= T::uid;
			}

			template<typename T>
			void InitializeComponentArrayForSignature(std::uint64_t signature)
			{
				m_componentArrays[T::uid][signature] = { sizeof(T) , malloc(sizeof(T) * MAX_ENTITIES_PER_ARCHETYPE) };
			}

			template<typename T>
			void SetComponentAtIndex(std::uint64_t signature, std::uint32_t index, const T& value = {})
			{
				reinterpret_cast<T*>(m_componentArrays[T::uid][signature].ptr)[index] = value;
			}

			void Cleanup()
			{
				for (int i = 1; i <= MAX_COMPONENTS; i++)
				{
					for (auto it : m_componentArrays[i])
					{
						free(it.second.ptr);
					}
				}
			}

			std::unordered_map<std::uint64_t, std::uint32_t>		m_signatureCounts;
			std::unordered_map<std::uint64_t, SOAGeneric>	m_componentArrays[MAX_COMPONENTS + 1]; // just to make things easy, there is no component with ID 0
		};
	}
}