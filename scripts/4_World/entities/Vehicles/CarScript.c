modded class CarScript
{
    bool m_CarLock_IsLocked = false;
    int  CarLockOwner       = -1;
    int  CarLockPassword    = -1;
    int  m_SoundToPlay      = 0;

    // Nome do jogador que colocou o CarLock (mostrado depois do nome do carro).
    // Nao da' pra sincronizar string por net sync var, entao e' sincronizado por
    // RPC (broadcast quando muda + pedido do cliente quando ele ainda nao sabe).
    string CarLockOwnerName = "";
    // Guarda (so' no cliente) se ja' pedimos o nome ao servidor, pra nao spammar.
    protected bool m_CarLockNameRequested = false;

    // ========================================================================
    // DESTRAVAMENTO TEMPORARIO (LOCKPICK)
    // ------------------------------------------------------------------------
    // O lockpick nao arranca mais o CarLock: ele deixa o carro ABERTO por um
    // tempo (LockpickBypassSeconds) e depois o carro tranca sozinho. O cadeado e
    // a senha do dono continuam no lugar - e, como o CarLock virou tambem a
    // apolice do seguro, o seguro do dono nao e' mais perdido num arrombamento.
    //
    // LEVE DE PROPOSITO: nao existe loop verificando nada. O estado e' um epoch
    // gravado no carro; quem decide se esta aberto e' uma comparacao de inteiro.
    // Ha' apenas UM CallLater por arrombamento, so' pra avisar/sincronizar o fim.
    // ========================================================================
    int  m_CarLockBypassUntil = 0;      // epoch (seg) em que volta a trancar. 0 = sem bypass.
    bool m_CarLockBypassed    = false;  // sincronizado pro cliente (esconder/mostrar cargo)

    // ========================================================================
    // ESTADO DO SEGURO (mora AQUI, de proposito)
    // ------------------------------------------------------------------------
    // O CarLock e' a apolice, e o CarLock ja' era o unico mod que escrevia no
    // OnStoreSave do veiculo (dono, senha e nome). Guardar o seguro no MESMO
    // bloco - em vez de o DantyzShop abrir um segundo bloco - evita uma
    // fragilidade permanente: dois mods gravando em sequencia no mesmo stream
    // quebram TODOS os veiculos se a ordem de carregamento mudar um dia.
    //
    // Quem le/escreve isso de fora e' o DantyzShop, pelos metodos Ins*.
    // ========================================================================
    protected bool   m_InsActive     = false;  // seguro ativo neste carro
    protected int    m_InsPrice      = 0;      // preco pago (base do reembolso)
    protected string m_InsOwnerSteam = "";     // SteamID64 do dono do seguro

    bool   InsIsActive()    { return m_InsActive; }
    int    InsGetPrice()    { return m_InsPrice; }
    string InsGetOwner()    { return m_InsOwnerSteam; }

    //! Liga/desliga o seguro. Persistido junto do CarLock no OnStoreSave.
    void InsSet(bool active, string ownerSteamID = "", int price = 0)
    {
        m_InsActive = active;

        if (active)
        {
            m_InsOwnerSteam = ownerSteamID;
            m_InsPrice = price;
        }
        else
        {
            m_InsOwnerSteam = "";
            m_InsPrice = 0;
        }
    }

    protected EffectSound m_AlarmSound;
    protected EffectSound m_BipSound;

    void CarScript()
    {
        RegisterNetSyncVariableInt("m_SoundToPlay");
        RegisterNetSyncVariableBool("m_CarLock_IsLocked");
        RegisterNetSyncVariableInt("CarLockOwner");
        RegisterNetSyncVariableInt("CarLockPassword");
        RegisterNetSyncVariableBool("m_CarLockBypassed");

        if (GetGame().IsServer())
            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.CarLockLogInit, 10, false);
    }

    bool IsPasswordDefined()
    {
        return (CarLockOwner != -1 && CarLockPassword == -1);
    }

    void CarLockLogInit()
    {
        if (CarLockOwner != -1 && CarLockPassword != -1)
            Print(GetDisplayName() + " (ownerId:" + CarLockOwner + " - pos " + GetPosition() + " - islocked:" + m_CarLock_IsLocked + " - password:" + CarLockPassword + ") initialized");
    }

    void CarLockLogClaimed()
    {
        Print(GetDisplayName() + " (ownerId:" + CarLockOwner + " - pos " + GetPosition() + " - islocked:" + m_CarLock_IsLocked + " - password:" + CarLockPassword + ") new car claimed");
    }

    override void EEDelete(EntityAI parent)
    {
        super.EEDelete(parent);
        if (GetGame().IsServer())
        {
            if (CarLockOwner != -1 && CarLockPassword != -1)
                Print(GetDisplayName() + " (ownerId:" + CarLockOwner + " - pos " + GetPosition() + " - islocked:" + m_CarLock_IsLocked + " - password:" + CarLockPassword + ") has been deleted");
        }
    }

    void ~CarScript()
    {
        if (m_AlarmSound)
            m_AlarmSound.Stop();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();

        // Sincronizacao do NOME do dono (client-side):
        if (GetGame().IsClient())
        {
            if (CarLockOwner == -1)
            {
                // Carro sem dono: limpa o nome e libera pra pedir de novo depois.
                CarLockOwnerName = "";
                m_CarLockNameRequested = false;
            }
            else if (CarLockOwnerName == "" && !m_CarLockNameRequested)
            {
                // Carro com dono mas ainda nao sabemos o nome: pede ao servidor.
                m_CarLockNameRequested = true;
                GetRPCManager().SendRPC("CarLock", "CarLockNameRequest", new Param1<int>(0), true, NULL, this);
            }
        }

        if (m_SoundToPlay == 0)
            return;

        switch (m_SoundToPlay)
        {
            case 1:
                PlayCarLockSound();
                break;
            case 2:
                PlayCarUnlockSound();
                break;
            case 3:
                PlayCarLockAlarm();
                break;
        }
        m_SoundToPlay = 0;
    }

    void SetSoundToPlay(int id = 0)
    {
        m_SoundToPlay = id;
        SetSynchDirty();
    }

    void SetCarLockPassword(int pw)
    {
        CarLockPassword = pw;
        CarLockLogClaimed();
        SetSynchDirty();
    }

    void SetCarLockOwner(int low_uid)
    {
        CarLockOwner = low_uid;
        SetSynchDirty();
    }

    // SERVIDOR: define o nome do dono e avisa TODOS os clientes (broadcast).
    // Passar "" limpa o nome (ex: quando o CarLock e' removido).
    void SetCarLockOwnerName(string name)
    {
        CarLockOwnerName = name;

        if (GetGame().IsServer())
            GetRPCManager().SendRPC("CarLock", "CarLockNameSync", new Param1<string>(name), true, NULL, this);
    }

    // CLIENTE: recebe o nome vindo do servidor (via RPC) e guarda localmente.
    void SetCarLockOwnerNameClient(string name)
    {
        CarLockOwnerName = name;
        m_CarLockNameRequested = false;
    }

    void SetCarLock(bool rep)
    {
        m_CarLock_IsLocked = rep;

        // Se o DONO tranca o carro no meio do prazo do lockpick, o prazo acaba
        // ali: quem arrombou perde o acesso. Senao o carro continuaria aberto
        // mesmo depois de o dono ter trancado, o que nao faz sentido.
        if (rep && GetGame().IsServer() && m_CarLockBypassUntil > 0)
        {
            m_CarLockBypassUntil = 0;
            m_CarLockBypassed = false;
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(EndCarLockBypass);
        }

        SetSynchDirty();
        SynchronizeValues();
    }

    // ------------------------------------------------------------------------
    // BYPASS DO LOCKPICK
    // ------------------------------------------------------------------------

    //! O carro esta destravado temporariamente agora?
    bool IsCarLockBypassed()
    {
        if (GetGame().IsServer())
            return (m_CarLockBypassUntil > 0 && CarLockTime.NowEpoch() < m_CarLockBypassUntil);

        // No cliente nao ha' relogio confiavel do servidor: usa a flag sincronizada.
        return m_CarLockBypassed;
    }

    //! O carro esta efetivamente FECHADO pra quem nao e' dono?
    //! (trancado E sem bypass ativo)
    bool IsCarLockBlocking()
    {
        return m_CarLock_IsLocked && !IsCarLockBypassed();
    }

    //! Quantos segundos faltam pro carro trancar de novo (0 = nao esta aberto).
    int GetCarLockBypassSecondsLeft()
    {
        if (!GetGame().IsServer() || m_CarLockBypassUntil <= 0)
            return 0;

        int left = m_CarLockBypassUntil - CarLockTime.NowEpoch();
        if (left < 0)
            left = 0;

        return left;
    }

    //! Teto do prazo: 7 dias. O CallLater recebe MILISSEGUNDOS num int, e int
    //! estoura em ~2.147.483.647 - ou seja, qualquer valor acima de ~2.147.483
    //! segundos viraria numero negativo e o carro destrancaria PARA SEMPRE.
    //! 7 dias (604800s = 604.800.000ms) fica bem dentro do limite e ja' cobre
    //! configuracoes longas tipo 24h.
    static const int CARLOCK_BYPASS_MAX_SECONDS = 604800;

    //! Abre o carro por X segundos sem tocar em dono, senha ou seguro.
    void StartCarLockBypass(int seconds)
    {
        if (!GetGame().IsServer() || seconds <= 0)
            return;

        if (seconds > CARLOCK_BYPASS_MAX_SECONDS)
        {
            Print("[CarLock] LockpickBypassSeconds " + seconds + " acima do teto - limitado a " + CARLOCK_BYPASS_MAX_SECONDS + "s (7 dias).");
            seconds = CARLOCK_BYPASS_MAX_SECONDS;
        }

        m_CarLockBypassUntil = CarLockTime.NowEpoch() + seconds;
        m_CarLockBypassed = true;

        // Destranca o inventario e avisa os clientes.
        SynchronizeValues();

        // UM unico callback, so' pra fechar de volta no fim do prazo.
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(EndCarLockBypass);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(EndCarLockBypass, seconds * 1000, false);

        Print(GetDisplayName() + " (ownerId:" + CarLockOwner + ") destravado pelo lockpick por " + seconds + "s");
    }

    //! Fim do prazo: o carro volta a trancar sozinho. Dono e senha intactos.
    void EndCarLockBypass()
    {
        if (!GetGame().IsServer())
            return;

        m_CarLockBypassUntil = 0;
        m_CarLockBypassed = false;

        SynchronizeValues();

        Print(GetDisplayName() + " (ownerId:" + CarLockOwner + ") voltou a trancar (fim do lockpick)");
    }

    void StopAlarm()
    {
        if (m_AlarmSound)
            m_AlarmSound.SoundStop();
    }

    void PlayCarLockAlarm()
    {
        if (m_AlarmSound && m_AlarmSound.IsSoundPlaying())
            m_AlarmSound.SoundStop();
        
        PlaySoundSetLoop(m_AlarmSound, "cl_CarLock_CarAlarm_SoundSet", 0, 0);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StopAlarm, 300000, false);
    }

    void PlayCarLockSound()
    {
        if (m_AlarmSound && m_AlarmSound.IsSoundPlaying())
            m_AlarmSound.SoundStop();
        
        PlaySoundSet(m_BipSound, "cl_CarLock_CarLock_SoundSet", 0, 0);
    }

    void PlayCarUnlockSound()
    {
        if (m_AlarmSound && m_AlarmSound.IsSoundPlaying())
            m_AlarmSound.SoundStop();
        
        PlaySoundSet(m_BipSound, "cl_CarLock_CarUnlock_SoundSet", 0, 0);
    }

    override bool CanDisplayAttachmentSlot(string slot_name)
    {
        if (!super.CanDisplayAttachmentSlot(slot_name))
            return false;

        return !IsCarLockBlocking();
    }

    override bool CanDisplayAttachmentCategory(string category_name)
    {
        if (!super.CanDisplayAttachmentCategory(category_name))
            return false;

        return !IsCarLockBlocking();
    }

    void SynchronizeValues()
    {
        if (GetGame().IsServer())
        {
            if (GetInventory())
            {
                // Trancado E sem bypass -> inventario bloqueado.
                // Com bypass ativo o carro se comporta como destrancado, mesmo
                // com o CarLock ainda instalado.
                if (IsCarLockBlocking())
                    GetInventory().LockInventory(HIDE_INV_FROM_SCRIPT);
                else
                    GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);
            }
            SetSynchDirty();
        }
    }

    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        Param3<bool, int, int> data = new Param3<bool, int, int>(m_CarLock_IsLocked, CarLockOwner, CarLockPassword);
        ctx.Write(data);
        // Nome do dono (gravado DEPOIS do bloco antigo, pra saves antigos ainda
        // carregarem - a leitura do nome e' opcional/protegida no OnStoreLoad).
        Param1<string> nameData = new Param1<string>(CarLockOwnerName);
        ctx.Write(nameData);

        // ====================================================================
        // BLOCO UNICO do CarLock+Seguro+Lockpick.
        // --------------------------------------------------------------------
        // ATENCAO: se voce ADICIONAR ou REMOVER campos daqui, os veiculos JA
        // SALVOS vao acusar
        //   "[EntityAI::OnStoreLoad] Scripted variables corrupted upon <carro>"
        // e carregar SEM attachments e SEM cargo (carro pelado, cargo perdido).
        // Isso ja' aconteceu neste servidor. Se precisar mesmo mudar, avise os
        // jogadores para tirarem o que tem dentro dos carros antes.
        //
        // Tudo num bloco SO', e escrito por UM mod so', de proposito: dois mods
        // gravando em sequencia no mesmo stream quebram todos os veiculos caso a
        // ordem de carregamento mude.
        // ====================================================================
        Param4<bool, int, string, int> extraData = new Param4<bool, int, string, int>(m_InsActive, m_InsPrice, m_InsOwnerSteam, m_CarLockBypassUntil);
        ctx.Write(extraData);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;

        Param3<bool, int, int> data = new Param3<bool, int, int>(false, -1, -1);
        if (ctx.Read(data))
        {
            m_CarLock_IsLocked = data.param1;
            CarLockOwner = data.param2;
            CarLockPassword = data.param3;
        }
        // Leitura do nome do dono. Em saves ANTIGOS (sem esse campo) o Read falha
        // e o nome fica vazio - sem quebrar o carregamento.
        Param1<string> nameData = new Param1<string>("");
        if (ctx.Read(nameData))
            CarLockOwnerName = nameData.param1;

        // Bloco unico: seguro + prazo do lockpick.
        Param4<bool, int, string, int> extraData = new Param4<bool, int, string, int>(false, 0, "", 0);
        if (ctx.Read(extraData))
        {
            m_InsActive          = extraData.param1;
            m_InsPrice           = extraData.param2;
            m_InsOwnerSteam      = extraData.param3;
            m_CarLockBypassUntil = extraData.param4;
        }

        // Retoma o prazo do lockpick se ainda nao venceu. Sem isso o CallLater
        // morreria no restart e o carro ficaria aberto pra sempre.
        if (m_CarLockBypassUntil > 0 && GetGame().IsServer())
        {
            int leftSec = m_CarLockBypassUntil - CarLockTime.NowEpoch();

            // Nunca agendar milissegundos que estourem o int.
            if (leftSec > CARLOCK_BYPASS_MAX_SECONDS)
                leftSec = CARLOCK_BYPASS_MAX_SECONDS;

            if (leftSec > 0)
            {
                m_CarLockBypassed = true;
                GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(EndCarLockBypass, leftSec * 1000, false);
            }
            else
            {
                m_CarLockBypassUntil = 0;
                m_CarLockBypassed = false;
            }
        }

        SynchronizeValues();
        return true;
    }

    override bool IsInventoryVisible()
    {
        return !IsCarLockBlocking() && (GetGame().GetPlayer() && (!GetGame().GetPlayer().GetCommand_Vehicle() || GetGame().GetPlayer().GetCommand_Vehicle().GetTransport() == this));
    }

    // Mostra o nome do dono depois do nome do carro (no cursor de acao, etc.)
    // quando o carro esta trancado com CarLock. Some quando o CarLock e' removido.
    override string GetDisplayName()
    {
        string baseName = super.GetDisplayName();
        // Mostra enquanto o CarLock estiver COLOCADO (dono definido), trancado ou
        // nao. Some quando o CarLock e' removido (dono volta a -1).
        if (CarLockOwner != -1 && CarLockOwnerName != "")
        {
            // Avisa que o carro esta aberto por causa do lockpick, pra o dono
            // perceber que foi arrombado (e o ladrao saber que tem prazo).
            if (IsCarLockBypassed())
                return baseName + " - Dono: " + CarLockOwnerName + " [DESTRAVADO]";

            return baseName + " - Dono: " + CarLockOwnerName;
        }
        return baseName;
    }
};
