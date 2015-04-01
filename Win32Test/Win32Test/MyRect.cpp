#include "pch.h"
#include "MyRect.h"

MyRect::MyRect(MyPoint origin, float width, float height)
	:m_Width(width), m_Height(height), m_Angle(0.f)
{
	SetColor(255, 255, 0);
}

MyRect::MyRect()
	: m_Angle(0.f), m_Width(0.f), m_Height(0.f)
{
	SetColor(255, 255, 0);
}


MyRect::~MyRect()
{
}

void MyRect::Draw(HDC hdc)
{
	HBRUSH myBrush;
	HBRUSH oldBrush;
	myBrush = CreateSolidBrush(RGB(m_Color._r, m_Color._g, m_Color._b));
	oldBrush = (HBRUSH) SelectObject(hdc, myBrush);
	
	POINT pointArr[RPT_MAX] = {0, };
	GetPoints(pointArr);
	Polygon(hdc, pointArr, RPT_MAX);

	SelectObject(hdc, oldBrush);
	DeleteObject(myBrush);
}

void MyRect::GetPoints(POINT* inputArr)
{
	MyPoint pointArr[RPT_MAX];
	pointArr[RPT_UPLEFT]._x = -m_Width / 2;
	pointArr[RPT_UPLEFT]._y = m_Height / 2;

	pointArr[RPT_UPRIGHT]._x = m_Width / 2;
	pointArr[RPT_UPRIGHT]._y = m_Height / 2;

	pointArr[RPT_BOTTOMLEFT]._x = -m_Width / 2;
	pointArr[RPT_BOTTOMLEFT]._y = -m_Height / 2;

	pointArr[RPT_BOTTOMRIGHT]._x = m_Width / 2;
	pointArr[RPT_BOTTOMRIGHT]._y = -m_Height / 2;

	float radian = (m_Angle * 3.141592f) / 180.f;
	for(int i = 0; i < RPT_MAX; ++i)
	{
		float x = pointArr[i]._x;
		float y = pointArr[i]._y;
		pointArr[i]._x = x * cos(radian) - y * sin(radian) + m_Origin._x;
		pointArr[i]._y = x * sin(radian) + y * cos(radian) + m_Origin._y;
		inputArr[i] = pointArr[i];
	}


}

void MyRect::SetSize(float width, float height)
{
	m_Width = width;
	m_Height = height;
}

void MyRect::Update(float deltaT)
{
	m_Angle += 30.f * deltaT;
	if(m_Angle > 360.f)
	{
		m_Angle -= 360.f;
	}
}

void MyRect::CheckCollide(const MyRect& other)
{
	if(IsCollide(other))
	{
		SetColor(255, 0, 0);
	}
	else
	{
		SetColor(255, 255, 0);
	}
}

bool MyRect::IsCollide(const MyRect& other)
{
	MyPoint otherOrigin(other.m_Origin._x - m_Origin._x, other.m_Origin._y - m_Origin._y);
	otherOrigin.Rotate(-m_Angle);

	float myTop = - m_Height / 2;
	float myBottom = m_Height / 2;
	float otherBottom = otherOrigin._y + other.m_Height / 2;
	float otherTop = otherOrigin._y - other.m_Height / 2;
	float myLeft = - m_Width / 2;
	float myRight = m_Width / 2;
	float otherRight = otherOrigin._x + other.m_Width / 2;
	float otherLeft = otherOrigin._x - other.m_Width / 2;

	if(otherBottom < myTop)
		return false;
	else if(otherRight < myLeft)
		return false;
	else if(myBottom < otherTop)
		return false;
	else if(myRight < otherLeft)
		return false;
	else
		return true;
}

