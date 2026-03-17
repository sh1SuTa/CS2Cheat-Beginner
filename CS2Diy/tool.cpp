

#include"tool.h"


void 改fov角度() {
    工具::Process cs2(TEXT("cs2.exe"));
    
    //寻找dll模块
    auto client = cs2.get_module_handle(TEXT("client.dll"));
    std::cout << XorStr("寻找dll模块完成：")<<client << std::endl;
    //addr64 8字节的无符号long long
    auto local_player = cs2.read<工具::addr64>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
    
    //指向摄像机服务的指针
    auto cam_ser = cs2.read<工具::addr64>(local_player + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pCameraServices);
    
    //fov的地址
    auto fov_addr = cam_ser + cs2_dumper::schemas::client_dll::CCSPlayerBase_CameraServices::m_iFOV;
    //cs2.write(fov_addr, Menu::视野角度);

    mem::Write( (CHAR*)fov_addr, &Menu::视野角度, sizeof(Menu::视野角度));
    
    

}

void 改fov角度2() {
    gameAddress::clientAddress + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn;
}

void 旋转大陀螺() {
    
            float 转速 = 1.f;
            mem::Write(gameAddress::clientAddress + 视角::yam, &工具::初始值, sizeof(工具::初始值));
            工具::初始值 = 工具::初始值 + 转速;
            if (工具::初始值 == 180)
            {
                工具::初始值 = -180;
            }

}

void 一直跳() {
    Sleep(10);
	if (gameAddress::flag == 65665)
	{
		mem::Write(gameAddress::clientAddress + buttons::jump, &gameAddress::jumpOn, sizeof(gameAddress::jumpOn));
	}
    if (gameAddress::flag==65664)
    {
        mem::Write(gameAddress::clientAddress + buttons::jump, &gameAddress::jumpOff, sizeof(gameAddress::jumpOff));
    }

	

	

 
}
void 作弊线程1() {
    while (true)
    {
        if (Menu::旋转启用)
        {
            旋转大陀螺();
        }
		mem::Read(gameAddress::g_handle, gameAddress::clientAddress + offsets::client_dll::dwLocalPlayerPawn, &gameAddress::localAddress, sizeof(gameAddress::localAddress));
        mem::Read(gameAddress::g_handle, gameAddress::localAddress + schemas::client_dll::C_BaseEntity::m_fFlags, &gameAddress::flag, sizeof(gameAddress::flag));
        if (GetAsyncKeyState(VK_SPACE)) {
            一直跳();
		}
		else {
			mem::Write(gameAddress::clientAddress + buttons::jump, &gameAddress::jumpOff, sizeof(gameAddress::jumpOff));
		}
        
    }

}

