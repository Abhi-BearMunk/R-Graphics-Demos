#pragma once
#include <cstdint>
namespace R
{
	namespace ECS
	{
		constexpr uint32_t MAX_ENTITIES = 2048;

		/// <summary>
		/// Basic structure of array for PODs
		/// </summary>
		/// <typeparam name="T"> Component type </typeparam>
		template <typename T>
		class SOA
		{
		public:
			void Allocate()
			{
				heapArr = new T[MAX_ENTITIES];
			}
			void DeAllocate() 
			{ 
				delete[] heapArr; 
			}
			void SetDataAtIndex(const T& t, uint32_t index)
			{
				heapArr[index] = t;
			}
			T& GetDataAtIndex(uint32_t index)
			{
				return heapArr[index];
			}
		private:
			T* heapArr;
		};

		/// <summary>
		/// Base case for SOA collection recursion
		/// NOTE: Its a convinience class, avoid putting actual data
		/// </summary>
		/// <typeparam name="T"> Type of Component </typeparam>
		template <typename... T>
		class SOACollection {};

		/// <summary>
		/// Recursion for SOA collection
		/// NOTE: Its a convinience class, avoid putting actual data
		/// </summary>
		/// <typeparam name="T"> Type of Component </typeparam>
		/// <typeparam name="...Args"> Type of following Components </typeparam>
		template <typename T, typename... Args>
		class SOACollection<T, Args...> : public SOA<T>, public SOACollection<Args...> {};		

		/// <summary>
		/// A non templated base class for Archetypes
		/// providing functionality for individual components
		/// </summary>
		class ArchetypeBase
		{
		public:
			
		};

		/// <summary>
		/// Archetype represents cache friendly collection of entities with a unique "permutation" of components 
		/// </summary>
		/// <typeparam name="...T"> Type of Components </typeparam>
		template <typename... T>
		class Archetype : public SOACollection<T...>
		{
		public:
			Archetype()
			{
				([&]<T>() { m_signature |= T::uid; }, ...);
				(static_cast<SOA<T>>(*this).Allocate(), ...);
			}

			~Archetype()
			{
				(static_cast<SOA<T>>(*this).DeAllocate(), ...);
			}

			uint32_t GetCount()
			{
				return m_count;
			}

			uint64_t GetSignature()
			{
				return m_signature;
			}

			void AddEntity(const T&... t)
			{
				// We do this instead of making AddEntity virtual to save vtable lookup, 
				// maybe do this for destructors too ? TODO!! 
				(static_cast<SOA<T>>(*this).SetDataAtIndex(t, m_count), ...);
				m_count++;
			}

			template<typename T>
			T& GetComponent(uint32_t index)
			{
				return reinterpret_cast<SOA<T>>(*this).GetDataAtIndex(index);
			}

			// TODO !!
			void MarkForKill(const uint32_t& index)
			{

			}

			// TODO !!
			void CleanupKilledEntities(const uint32_t& index)
			{

			}

			void Reset()
			{
				m_count = 0;
			}
		private:
			uint32_t m_count = 0;
			uint64_t m_signature = 0;
		};
	}
}