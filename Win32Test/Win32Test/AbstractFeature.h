#pragma once
#include "BasicFeatures.h"

class AbstractFeature
{
public:
	AbstractFeature();
	~AbstractFeature();

	virtual void	Draw(HDC hdc) = 0;

	void			SetPosition(float posX, float posY);
	void			SetColor(int r, int g, int b);

protected:
	MyPoint		m_Origin;
	Color		m_Color;
};

