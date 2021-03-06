#include "pch.h"
#include "JobSystem.h"
#include "Utils/Logger.h"
R::Job::JobSystem::JobSystem()
{
	m_JobsWithThreadAffinity.resize(std::thread::hardware_concurrency());
	for (std::uint32_t i = 0; i < std::thread::hardware_concurrency(); i++)
	{
		m_threadPool.emplace_back(std::thread(&JobSystem::ProcessJobs, this, i));
#ifdef _DEBUG
		m_threadJobs.push_back(0);
#endif // _DEBUG
	}
}

R::Job::JobSystem::~JobSystem()
{
	m_stopRunning = true;
	m_wakeSleepingThread.notify_all();
	for (auto& thread : m_threadPool)
	{
		thread.join();
	}
#ifdef _DEBUG
	for (std::uint32_t i = 0; i < m_threadJobs.size(); i++)
	{
		R_LOG_DEBUG("Num jobs by thread {} = {}", i, m_threadJobs[i]);
	}
#endif // _DEBUG
}

void R::Job::JobSystem::KickJobsWithPriority(const JobDesc* aDesc, std::uint32_t nJobs)
{
	{
		std::lock_guard<std::mutex> lock(m_qInUse);
		for (std::uint32_t i = 0; i < nJobs; i++)
		{
			switch (aDesc[i].priorityOrAffinity.priority)
			{
			case JobPriority::P_HIGH:
				m_highPriorityJobs.push(aDesc[i]);
				break;
			case JobPriority::P_MEDIUM:
				m_medPriorityJobs.push(aDesc[i]);
				break;
			case JobPriority::P_LOW:
				m_lowPriorityJobs.push(aDesc[i]);
				break;
			}
		}
	}
	m_wakeSleepingThread.notify_one();
}

void R::Job::JobSystem::KickJobsWithAffinity(const JobDesc* aDesc, std::uint32_t nJobs)
{
	{
		std::lock_guard<std::mutex> lock(m_qInUse);
		for (std::uint32_t i = 0; i < nJobs; i++)
		{
			m_JobsWithThreadAffinity[aDesc[i].priorityOrAffinity.affinity].push(aDesc[i]);
		}
	}
	m_wakeSleepingThread.notify_all();
}

void R::Job::JobSystem::WaitForCounter(JobCounter* pCounter)
{
	std::unique_lock lk(pCounter->cMutex);
	if (pCounter->counter > 0)
	{
		pCounter->cv.wait(lk, [&] { return pCounter->counter <= 0; });
	}
}

void R::Job::JobSystem::KickJobsWithPriorityAndWait(const JobDesc* aDesc, std::uint32_t nJobs)
{
	KickJobsWithPriority(aDesc, nJobs);
	assert(nJobs != 0);
	WaitForCounter(aDesc[0].pCounter);
}

void R::Job::JobSystem::KickJobsWithAffinityAndWait(const JobDesc* aDesc, std::uint32_t nJobs)
{
	KickJobsWithAffinity(aDesc, nJobs);
	assert(nJobs != 0);
	WaitForCounter(aDesc[0].pCounter);
}

void R::Job::JobSystem::ProcessJobs(std::uint32_t tid)
{
	JobDesc job;
	while (!m_stopRunning)
	{
		{
			std::unique_lock lock(m_qInUse);
			m_wakeSleepingThread.wait(lock, [&] { return LockedJobsAvailable(tid) || SharedJobsAvailable() || m_stopRunning; });
			if (m_stopRunning)
				return;
			if (LockedJobsAvailable(tid))
			{
				job = m_JobsWithThreadAffinity[tid].front();
				m_highPriorityJobs.pop();
			}
			else if (!m_highPriorityJobs.empty())
			{
				job = m_highPriorityJobs.front();
				m_highPriorityJobs.pop();
			}
			else if (!m_medPriorityJobs.empty())
			{
				job = m_medPriorityJobs.front();
				m_medPriorityJobs.pop();
			}
			else if (!m_lowPriorityJobs.empty())
			{
				job = m_lowPriorityJobs.front();
				m_lowPriorityJobs.pop();
			}
		}
		if (SharedJobsAvailable())
		{
			m_wakeSleepingThread.notify_one();
		}

		assert(job.jobFunc != nullptr);
		job.jobFunc(job.param, tid);
#ifdef _DEBUG
		m_threadJobs[tid]++;
#endif // _DEBUG
		if (job.pCounter)
		{
			{
				std::lock_guard<std::mutex> lk(job.pCounter->cMutex);
				job.pCounter->counter--;
			}
			if (job.pCounter->counter == 0)
			{
				job.pCounter->cv.notify_all();
			}
		}
	}
}

bool R::Job::JobSystem::SharedJobsAvailable()
{
	return !m_highPriorityJobs.empty() || !m_medPriorityJobs.empty() || !m_lowPriorityJobs.empty();
}

bool R::Job::JobSystem::LockedJobsAvailable(std::uint32_t tid)
{
	return !m_JobsWithThreadAffinity[tid].empty();
}
