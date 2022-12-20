#pragma once

#include "CubicBezier.h"

class AirBrushData
{
public:
	constexpr static int SampleCount = 32;
	GLuint Gradient = 0;
	Geom::CubicBezier Curve;

	AirBrushData();

	AirBrushData(const AirBrushData& other);
	AirBrushData(AirBrushData&& other) noexcept;
	AirBrushData& operator=(AirBrushData other);

	~AirBrushData();
	void UpdateGradient();
	void SetUniform();
};

