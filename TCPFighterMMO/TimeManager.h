#pragma once

class TimeManager
{
private:
	TimeManager() {};
	~TimeManager() {};

	unsigned int nextTick;
	unsigned int oldTick;
	int frameTime = 20;

public:
	static TimeManager* GetInstance(void)
	{
		static TimeManager Sys;
		return &Sys;
	}

public:
	void TimeInit();
	// 프레임 설정 함수, 기본값은 50fps임.
	void SetFPS(int FPS)
	{
		frameTime = 1000 / FPS;
	}
	bool CheckFrameTime();
};
