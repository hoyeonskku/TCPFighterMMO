#pragma once

class TimeManager
{
private:
	TimeManager() {};
	~TimeManager() {};

	unsigned int nextTick;
	unsigned int oldTick;
	int frameTime = 40;

public:
	static TimeManager* GetInstance(void)
	{
		static TimeManager Sys;
		return &Sys;
	}

public:
	void Init();
	// 프레임 설정 함수, 기본값은 50fps임.
	void SetFPS(int FPS)
	{
		frameTime = 1000 / FPS;
	}
	bool CheckFrameTime();
};
