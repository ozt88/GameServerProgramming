#pragma once
#include "AbstractFeature.h"
class MyPolygon : public AbstractFeature
{
public:
	MyPolygon();
	~MyPolygon();

	void	Draw(HDC hdc) override;
	void	Update(float dTime);
	void	AddPoint(const MyPoint& newPoint);
	bool	IsIn(const MyPoint& checkPoint);
	void	CheckIn(const MyPoint& checkPoint);
	void	Rotate(float angle);
private:
	static const unsigned int	m_MaxPointNum = 20;
	std::vector<MyPoint>		m_Points;

};

