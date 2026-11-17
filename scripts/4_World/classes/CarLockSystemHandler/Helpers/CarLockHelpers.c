static ref CarLockCore m_CarLockCore;
static CarLockCore GetCarLockCore() {
	if ( !m_CarLockCore ) {
		m_CarLockCore = new CarLockCore;
	}
	return m_CarLockCore;
}

static CarLockSettings GetCarLockSettings() {
	return GetCarLockCore().m_CarLockSettings;
}

static PlayerBase CLGetPlayerByIdentity(PlayerIdentity sender)
{
	int	low	 =	0;
	int	high =	0;
	GetGame().GetPlayerNetworkIDByIdentityID( sender.GetPlayerId(), low, high );
	return PlayerBase.Cast( GetGame().GetObjectByNetworkId(low, high ));
}
