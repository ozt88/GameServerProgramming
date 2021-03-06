#include "pch.h"
#include "GameTimer.h"


GameTimer::GameTimer()
	:m_SecondsPerCount(0.0f), m_DeltaTime(-1.0f), m_BaseTime(0), 
	m_PausedTime(0), m_PrevTime(0), m_CurrTime(0), m_Stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*) &countsPerSec);
	m_SecondsPerCount = 1.0f / (double) countsPerSec;
}


GameTimer::~GameTimer()
{
}

float GameTimer::TotalTime() const
{
	double totalTime = 0.f;
	__int64 totalTickCount = 0;

	if(m_Stopped)
	{
		totalTickCount = ( m_StopTime - m_PausedTime ) - m_BaseTime;
	}
	else
	{
		totalTickCount = ( m_CurrTime - m_PausedTime ) - m_BaseTime;
	}

	totalTime = totalTickCount * m_SecondsPerCount;
	return (float)totalTime;
}

float GameTimer::DeltaTime() const
{
	return (float)m_DeltaTime;
}

void GameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*) &currTime);
	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_Stopped = false;
}

void GameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*) &startTime);
	if(m_Stopped)
	{
		m_PausedTime += startTime - m_StopTime;
		m_PrevTime = startTime;
		m_StopTime = 0;
		m_Stopped = false;
	}
}

void GameTimer::Stop()
{
	if(!m_Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*) &currTime);
		m_StopTime = currTime;
		m_Stopped = true;
	}
}

void GameTimer::Tick()
{
	if(m_Stopped)
	{
		m_DeltaTime = 0.0f;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*) &currTime);
	m_CurrTime = currTime;

	m_DeltaTime = ( m_CurrTime - m_PrevTime ) * m_SecondsPerCount;
	m_PrevTime = m_CurrTime;

	if(m_DeltaTime < 0.f)
	{
		m_DeltaTime = 0.f;
	}
}
