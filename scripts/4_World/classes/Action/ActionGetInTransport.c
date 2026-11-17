modded class ActionGetInTransport: ActionBase
{
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        CarScript carScript;
        if (Class.CastTo(carScript, target.GetObject()))
        {
            // Com o prazo do lockpick valendo, da' pra entrar no carro mesmo
            // com o CarLock instalado.
            if (carScript.IsCarLockBlocking())
                return false;
        }

        return super.ActionCondition(player, target, item);
	}
};
