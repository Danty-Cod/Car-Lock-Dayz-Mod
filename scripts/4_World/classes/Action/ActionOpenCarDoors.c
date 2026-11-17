// IsCarLockBlocking() em vez de m_CarLock_IsLocked: o carro arrombado com
// lockpick continua com o cadeado instalado (e com o seguro do dono), mas fica
// ABERTO enquanto o prazo nao vence.
modded class ActionCarDoors: ActionInteractBase
{
    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        CarScript carScript;
        if (Class.CastTo(carScript, target.GetParent()))
        {
            if (carScript.IsCarLockBlocking())
                return false;
        }

		return super.ActionCondition(player, target, item);
	 }
}

modded class ActionCarDoorsOutside: ActionInteractBase
{
    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        CarScript carScript;
        if (Class.CastTo(carScript, target.GetParent()))
        {
            if (carScript.IsCarLockBlocking())
                return false;
        }

		return super.ActionCondition(player, target, item);
	}
}
