#include "GameLogic.h"
#include "Session.h"
#include "SessionManager.h"
#include "ObjectManager.h"
#include "TimeManager.h"
#include "Player.h"
#include <list>
#include "Windows.h"
#include <iostream>


int dir[8][2] = { {0, -3}, {-2, -3}, {-2, 0}, {-2, 3}, {0, 3}, {2, 3}, {2, 0}, {2, -3} };

void Update()
{
	if (!TimeManager::GetInstance()->CheckFrameTime())
		return;

	for (auto& pair : ObjectManager::GetInstance()->GetObjectMap())
	{
		int tempX, tempY;
		if (pair.second->moveFlag == true)
		{
			tempY = pair.second->y + dir[pair.second->dir][0];
			tempX = pair.second->x + dir[pair.second->dir][1];

			// 범위 밖으로 벗어나면
			if (tempX < dfRANGE_MOVE_LEFT)
			{
				//session->player->x = dfRANGE_MOVE_LEFT;
				continue;
			}
			if (tempX > dfRANGE_MOVE_RIGHT)
			{
				//session->player->x = dfRANGE_MOVE_RIGHT;
				continue;
			}
			if (tempY < dfRANGE_MOVE_TOP)
			{
				//session->player->y = dfRANGE_MOVE_TOP;
				continue;
			}
			if (tempY > dfRANGE_MOVE_BOTTOM)
			{
				//session->player->y = dfRANGE_MOVE_BOTTOM;
				continue;
			}
			// 조건에 부합하는 경우에만 값 갱신
			pair.second->y = tempY;
			pair.second->x = tempX;
		}
	}

	ObjectManager::GetInstance()->RemovePlayers();
}
