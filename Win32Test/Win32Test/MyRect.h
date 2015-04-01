#pragma once
#include "AbstractFeature.h"
class MyRect : public AbstractFeature
{
public:
	enum RectPointType
	{
		RPT_UPLEFT,
		RPT_UPRIGHT,
		RPT_BOTTOMRIGHT,
		RPT_BOTTOMLEFT,
		RPT_MAX,
	};

	MyRect();
	MyRect(MyPoint origin, float width, float height);
	~MyRect();

	virtual void	Draw(HDC hdc) override;
	void			Update(float deltaT);
	void			Rotate(float Angle);
	void			SetSize(float width, float height);
	void			GetPoints(POINT* pointArr);
	void			CheckCollide(const MyRect& other);
	bool			IsCollide(const MyRect& other);

private:
	float			m_Angle;
	float			m_Width;
	float			m_Height;
};
