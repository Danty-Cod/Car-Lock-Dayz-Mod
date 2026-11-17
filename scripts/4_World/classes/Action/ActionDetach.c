modded class ActionDetach
{
    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
    {
    	if (target)
    	{
    		if (target.GetObject().IsInherited(CarWheel))
    		{
    			if (CarScript.Cast(target.GetParent()).IsCarLockBlocking())
    				return false;
    		}
    	}
        return super.ActionCondition(player, target, item);
    }
}
