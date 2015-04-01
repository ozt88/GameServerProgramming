#include "pch.h"
#include "AbstractFeature.h"

AbstractFeature::AbstractFeature()
	:m_Origin(), m_Color()
{
}


AbstractFeature::~AbstractFeature()
{
}

void AbstractFeature::SetPosition(float posX, float posY)
{
	m_Origin._x = posX;
	m_Origin._y = posY;
}

void AbstractFeature::SetColor(int r, int g, int b)
{
	m_Color._r = r;
	m_Color._g = g;
	m_Color._b = b;
}
