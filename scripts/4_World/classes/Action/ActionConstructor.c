modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        actions.Insert(ActionAttachCarLock);
        actions.Insert(ActionDetachCarLock);  // Registrado mas não usado (removido do Pliers)
        actions.Insert(ActionDetachCarLockPick);  // Única forma de remover CarLock
        actions.Insert(ActionCarUnLock);
        actions.Insert(ActionCarLock);
        actions.Insert(ActionCarLockMenu);
    }
}
