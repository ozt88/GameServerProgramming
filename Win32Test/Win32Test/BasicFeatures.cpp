#include "pch.h"
#include "BasicFeatures.h"


MyPoint::MyPoint()
	:_x(0.f), _y(0.f)
{
}

MyPoint::MyPoint(float x, float y)
	: _x(x), _y(y)
{
}

MyPoint::~MyPoint()
{
}

float MyPoint::GetDistance(const MyPoint& other)
{
	return sqrt(pow(_x - other._x, 2) + pow(_y - other._y, 2));
}

void MyPoint::Rotate(float angle)
{
	float radian = (angle * 3.1415926535f) / 180.f;
	float tmpX = _x;
	float tmpY = _y;
	_x = tmpX * cos(radian) - tmpY * sin(radian);
	_y = tmpX * sin(radian) + tmpY * cos(radian);
}

MyPoint::operator POINT() const
{
	POINT ret;
	ret.x = (LONG)_x, ret.y = (LONG)_y;
	return ret;
}

MyPoint MyPoint::operator+( const MyPoint& rhs )
{
	return MyPoint(_x + rhs._x, _y + rhs._y);
}

Line::Line()
	:_a(0), _b(0), _c(0)
{
}

Line::~Line()
{
}

float Line::GetDistanceFromPoint(float posX, float posY)
{
	return (float)abs(_a*posX + _b*posY + _c) 
			/ (float)sqrt(pow(_a, 2) + pow(_b, 2));
}

float Line::GetPosYFromX(float posX)
{
	return ( -_a * posX - _c ) / _b;
}

bool Edge::IsCrossed(const Edge& other)
{
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
	float Ax = _head._x;
	float Ay = _head._y;
	float Bx = _tail._x;
	float By = _tail._y;
	float Cx = other._head._x;
	float Cy = other._head._y;
	float Dx = other._tail._x;
	float Dy = other._tail._y;

	if(MAX(Ax, Bx) < MIN(Cx, Dx)) 
		return false;
	if(MIN(Ax, Bx) > MAX(Cx, Dx)) 
		return false;
	if(MAX(Ay, By) < MIN(Cy, Dy)) 
		return false;
	if(MIN(Ay, By) > MAX(Cy, Dy)) 
		return false;

	/* line AB & segment CD */
#define f_AB(x,y) ((y - Ay) * (Bx - Ax) - (x - Ax) * (By - Ay))
	if(( f_AB(Cx, Cy) * f_AB(Dx, Dy) ) > 0) 
		return false;

	/* line CD & segment AB */
#define f_CD(x,y) ((y - Cy) * (Dx - Cx) - (x - Cx) * (Dy - Cy))
	if(( f_CD(Ax, Ay) * f_CD(Bx, By) ) > 0) 
		return false;

	return true;
}

Edge::Edge()
	:_head(), _tail()
{
}

Edge::Edge(const MyPoint& head, const MyPoint& tail)
	: _head(head), _tail(tail)
{
}

Edge::~Edge()
{
}
