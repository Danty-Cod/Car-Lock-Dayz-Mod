class  ActionDetachCarLockCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(20);
	}
};

class  ActionDetachCarLock: ActionContinuousBase
{
	void  ActionDetachCarLock()
	{
		m_CallbackClass =  ActionDetachCarLockCB;
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
		return "Detach CarLock";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ( GetGame().IsServer() )
				return true;

		CarScript ntarget = CarScript.Cast( target.GetObject() );
		if (ntarget && (ntarget.CarLockOwner == player.GetCarLockLowId() ||player.HasPassword(ntarget.CarLockPassword,ntarget.CarLockOwner)) || player.IsCarLockAdmin())
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
		if (car)
		{
			car.SetCarLockOwner(-1);
			car.SetCarLockPassword(-1);
			car.SetCarLock(false);
			car.SetCarLockOwnerName(""); // limpa o nome do dono (some do carro)
			
			PlayerBase player = action_data.m_Player;
			if (player)
			{
				player.GetInventory().CreateInInventory("CarLock");
			}
		}
	}
};
