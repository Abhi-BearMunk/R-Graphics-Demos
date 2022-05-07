#pragma once
#include "pch.h"
namespace R
{
	namespace ECS
	{
		constexpr uint32_t MAX_ENTITIES_PER_COMPONENT_PER_ARCHETYPE = 1 << 20; // A little over a million
		constexpr uint32_t MAX_COMPONENTS = 64;

		struct Entity
		{
		public:
			uint64_t signature;
			uint32_t index;
		};

		template<size_t numComps>
		struct Interest
		{
			void* ppComps[numComps];
			uint32_t entityCount;
		};

		/// <summary>
		/// Wrapper for all things ECS
		/// Stores Arrays for each component type (on the heap) by UID and signature
		/// example:
		/// struct1 = { ... data, uid = 0b1 }, struct2 = { ... data, uid = 0b10 }
		/// Archetype1 = [struct1] (signature 0b1) , Archetype2 = [struct1, struct2] (signature 0b11)
		/// Now add 2 entities of Archetype1 and 3 of Archetype2
		/// Generated table looks like this
		/// ------------------------------------------------------------
		/// ComponentID			  SOA					 SOA        .....
		/// ------------------------------------------------------------
		/// [0b1]		| { 0b1, struct1[2] }	| { 0b11, struct1[3]	|
		/// [0b10]		| { 0b11, struct2[3] }	|						|
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
			template<typename... T>
			void RegisterArchetype()
			{
				uint64_t signature = GenerateSignature<T...>();
				// Register Archetype should be called only once per component combination
				assert(m_signatureCounts.find(signature) == m_signatureCounts.end());
				m_signatureCounts[signature] = 0;
				(InitializeComponentArrayForSignature<T>(signature), ...);
			}

			template<typename... T>
			Entity CreateEntity(const T&... t)
			{
				uint64_t signature = GenerateSignature<T...>();
				auto it = m_signatureCounts.find(signature);
				// Archetype should already be registered
				assert(it != m_signatureCounts.end());
				uint32_t index = it->second;
				// Add each component to the appropriate array
				(SetComponentAtIndex<T>(signature, index, t), ...);
				// Increment count
				it->second++;
				return { signature, index };
			}

			// TODO !!!
			void KillEntity(Entity& e)
			{

			}

			template<typename T>
			T& GetComponent(const Entity& e)
			{
				assert((e.signature & T::uid) != 0);
				return reinterpret_cast<T*>(m_componentArrays[T::uid][e.signature].ptr)[e.index];
			}

			template<typename... T>
			void InterestedIn(std::vector<Interest<sizeof...(T)>>& interests)
			{
				assert(interests.size() == 0);
				uint64_t signature = GenerateSignature<T...>();
				uint32_t numComponents = sizeof...(T);
				// TODO: Could be improved with a tree
				for (auto& it : m_signatureCounts)
				{
					if ((it.first & signature) == signature)
					{
						interests.push_back(Interest<sizeof...(T)>());
						interests.back().entityCount = it.second;
						int i = 0;
						(InterestedInHelper<T>(interests.back().ppComps, i, it.first), ...);
					}
				}
			}

		private:
			template<typename T>
			void InterestedInHelper(void** ppComps, int& i, uint64_t signature)
			{
				ppComps[i] = m_componentArrays[T::uid][signature].ptr;
				i++;
			}

			template<typename... T>
			uint64_t GenerateSignature()
			{
				uint64_t signature = 0;
				(GenerateSignatureHelper<T>(signature), ...);
				return signature;
			}

			template<typename T>
			void GenerateSignatureHelper(uint64_t& signature)
			{
				signature |= T::uid;
			}

			template<typename T>
			void InitializeComponentArrayForSignature(uint64_t signature)
			{
				m_componentArrays[T::uid][signature] = { sizeof(T) , malloc(sizeof(T) * MAX_ENTITIES_PER_COMPONENT_PER_ARCHETYPE) };
			}

			template<typename T>
			void SetComponentAtIndex(uint64_t signature, uint32_t index, const T& value = {})
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

			std::unordered_map<uint64_t, uint32_t>		m_signatureCounts;
			std::unordered_map<uint64_t, SOAGeneric>	m_componentArrays[MAX_COMPONENTS + 1]; // just to make things easy, there is no component with ID 0
		};
	}
}