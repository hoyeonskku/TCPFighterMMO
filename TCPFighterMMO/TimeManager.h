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
	void Init();
	// ������ ���� �Լ�, �⺻���� 50fps��.
	void SetFPS(int FPS)
	{
		frameTime = 1000 / FPS;
	}
	bool CheckFrameTime();
};
