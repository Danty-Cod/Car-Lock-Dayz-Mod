class ActionCarLockMenu: ActionInteractBase
{
	void ActionCarLockMenu()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_HUDCursorIcon = CursorIcons.CloseHood;
	}

  	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTObject(7);//CCTMan(10);
		m_ConditionItem = new CCINone;
	}

	override string GetText()
	{
		return "Open Lock Menu";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
    {
        if ( GetGame().IsServer() )
            return true;

        CarScript ntarget = CarScript.Cast( target.GetObject() );
		if(ntarget && ntarget.m_CarLock_IsLocked && ntarget.CarLockOwner == player.GetCarLockLowId() && ntarget.CarLockPassword ==-1)
			return true;
		
        if (ntarget && ntarget.m_CarLock_IsLocked && ntarget.CarLockOwner != player.GetCarLockLowId() && !player.HasPassword(ntarget.CarLockPassword,ntarget.CarLockOwner) && ntarget.CarLockPassword !=-1)
			return true;
		
		return false;
    }

    override void OnStartClient(ActionData action_data)
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
};
