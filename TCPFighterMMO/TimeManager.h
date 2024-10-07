#pragma once

class TimeManager
{
private:
	TimeManager() {};
	~TimeManager() {};

	unsigned int nextTick;
	unsigned int frameTick;
	unsigned int oldTick;
	unsigned int currentTick;
	int logicFrameTime = 40;
	int fps = 0;

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
		logicFrameTime = 1000 / FPS;
	}
	bool CheckLogicFrameTime();
	unsigned int GetCurrentTick();
};
