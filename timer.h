#pragma once

#define FRAMES_PER_SECOND	(90)
#define TICKS_PER_FRAME		(1)
#define TICKS_PER_SECOND	(TICKS_PER_FRAME * FRAMES_PER_SECOND)

class FrameLimiter
{
public:
	FrameLimiter() :
		TIME_Frequency(0.),
		TIME_Ticks(0.)
	{ }

	void Init();
	DWORD Sync();

private:
	double TIME_Frequency,
		TIME_Ticks;

	void Ticks();
};
