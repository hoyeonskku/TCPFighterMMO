#pragma once

class TimeManager
{
private:
	TimeManager() {};
	~TimeManager() {};

	unsigned int nextTick;
	unsigned int prevTick;
	unsigned int frameTick;
	unsigned int oldTick;
	unsigned int currentTick;
	int logicFrameTime = 40;

public:
	static TimeManager* GetInstance(void)
	{
		static TimeManager Sys;
		return &Sys;
	}

	int currentFrameCount = 0;
	int fps = 0;

public:
	void Init();
	// ������ ���� �Լ�, �⺻���� 25fps��.
	void SetFPS(int FPS)
	{
		logicFrameTime = 1000 / FPS;
	}
	bool CheckLogicFrameTime();
	unsigned int GetCurrentTick();
};
