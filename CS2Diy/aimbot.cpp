#include"cheat.h"

void 自瞄队列()
{
	//cheat::FOV = Menu::Fov*8 ;
	//获取对象玩家的三维坐标
	float World[3] = { cheat::ActorPlayer.Axis.x,cheat::ActorPlayer.Axis.y,cheat::ActorPlayer.Axis.z + 69.f };
	float screen[2];
	if (WorldScreen2d(World, screen))
	{
		//计算准星到目标的距离
		cheat::ActorDistance[0] = calculateDistance({cheat::screenWidth,cheat::screenHeight }, {screen[0],screen[1]});
		//找出最小距离
		if (cheat::ActorDistance[0]<= cheat::ActorDistance[1])
		{
			cheat::ActorDistance[1] = cheat::ActorDistance[0];
			cheat::EnterAimAddress[0] = cheat::ActorPlayer.Address[1];
			//对象骨骼地址给EnterAimAddress[1]
			cheat::EnterAimAddress[1] = cheat::ActorPlayer.SkeletonAddress[1];
		}
	}
}

void aimbot()
{
	if (cheat::LocalPlayer.Health <= 0) {
		return; // 本地玩家死亡时不进行瞄准
	}
	 

	cheat::AimAddress[0] = cheat::EnterAimAddress[0];
	cheat::AimAddress[1] = cheat::EnterAimAddress[1];
	int lin_hp;
	mem::Read(gameAddress::g_handle, cheat::AimAddress[0] + 0xB5C, &lin_hp, 4);
	// 判断对方血量是否 > 0
	if (lin_hp > 0)
	{
		cheat::FOV = Menu::Fov * 8;
		// 判断是否在 FOV 范围内
		if (cheat::FOV > cheat::ActorDistance[1])
		{
			cheat::Aimmouse = Aiming(cheat::LocalPlayer.Axis, cheat::AimAddress[1]);

			if (GetAsyncKeyState(Menu::Aimkey))
			{
				mem::Write(gameAddress::clientAddress + 视角::yam, &cheat::Aimmouse.x, sizeof(cheat::Aimmouse.x));
				mem::Write(gameAddress::clientAddress + 视角::pitch, &cheat::Aimmouse.y, sizeof(cheat::Aimmouse.y));
			}
		}
	}
	cheat::ActorDistance[1] = FLT_MAX;
}
//自瞄算法
D2D Aiming(D3D LocalAxis, char* AimAddress)
{
	char* Aimindex;
	D3D ActorAxis;
	D3D AimAxis;
	//目标轴
	D2D Aimmouse;
	float P_I = 3.1415926535f;
	//6为目标头部
	Aimindex = AimAddress + Menu::Aimplace * 32;
	mem::Read(gameAddress::g_handle, Aimindex, &ActorAxis.x, sizeof(ActorAxis.x));
	mem::Read(gameAddress::g_handle, Aimindex + 4, &ActorAxis.y, sizeof(ActorAxis.y));
	mem::Read(gameAddress::g_handle, Aimindex + 8, &ActorAxis.z, sizeof(ActorAxis.z));
	AimAxis.z = LocalAxis.z - ActorAxis.z;
	AimAxis.x = ActorAxis.x - LocalAxis.x;
	AimAxis.y = ActorAxis.y - LocalAxis.y;
	if (AimAxis.x != 0)
		Aimmouse.x = atan2(AimAxis.y, AimAxis.x) * 180 / P_I; // 使用 atan2 处理所有象限

	if (AimAxis.z != 0)
		Aimmouse.y = atan(AimAxis.z / sqrt(AimAxis.x * AimAxis.x + AimAxis.y * AimAxis.y)) * 180 / P_I;


	Aimmouse.y = atan(AimAxis.z / sqrt(AimAxis.x * AimAxis.x + AimAxis.y * AimAxis.y)) / P_I * 180;
	return Aimmouse;

}
cheat::ActorInfo 选择最近目标(const std::vector<cheat::ActorInfo>& targets) {
	cheat::ActorInfo bestTarget{};
	float minDistance = FLT_MAX;

	for (const auto& target : targets) {
		if (target.distance < minDistance) {
			minDistance = target.distance;
			bestTarget = target;
		}
	}
	return bestTarget;
}
