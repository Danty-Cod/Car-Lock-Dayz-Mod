class CarLockServer
{
	void CarLockServer()
	{
	}

	void CarLockRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetGame().IsServer())
			return;

		Param2<CarScript, bool> data;
		if (!ctx.Read(data))
			return;

		CarScript car = data.param1;
		if (!car)
			return;

		PlayerBase player = CLGetPlayerByIdentity(sender);
		if (!player)
			return;

		// Validacao de permissao NO SERVIDOR (antes nao havia nenhuma, qualquer
		// um podia trancar/destrancar via RPC). Dono, quem tem a senha, ou admin.
		bool allowed = (car.CarLockOwner == player.GetCarLockLowId()) || player.HasPassword(car.CarLockPassword, car.CarLockOwner) || player.IsCarLockAdmin();

		if (!allowed)
		{
			player.MessageImportant("Voce nao tem permissao para este carro!");
			return;
		}

		car.SetCarLock(data.param2);

		if (data.param2)
			car.SetSoundToPlay(1);
		else
			car.SetSoundToPlay(2);
	}

	void CarLockPasswordRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetGame().IsServer())
			return;

		Param2<CarScript, int> data;
		if (!ctx.Read(data))
			return;

		CarScript car = data.param1;
		if (!car)
			return;

		PlayerBase player = CLGetPlayerByIdentity(sender);
		if (!player)
			return;

		car.SetCarLockOwner(player.GetCarLockLowId());
		car.SetCarLock(true);
		car.SetSoundToPlay(1);

		// Guarda o nome do jogador que colocou o CarLock e avisa os clientes.
		if (sender)
			car.SetCarLockOwnerName(sender.GetName());

		if (data.param2 != -1)
		{
			car.SetCarLockPassword(data.param2);
			// Salva a senha tambem no banco do dono (server-side), para o
			// HasPassword funcionar de forma consistente.
			player.AddPassword(data.param2);
		}
	}

	// ========================================================================
	// CarLockNameRequest - o CLIENTE pede o nome do dono de um carro (quando o
	// carro carregou/entrou depois e ele ainda nao sabe o nome). O servidor
	// responde SO' pra quem pediu, com o nome atual do dono.
	// ========================================================================
	void CarLockNameRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetGame().IsServer())
			return;

		Param1<int> data;
		if (!ctx.Read(data))
			return;

		CarScript car = CarScript.Cast(target);
		if (!car || !sender)
			return;

		GetRPCManager().SendRPC("CarLock", "CarLockNameSync", new Param1<string>(car.CarLockOwnerName), true, sender, car);
	}

	// ========================================================================
	// CarLockTryPasswordRequest - player digitou a senha de um carro de outro
	// jogador (compartilhamento de senha). Validacao feita NO SERVIDOR.
	// ------------------------------------------------------------------------
	// Se a senha bater com a do carro:
	//   - registra a senha no banco do player (AddPassword) -> a partir daqui
	//     ele pode usar as acoes normais de Trancar/Destrancar
	//   - destranca o carro na hora
	// Se nao bater: avisa "Senha incorreta".
	// ========================================================================
	void CarLockTryPasswordRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!GetGame().IsServer())
			return;

		Param2<CarScript, int> data;
		if (!ctx.Read(data))
			return;

		CarScript car = data.param1;
		if (!car)
			return;

		PlayerBase player = CLGetPlayerByIdentity(sender);
		if (!player)
			return;

		int attempt = data.param2;

		// Senha do carro precisa estar definida
		if (car.CarLockPassword == -1)
		{
			player.MessageImportant("Este carro nao tem senha definida!");
			return;
		}

		// Dono nao precisa digitar senha, mas se digitar certo, tudo bem.
		bool ok = (attempt == car.CarLockPassword) || (car.CarLockOwner == player.GetCarLockLowId()) || player.IsCarLockAdmin();

		if (ok)
		{
			player.AddPassword(car.CarLockPassword);
			car.SetCarLock(false);
			car.SetSoundToPlay(2);
			player.MessageImportant("Carro destrancado!");
		}
		else
		{
			player.MessageImportant("Senha incorreta!");
		}
	}
};
