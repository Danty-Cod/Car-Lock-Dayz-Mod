class CarLockCore
{
	ref CarLockServer m_CarLockServer;
	ref CarLockSettings m_CarLockSettings;
	ref CarLockMenu iCarLockMenu;

	void CarLockCore()
	{
		Print("CarLockCore - Started ! philmriss#7137");

		if (GetGame().IsServer() || !GetGame().IsMultiplayer())
		{
			InitServerFolder();
			m_CarLockSettings = CarLockSettings.Load();
			m_CarLockServer = new CarLockServer;
		}
		else
		{
			InitClientFolder();
			m_CarLockSettings = new CarLockSettings;
		}
		
		InitRPC();
	}

	void ~CarLockCore()
	{
	}

	void InitClientFolder()
	{
		if (!FileExist("$saves:SimpleModParty\\"))
		{
			MakeDirectory("$saves:SimpleModParty\\");
		}
		
		if (!FileExist(CARLOCK_CONFIG_ROOT_CLIENT))
		{
			MakeDirectory(CARLOCK_CONFIG_ROOT_CLIENT);
			if (!FileExist(CARLOCK_CONFIG_DIR_CLIENT))
				MakeDirectory(CARLOCK_CONFIG_DIR_CLIENT);
		}
	}

	void InitServerFolder()
	{
		if (!FileExist("$profile:SimpleModParty\\"))
		{
			MakeDirectory("$profile:SimpleModParty\\");
		}
		
		if (!FileExist(CARLOCK_CONFIG_ROOT_SERVER))
		{
			MakeDirectory(CARLOCK_CONFIG_ROOT_SERVER);
			if (!FileExist(CARLOCK_CONFIG_DIR_SERVER))
				MakeDirectory(CARLOCK_CONFIG_DIR_SERVER);
		}
	}

	void SendConfigToClient(PlayerBase player)
	{
		GetRPCManager().SendRPC("CarLock", "CarLockConfigResponse", new Param1<ref CarLockSettings>(m_CarLockSettings), true, player.GetIdentity());
	}

	void InitRPC()
	{
		if (GetGame().IsServer())
		{
			GetRPCManager().AddRPC("CarLock", "CarLockRequest", m_CarLockServer, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("CarLock", "CarLockPasswordRequest", m_CarLockServer, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("CarLock", "CarLockTryPasswordRequest", m_CarLockServer, SingeplayerExecutionType.Client);
			GetRPCManager().AddRPC("CarLock", "CarLockNameRequest", m_CarLockServer, SingeplayerExecutionType.Client);
		}
		else
		{
			GetRPCManager().AddRPC("CarLock", "CarLockConfigResponse", this, SingeplayerExecutionType.Server);
			GetRPCManager().AddRPC("CarLock", "CarLockNameSync", this, SingeplayerExecutionType.Server);
		}
	}

	void CarLockConfigResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetGame().IsClient())
			return;

		Param1<ref CarLockSettings> data;
		if (!ctx.Read(data))
			return;

		m_CarLockSettings = data.param1;
	}

	// CLIENTE: recebe o nome do dono de um carro (broadcast ou resposta a pedido)
	// e guarda no proprio carro (via target). "" = carro sem dono.
	void CarLockNameSync(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetGame().IsClient())
			return;

		Param1<string> data;
		if (!ctx.Read(data))
			return;

		CarScript car = CarScript.Cast(target);
		if (!car)
			return;

		car.SetCarLockOwnerNameClient(data.param1);
	}
};
