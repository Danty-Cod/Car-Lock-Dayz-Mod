modded class Pliers extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		// ActionDetachCarLock removida - apenas CarLockpick pode remover CarLock
	}
};

class CarLockPick extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionDetachCarLockPick);
	}
};

class CarLock extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionAttachCarLock);
	}
};
