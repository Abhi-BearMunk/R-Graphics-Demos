#pragma once
#include <cstdint>
#include <unordered_map>
#include "WorldConsts.h"

namespace R
{
	namespace ECS
	{
		class ArchetypeBase
		{
		protected:
			ArchetypeBase() {};
			virtual ~ArchetypeBase() {};
		};

		/// <summary>
		/// A recipy for different component combinations
		/// NOT FREE THREADED!
		/// </summary>
		/// <typeparam name="...T"> Components with uid </typeparam>
		template <typename... T>
		class Archetype : ArchetypeBase
		{
		public:
			Archetype();
			~Archetype();
			inline uint32_t GetCount() const { return count; }
			template <class U> 
			inline U& GetComponent(uint32_t index) { return reinterpret_cast<U*>(componentsArray[U::uid])[index]; }
			uint32_t AddEntity(const T&... t);
			// TODO !!!
			void MarkEntityDead(uint32_t index);
			// TODO !!!
			void FlushDeadEntities();
		private:
			// TODO : This is ugly, can we get the compiler to figure this out?
			std::unordered_map<uint64_t, uintptr_t> componentsArray;	
			uint64_t signature;
			uint32_t count;

			void Swap(uint32_t i, uint32_t j);
			template<typename U>
			void InitializeComponentArray();
			template<typename U>
			void InitializeEntity(const U& u);
			void RemoveEntity(uint32_t index);
		};

		template<typename ...T>
		Archetype<T...>::Archetype()
			:componentsArray(sizeof...(T)), count(0), signature(0)
		{
			(InitializeComponentArray<T>(), ...);
		}

		template<typename ...T>
		inline Archetype<T...>::~Archetype()
		{
			for(auto )
		}

		template<typename ...T>
		uint32_t Archetype<T...>::AddEntity(const T & ...t)
		{
			(InitializeEntity<T>(t), ...);
			return count++;
		}

		// TODO !!!
		template<typename ...T>
		inline void Archetype<T...>::MarkEntityDead(uint32_t index)
		{
		}

		// TODO !!!
		template<typename ...T>
		inline void Archetype<T...>::FlushDeadEntities()
		{
		}

		template<typename ...T>
		void Archetype<T...>::RemoveEntity(uint32_t index)
		{
			Swap(index, count - 1);
			count--;
		}

		template<typename ...T>
		inline void Archetype<T...>::Swap(uint32_t i, uint32_t j)
		{
			for (uintptr_t compArr : componentsArray)
			{
				auto temp = compArr[i];
				compArr[i] = compArr[j];
				compArr[j] = temp;
			}
		}

		template<typename ...T>
		template<typename U>
		inline void Archetype<T...>::InitializeComponentArray()
		{
			signature |= U::uid;
			componentsArray[U::uid] = reinterpret_cast<uintptr_t>(new U[MAX_ENTITIES]);
		}

		template<typename ...T>
		template<typename U>
		inline void Archetype<T...>::InitializeEntity(const U& u)
		{
			reinterpret_cast<U*>(componentsArray[U::uid])[count] = u;
		}
	}
}

































//template <class... Args> class Archetype 
//{ 
//public:
//	uint32_t Create() { return count++; };
//	//void GetComponent()
//	//{
//	//	return 0;
//	//}
//	/*template <class U>
//	U& GetComponent()
//	{
//		assert(false);
//		return U();
//	}*/
//private:
//	uint32_t count = 0; 
//};

//template <class T, class... Args>
//class Archetype<T, Args...> : public Archetype<Args...>
//{
//public:
//	Archetype() 
//		: Archetype<Args...>(), aos(new T[MAX_ENTITIES]){}
//	uint32_t Create(const T& t, const Args&... args)
//	{
//		Archetype<Args...>& base = *this;
//		int count = base.Create(args...);
//		aos[count] = t;
//		return count;
//	};
//	template <class U, class = typename std::enable_if<std::is_same<T, U>::value, int>::type>
//	U& GetComponent(uint32_t index) 
//	{ 
//		return aos[index]; 
//	}
//	template <class U, class = typename std::enable_if<!std::is_same<T, U>::value, int>::type>
//	U& GetComponent(uint32_t index)
//	{
//		Archetype<Args...>& base = *this;
//		return base.GetComponent<U>(index);
//	}
//private:
//	T* aos;
//};