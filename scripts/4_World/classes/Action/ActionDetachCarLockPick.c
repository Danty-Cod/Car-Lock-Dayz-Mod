class  ActionDetachCarLockPickCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		// Tempo do arrombamento vem do JSON (LockpickActionSeconds). O config e'
		// enviado pro cliente no connect, por isso cliente e servidor contam igual.
		float actionTime = 150;

		CarLockSettings pickSettings = GetCarLockSettings();
		if (pickSettings && pickSettings.LockpickActionSeconds > 0)
			actionTime = pickSettings.LockpickActionSeconds;

		m_ActionData.m_ActionComponent = new CAContinuousTime(actionTime);
	}
};

class  ActionDetachCarLockPick: ActionContinuousBase
{
	void  ActionDetachCarLockPick()
	{
		m_CallbackClass =  ActionDetachCarLockPickCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
	}

	override void CreateConditionComponents()
	{
    m_ConditionTarget = new CCTNonRuined(10);
		m_ConditionItem = new CCINonRuined;
	}

	override string GetText()
	{
		return "Arrombar carro";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ( GetGame().IsServer())
			return true;

		CarScript ntarget = CarScript.Cast( target.GetObject() );

		// Ja' esta destravado pelo lockpick: nao deixa arrombar de novo (evita
		// ficar renovando o prazo em cima do mesmo carro).
		if (ntarget && ntarget.IsCarLockBypassed())
			return false;

		if (ntarget && ntarget.CarLockOwner != -1)
		{
			bool IsEmpty = true;
			Transport transport = Transport.Cast(target.GetObject());
			if (!transport)return false;

			int crewSize = transport.CrewSize();
			for (int j = 0; j < crewSize; j++)
			{
				if (transport.CrewMember(j))
					IsEmpty = false;
			}

			if (IsEmpty)return true;
		}
		return false;
	}

	override void OnFinishProgressServer( ActionData action_data )
	{
		CarScript car = CarScript.Cast(action_data.m_Target.GetObject());
		PlayerBase player = action_data.m_Player;
		
		if (!car || !player)
			return;

		// ====================================================================
		// NAO remove mais o CarLock.
		// --------------------------------------------------------------------
		// O CarLock e' tambem a apolice do SEGURO do dono. Arrancar o cadeado
		// apagava o seguro de quem pagou, o que dava pra usar como sacanagem.
		// Agora o lockpick DESTRAVA o carro por um tempo: o ladrao entra, pega o
		// que quiser e usa o carro, e depois do prazo o carro tranca sozinho com
		// o dono e a senha originais - cadeado e seguro intactos.
		// ====================================================================
		CarLockSettings settings = GetCarLockSettings();

		int bypassSeconds = 1800;
		int carDamage     = 500;
		float breakChance = 0.3;
		int pickDamage    = 50;

		if (settings)
		{
			if (settings.LockpickBypassSeconds > 0)
				bypassSeconds = settings.LockpickBypassSeconds;

			carDamage   = settings.LockpickDamage;
			breakChance = settings.LockpickBreakChance;
			pickDamage  = settings.LockpickDamageToPick;
		}

		car.StartCarLockBypass(bypassSeconds);

		// Ativa alarme
		car.SetSoundToPlay(3);

		// Causa dano ao carro
		if (carDamage > 0)
			car.AddHealth("", "", -carDamage);

		// A ferramenta SEMPRE se gasta ao terminar o arrombamento.
		// Com LockpickBreakChance ela e' ARRUINADA de vez (quebra); fora isso
		// leva LockpickDamageToPick de dano. Antes o codigo so' dava dano, e com
		// chance de 0.3 dava pra arrombar varias vezes sem gastar nada.
		if (action_data.m_MainItem)
		{
			if (Math.RandomFloat01() < breakChance)
			{
				action_data.m_MainItem.SetHealth("", "", 0.0);
				player.MessageImportant("O lockpick quebrou!");
			}
			else if (pickDamage > 0)
			{
				action_data.m_MainItem.DecreaseHealth(pickDamage, false);
			}
		}

		int bypassMin = bypassSeconds / 60;
		player.MessageImportant("Carro destravado por " + bypassMin + " minutos! Alarme ativado!");
	}
};
