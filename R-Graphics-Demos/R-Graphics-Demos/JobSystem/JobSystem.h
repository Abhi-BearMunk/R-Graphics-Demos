#pragma once
#include "pch.h"
namespace R
{
	namespace Job
	{
		typedef void Entrypoint(void* param, std::uint32_t tid);
		class JobSystem
		{
		public:
			JobSystem();
			~JobSystem();

			enum class JobPriority : std::uint32_t
			{
				P_HIGH,
				P_MEDIUM,
				P_LOW,
			};

			union JobPriorityOrAffinity
			{
				JobPriority priority;
				std::uint32_t affinity;
			};

			struct JobCounter
			{
				std::atomic<int> counter = 0;
				std::mutex cMutex;
				std::condition_variable cv;
			};

			struct JobDesc
			{
				Entrypoint* jobFunc = nullptr;
				void* param = nullptr;
				JobCounter* pCounter = nullptr;
				JobPriorityOrAffinity priorityOrAffinity = { JobPriority::P_MEDIUM };
			};

			/// <summary>
			/// NOTE : This assumes all the jobs share the same counter.
			/// Kick a bunch of jobs
			/// </summary>
			/// <param name="aDesc"> an array of jobs </param>
			/// <param name="nJobs"> num jobs </param>
			void KickJobsWithPriority(const JobDesc* aDesc, std::uint32_t nJobs);

			/// <summary>
			/// Kick jobs which are locked to threads
			/// For max efficiency kick atleast one job for each thread
			/// </summary>
			/// <param name="aDesc"> an array of jobs </param>
			/// <param name="nJobs"> num jobs </param>
			void KickJobsWithAffinity(const JobDesc* aDesc, std::uint32_t nJobs);

			/// <summary>
			/// Wait for a given counter to become '0'
			/// </summary>
			/// <param name="pCounter"></param>
			void WaitForCounter(JobCounter* pCounter);

			/// <summary>
			/// NOTE : This assumes all the jobs share the same counter.
			/// Should only be called from main thread.
			/// Our job system don't yet fully support waiting on other jobs from within!
			/// Using this inside a job may lead to a DEADLOCK!
			/// </summary>
			/// <param name="aDesc"> an array of jobs </param>
			/// <param name="nJobs"> num jobs </param>
			void KickJobsWithPriorityAndWait(const JobDesc* aDesc, std::uint32_t nJobs);

			/// <summary>
			/// Check desc of KickJobsWithAffinity(...) and KickJobsWithPriorityAndWait(...)
			/// </summary>
			/// <param name="aDesc"> an array of jobs </param>
			/// <param name="nJobs"></param>
			/// <param name="nJobs"> num jobs </param>
			void KickJobsWithAffinityAndWait(const JobDesc* aDesc, std::uint32_t nJobs);

			inline std::uint32_t GetNumWorkers() { return static_cast<std::uint32_t>(m_threadPool.size()); }
		private:
			std::queue<JobDesc>					m_highPriorityJobs;
			std::queue<JobDesc>					m_medPriorityJobs;
			std::queue<JobDesc>					m_lowPriorityJobs;
			std::vector<std::queue<JobDesc>>    m_JobsWithThreadAffinity;
			std::mutex							m_qInUse;
			std::condition_variable				m_wakeSleepingThread;
			std::vector<std::thread>			m_threadPool; // TODO : Change to windows threads to be able to set affinity
#ifdef _DEBUG
			std::vector<std::uint32_t>			m_threadJobs;
#endif // _DEBUG
			std::atomic<bool>					m_stopRunning = false;
			void ProcessJobs(std::uint32_t tid);
			bool SharedJobsAvailable();
			bool LockedJobsAvailable(std::uint32_t tid);
		};
	}
}

