class  ActionCarLockCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(1);
	}
};

class  ActionCarLock: ActionInteractBase
{
	void  ActionCarLock()
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
		return "Lock Car";
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (GetGame().IsServer())
			return true;

		CarScript ntarget = CarScript.Cast(target.GetObject());
		if (!ntarget)
			return false;

		// Show lock action only when car is unlocked and has an owner
		return (!ntarget.m_CarLock_IsLocked && ntarget.CarLockOwner != -1);
	}

	override void OnStartServer(ActionData action_data)
	{
		CarScript car = CarScript.Cast(action_data.m_Target.GetObject());
		PlayerBase player = action_data.m_Player;
		
		if (!car || !player)
		{
			return;
		}

		// Verify car is unlocked
		if (car.m_CarLock_IsLocked)
		{
			player.MessageImportant("Carro ja esta trancado!");
			return;
		}

		// Verify ownership
		bool canLock = false;

		// Check if player is owner
		if (car.CarLockOwner == player.GetCarLockLowId())
		{
			canLock = true;
		}

		// Check if player has password
		if (!canLock && player.HasPassword(car.CarLockPassword, car.CarLockOwner))
		{
			canLock = true;
		}

		// Check if player is admin
		if (!canLock && player.IsCarLockAdmin())
		{
			canLock = true;
		}

		if (canLock)
		{
			car.SetCarLock(true);
			car.SetSoundToPlay(1);
			player.MessageImportant("Carro trancado!");
		}
		else
		{
			player.MessageImportant("Voce nao tem permissao para trancar este carro!");
		}
	}
}
