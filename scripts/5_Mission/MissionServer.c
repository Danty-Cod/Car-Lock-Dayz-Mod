modded class MissionServer extends MissionBase
{
	protected bool m_IsInitialized = false;
	
	void MissionServer()
	{
		// sem licença/keys: o mod sempre inicia
		m_IsInitialized = true;
		
		m_CarLockCore = NULL;
		GetCarLockCore();
	}
	
	bool IsInitialized()
	{
		return m_IsInitialized;
	}

	void ~MissionServer() {
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player,identity);
		player.SetLowSteamUID();
		GetCarLockCore().SendConfigToClient(player);
	}
};
