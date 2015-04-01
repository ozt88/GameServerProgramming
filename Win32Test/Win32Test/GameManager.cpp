#include "pch.h"
#include "GameManager.h"

GameManager* GameManager::m_Instance = nullptr;

GameManager::GameManager()
{
	QueryPerformanceCounter(&m_StartTime);
}

GameManager::~GameManager()
{
}

GameManager* GameManager::GetInstance()
{
	if(m_Instance == nullptr)
	{
		m_Instance = new GameManager();
	}
	return m_Instance;
}

void GameManager::ReleaseInstance()
{
	if(m_Instance != nullptr)
	{
		delete m_Instance;
	}
	m_Instance = nullptr;
}

void GameManager::Update()
{
	float deltaT = GetElapseSecond();
	KeyInput();
	m_Polygon.Update(deltaT);
	m_Polygon.CheckIn(m_MousePos);
}

void GameManager::Render()
{
	HDC hdc = GetDC(m_Hwnd);
	//m_Rect1.Draw(m_MemoryDC);
	//m_MovingRect.Draw(m_MemoryDC);

	m_Polygon.Draw(m_MemoryDC);
	BitBlt(hdc, 0, 0, 960, 640, m_MemoryDC, 0, 0, SRCCOPY);
	Rectangle(m_MemoryDC, 0, 0, 960, 640);
	ReleaseDC(m_Hwnd, hdc);
}

void GameManager::Init(HWND hwnd)
{
	m_Hwnd = hwnd;
	HDC hdc = GetDC(m_Hwnd);
	m_MemoryDC = CreateCompatibleDC(hdc);
	m_MemoryBitmap = CreateCompatibleBitmap(hdc, 960, 640);
	SelectObject(m_MemoryDC, m_MemoryBitmap);
	ReleaseDC(m_Hwnd, hdc);

	m_Rect1.SetSize(100, 100);
	m_Rect1.SetPosition(200, 200);
	m_MovingRect.SetSize(100, 100);


	m_Polygon.AddPoint(MyPoint(-100, 100));
	m_Polygon.AddPoint(MyPoint(20, 150));
	m_Polygon.AddPoint(MyPoint(30, 80));
	m_Polygon.AddPoint(MyPoint(10, -80));
	m_Polygon.AddPoint(MyPoint(-80, -60));
	m_Polygon.SetPosition(100, 100);
}



void GameManager::SetPoint(float posX, float posY)
{
	m_MousePos._x = posX;
	m_MousePos._y = posY;
}

float GameManager::GetElapseSecond()
{
	static LARGE_INTEGER s_lastTime = m_StartTime;
	LARGE_INTEGER currentTime;
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceCounter(&currentTime);
	QueryPerformanceFrequency(&ticksPerSecond);
		
	float seconds = ( (float) currentTime.QuadPart 
					 - (float)s_lastTime.QuadPart )
					 /(float) ticksPerSecond.QuadPart;

	s_lastTime = currentTime;
	return seconds;
}

void GameManager::KeyInput()
{
	// ------------------ Init -------------------
	// 매 프레임 한번만 처리하면 됨.
	// 입력감지
	static	BYTE	byOldKey[MAX_KEY_NUM] = {0};

	if(GetKeyboardState(m_KeyState))
	{
		// DOWN 및 UP계산
		for(int i = 0; i < MAX_KEY_NUM; i++)
		{
			if(m_KeyState[i] & HOLDKEY)
			{
				if(!byOldKey[i])
				{
					byOldKey[i] = 1;
					m_KeyState[i] |= PUSHKEY;
				}
			}
			else
			{
				if(byOldKey[i])
				{
					byOldKey[i] = 0;
					m_KeyState[i] = PULLKEY;
				}
			}
		}
	}
	// ------------------ Init 끝 -------------------

	if(m_KeyState['C'] & PUSHKEY)
	{
		// C가 눌렸다면
	}
	if(m_KeyState['Z'] & PUSHKEY)
	{
		// Z 가 눌렸다면
	}
	if(m_KeyState['Z'] & PULLKEY)
	{
		// Z를 눌렀다가 띄었을때
	}
	if(m_KeyState[VK_UP] & HOLDKEY)
	{
		// 위쪽 화살표 누르는 중
	}
	else if(m_KeyState[VK_DOWN] & HOLDKEY)
	{
		// 아래쪽 화살표 누르는 중
	}
	if(m_KeyState[VK_LEFT] & HOLDKEY)
	{
		// 왼쪽 화살표 누르는 중
	}
	else if(m_KeyState[VK_RIGHT] & HOLDKEY)
	{
		// 오른쪽 화살표 누르는 중
		int a = 0;
	}
}

BYTE GameManager::GetKeyState(int keyCode)
{
	if(keyCode < 0 || keyCode >= MAX_KEY_NUM)
		return -1;
	else
		return m_KeyState[keyCode];
}

