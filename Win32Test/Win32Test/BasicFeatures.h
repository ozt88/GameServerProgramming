#pragma once
class MyPoint
{
public:
	MyPoint();
	MyPoint(float x, float y);
	~MyPoint();

	float		GetDistance(const MyPoint& other);
	void		Rotate(float angle);
	MyPoint		operator+(const MyPoint& rhs);
	operator POINT() const;

	float	_x;
	float	_y;
};


class Line
{
public:
	Line();
	Line(const MyPoint& head, const MyPoint& tail);
	Line(float posX, float posY);
	~Line();

	float GetDistanceFromPoint(float posX, float posY);
	float GetPosYFromX(float posX);

	int _a = 0;
	int _b = 0;
	int _c = 0;
};

class Edge
{
public:
	Edge();
	Edge(const MyPoint& head, const MyPoint& tail);
	~Edge();
	bool IsCrossed(const Edge& other);

private:
	MyPoint _head;
	MyPoint	_tail;
};

class Color
{
public:
	int _r = 0;
	int _g = 0;
	int _b = 0;
};
