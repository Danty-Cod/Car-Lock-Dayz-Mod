class ActionCarUnLock: ActionInteractBase
{
	void ActionCarUnLock()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
		m_ConditionItem = new CCINone;
	}

	override string GetText()
	{
		return "UnLock Car";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (GetGame().IsServer())
			return true;

		CarScript ntarget = CarScript.Cast(target.GetObject());
		if (!ntarget)
			return false;

		// Show unlock action only when car is locked and has an owner
		return (ntarget.m_CarLock_IsLocked && ntarget.CarLockOwner != -1);
	}

	override void OnStartServer(ActionData action_data)
	{
		CarScript car = CarScript.Cast(action_data.m_Target.GetObject());
		PlayerBase player = action_data.m_Player;
		
		if (!car || !player)
		{
			return;
		}

		// Verify car is locked
		if (!car.m_CarLock_IsLocked)
		{
			player.MessageImportant("Carro ja esta destrancado!");
			return;
		}

		// Verify ownership
		bool canUnlock = false;

		// Check if player is owner
		if (car.CarLockOwner == player.GetCarLockLowId())
		{
			canUnlock = true;
		}

		// Check if player has password
		if (!canUnlock && player.HasPassword(car.CarLockPassword, car.CarLockOwner))
		{
			canUnlock = true;
		}

		// Check if player is admin
		if (!canUnlock && player.IsCarLockAdmin())
		{
			canUnlock = true;
		}

		if (canUnlock)
		{
			car.SetCarLock(false);
			car.SetSoundToPlay(2);
			player.MessageImportant("Carro destrancado!");
		}
		else
		{
			player.MessageImportant("Voce nao tem permissao para destrancar este carro!");
		}
	}
}
