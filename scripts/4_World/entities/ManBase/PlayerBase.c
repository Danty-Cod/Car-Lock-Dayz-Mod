modded class PlayerBase extends ManBase
{
	int CarLockPhilmrissLowId=-1;

	override void Init()
	{
		super.Init();
		RegisterNetSyncVariableInt("CarLockPhilmrissLowId");
	}

	void SetLowSteamUID()
	{
		// Protecao: em alguns momentos (ex: durante criacao do player) a
		// identidade ainda nao existe. Sem isso, dava erro e o id ficava -1.
		if (!GetIdentity())
			return;

		CarLockPhilmrissLowId = GetIdentity().GetPlainId().Substring(8, 9).ToInt();
		SetSynchDirty();
	}

	// Retorna o low id do player, RECALCULANDO sozinho se ainda estiver -1.
	// ---------------------------------------------------------------------
	// IMPORTANTE: esse id e' a CHAVE do banco de senhas (CarLock_<id>.json) e
	// da checagem de dono. Antes, o SetLowSteamUID so' era chamado no connect,
	// entao quando o player MORRIA e RENASCIA (novo PlayerBase, id volta pra
	// -1) ele perdia dono/senha ate' relogar -> era o "as vezes nao funciona".
	// Agora, sempre que a identidade estiver disponivel, o id se recupera.
	int GetCarLockLowId()
	{
		if (CarLockPhilmrissLowId == -1)
			SetLowSteamUID();
		return CarLockPhilmrissLowId;
	}

	bool IsCarLockAdmin()
	{
		PlayerIdentity ident = GetIdentity();
		if (!ident)
			return false;

		// Compara o SteamID COMPLETO (exato) com a lista de admins.
		// ANTES usava uid.Contains(lowId) - continencia de texto com um id
		// cortado/sem zeros a esquerda, o que podia marcar players errados
		// como admin (e admin destranca qualquer carro sem senha).
		string fullId = ident.GetPlainId();

		TStringArray admins = GetCarLockSettings().Admins;
		foreach(string uid: admins)
		{
			if(uid == fullId)
				return true;
		}

		return false;
	}

	bool HasPassword(int pwd, int id)
	{
		CarLockData carLockData = CarLockData.Load(GetCarLockLowId().ToString());
		if(carLockData.CarLockPasswords.Count()>0)
		{
			for(int i=0;i<carLockData.CarLockPasswords.Count();i++)
			{
				if(carLockData.CarLockPasswords[i] == -1)
					continue;

				if(carLockData.CarLockPasswords[i] == pwd)
					return true;
			}
		}
		return false;
	}

	// Salva uma senha no banco do proprio player (lado SERVIDOR).
	// Usado quando o player digita a senha correta de um carro de outro
	// jogador (compartilhamento de senha). Assim o HasPassword passa a
	// retornar true para ele nas proximas vezes.
	void AddPassword(int pwd)
	{
		if (!GetGame().IsServer())
			return;
		if (pwd == -1)
			return;

		CarLockData carLockData = CarLockData.Load(GetCarLockLowId().ToString());
		if (carLockData.CarLockPasswords.Find(pwd) == -1)
		{
			carLockData.CarLockPasswords.Insert(pwd);
			carLockData.Save(CarLockPhilmrissLowId.ToString());
		}
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionCarLockMenu);
		AddAction(ActionCarLock);
		AddAction(ActionCarUnLock);
	}
};
