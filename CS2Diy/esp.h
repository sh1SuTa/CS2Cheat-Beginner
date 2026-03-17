#pragma once
#include"cheat.h"
#include"tools/Mem.h"

namespace 透视 {
	struct D2D
	{
		float x, y;
	};
}

void 绘制框框();
bool 计算2D方框大小();
透视::D2D 获取左边框();
透视::D2D 获取右边框();
透视::D2D 获取上边框();
透视::D2D 获取下边框();

void 绘制自适应方框();
