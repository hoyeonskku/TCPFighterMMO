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
	// ������ ���� �Լ�, �⺻���� 50fps��.
	void SetFPS(int FPS)
	{
		logicFrameTime = 1000 / FPS;
	}
	bool CheckLogicFrameTime();
	unsigned int GetCurrentTick();
};
