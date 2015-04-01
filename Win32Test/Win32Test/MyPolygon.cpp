#include "pch.h"
#include "MyPolygon.h"
#include "GameManager.h"

MyPolygon::MyPolygon()
{
	SetColor(255, 255, 0);
}


MyPolygon::~MyPolygon()
{
}

void MyPolygon::AddPoint(const MyPoint& newPoint)
{
	if(m_Points.size() >= m_MaxPointNum)
		return;

	m_Points.push_back(MyPoint(newPoint));
}

bool MyPolygon::IsIn(const MyPoint& checkPoint)
{
	Edge checkEdge(checkPoint, MyPoint(0, 0));
	std::vector<Edge> myEdges;
	MyPoint prevPoint(0, 0);
	for(auto point : m_Points)
	{
		if(prevPoint._x != 0 && prevPoint._y != 0)
		{
			myEdges.push_back(Edge(prevPoint + m_Origin, point + m_Origin));
		}
		prevPoint = point;
	}
	myEdges.push_back(Edge(prevPoint + m_Origin, 
		*m_Points.begin() + m_Origin));
	int crossCount = 0;
	for(auto edge : myEdges)
	{
		if(checkEdge.IsCrossed(edge))
		{
			crossCount++;
		}
	}
	
	bool isIn = (crossCount % 2 == 1);
	return isIn;
}

void MyPolygon::Draw(HDC hdc)
{
	HBRUSH myBrush;
	HBRUSH oldBrush;
	myBrush = CreateSolidBrush(RGB(m_Color._r, m_Color._g, m_Color._b));
	oldBrush = (HBRUSH) SelectObject(hdc, myBrush);

	POINT drawPoints[m_MaxPointNum] = {0, };
	int idx = 0;
	for(auto point : m_Points)
	{
		drawPoints[idx].x = static_cast<LONG>(point._x + m_Origin._x);
		drawPoints[idx++].y = static_cast<LONG>( point._y + m_Origin._y);
	}

	Polygon(hdc, drawPoints, idx);
	SelectObject(hdc, oldBrush);
	DeleteObject(myBrush);
}

void MyPolygon::CheckIn(const MyPoint& checkPoint)
{
	if(IsIn(checkPoint))
	{
		SetColor(255, 0, 0);
	}
	else
	{
		SetColor(255, 255, 255);
	}
}

void MyPolygon::Update(float dTime)
{
	float velUnit = 100.f;
	float xVelocity = 0.f;
	float yVelocity = 0.f;

	if(GameManager::GetInstance()->GetKeyState(VK_UP) & HOLDKEY)
	{
		yVelocity -= velUnit;
	}

	if(GameManager::GetInstance()->GetKeyState(VK_DOWN) & HOLDKEY)
	{
		yVelocity += velUnit;
	}

	if(GameManager::GetInstance()->GetKeyState(VK_LEFT) & HOLDKEY)
	{
		xVelocity -= velUnit;
	}

	if(GameManager::GetInstance()->GetKeyState(VK_RIGHT) & HOLDKEY)
	{
		xVelocity += velUnit;
	}

	m_Origin._x += xVelocity * dTime;
	m_Origin._y += yVelocity * dTime;
}