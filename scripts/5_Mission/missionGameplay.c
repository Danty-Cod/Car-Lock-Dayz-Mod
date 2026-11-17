modded class MissionGameplay extends MissionBase
{
	void MissionGameplay()
	{
		m_CarLockCore = NULL;
		GetCarLockCore();
	}

	void TryToLockVehicle(PlayerBase player)
	{
		if (!player)
			return;

		CarScript car = NULL;

		HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();
		if (vehCommand)
		{
			Transport transport = vehCommand.GetTransport();
			if (transport)
			{
				int crewIndex = transport.CrewMemberIndex(player);
				if (crewIndex == DayZPlayerConstants.VEHICLESEAT_DRIVER)
				{
					car = CarScript.Cast(transport);
				}
			}
		}

		if (!car)
		{
			array<Object> objs = new array<Object>;
			GetGame().GetObjectsAtPosition3D(player.GetPosition(), 1, objs, NULL);
			
			if (objs.Count() > 0)
			{
				for (int i = 0; i < objs.Count(); i++)
				{
					car = CarScript.Cast(objs[i]);
					if (car)
						break;
				}
			}
		}

		if (!car || car.CarLockOwner == -1)
			return;

		// A permissao e' validada NO SERVIDOR (CarLockServer.CarLockRequest).
		// O HasPassword do lado cliente nao e' confiavel (o banco de senhas
		// fica no servidor), entao apenas enviamos o pedido e deixamos o
		// servidor decidir. Players sem permissao recebem aviso do servidor.
		if (!car.m_CarLock_IsLocked)
		{
			GetRPCManager().SendRPC("CarLock", "CarLockRequest", new Param2<CarScript, bool>(car, true), true, NULL);
		}
		else
		{
			GetRPCManager().SendRPC("CarLock", "CarLockRequest", new Param2<CarScript, bool>(car, false), true, NULL);
		}
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		if (!GetUIManager().GetMenu() && GetUApi().GetInputByName("UACarLockToggle").LocalPress())
		{
			TryToLockVehicle(player);
		}
	}
};
