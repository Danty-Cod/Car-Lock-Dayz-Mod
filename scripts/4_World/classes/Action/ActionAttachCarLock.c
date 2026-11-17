class ActionAttachCarLockCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		// Tempo pra colocar o CarLock vem do JSON (AttachCarLockSeconds).
		float attachTime = 8;

		CarLockSettings attachSettings = GetCarLockSettings();
		if (attachSettings && attachSettings.AttachCarLockSeconds > 0)
			attachTime = attachSettings.AttachCarLockSeconds;

		m_ActionData.m_ActionComponent = new CAContinuousTime(attachTime);
	}
};

class ActionAttachCarLock: ActionContinuousBase
{
	void ActionAttachCarLock()
	{
		m_CallbackClass = ActionAttachCarLockCB;
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
		return "Attach CarLock";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if ( GetGame().IsServer() )
				return true;

		CarScript ntarget = CarScript.Cast( target.GetObject() );
		if (ntarget && ntarget.CarLockOwner==-1)
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
		if (action_data.m_MainItem)
		{
			action_data.m_MainItem.Delete();
		}
	}

	override void OnFinishProgressClient( ActionData action_data )
	{
    	if ( action_data.m_MainItem && action_data.m_MainItem.GetHierarchyRootPlayer() == action_data.m_Player )
		{
			CarScript car = CarScript.Cast(action_data.m_Target.GetObject());
			if(car)
			{
				if ( g_Game.GetUIManager().GetMenu() == NULL )
				{
					GetCarLockCore().iCarLockMenu = new CarLockMenu(car);
					GetCarLockCore().iCarLockMenu.Init();
					GetGame().GetUIManager().ShowScriptedMenu( GetCarLockCore().iCarLockMenu, NULL );
				}
			}
		}
   }
};
