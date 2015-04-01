#pragma once
#include "MyRect.h"
#include "MyPolygon.h"

#define FREEKEY	0x10	// ������ ���� ����.
#define PULLKEY	0x20	// ������ �������
#define PUSHKEY	0x40	// ������ ����
#define HOLDKEY	0x80	// ������ �ִ� ��
#define MAX_KEY_NUM 256
class GameManager
{
public:
	static GameManager*		GetInstance();
	void					ReleaseInstance();
	
	void					Init(HWND hwnd);
	void					Update();
	void					Render();

	void					SetPoint(float posX, float posY);
	BYTE					GetKeyState(int keyCode);

private:
	GameManager();
	~GameManager();
	float					GetElapseSecond();
	void					KeyInput();

private:
	static GameManager*		m_Instance;
	HWND					m_Hwnd;
	HDC						m_MemoryDC;
	HBITMAP					m_MemoryBitmap;
	MyPoint					m_MousePos;
	LARGE_INTEGER			m_StartTime;

	MyRect					m_Rect1;
	MyRect					m_MovingRect;
	MyPolygon				m_Polygon;

	BYTE					m_KeyState[MAX_KEY_NUM];
};
