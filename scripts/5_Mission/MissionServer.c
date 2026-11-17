modded class MissionServer extends MissionBase
{
	protected bool m_IsInitialized = false;
	
	void MissionServer()
	{
		Print("[CarLock] INICIANDO CARLOCK");
		
		// ============================================
		// VALIDAR LICENÇA ANTES DE INICIAR
		// ============================================
		Print("[CarLock] Obtendo validador de licença...");
		CarLockLicenseValidator validator = GetCarLockLicenseValidator();
		
		if (!validator)
		{
			Print("[CarLock] ❌ ERRO CRÍTICO: Validador não foi criado!");
			Print("[CarLock] ❌ DESLIGANDO SERVIDOR!");
			m_IsInitialized = false;
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ShutdownServer, 3000, false);
			return;
		}
		
		Print("[CarLock] Validador obtido, iniciando validação...");
		bool licenseValid = validator.ValidateLicense();
		Print("[CarLock] Resultado da validação: " + licenseValid.ToString());
		
		if (!licenseValid)
		{
			Print("[CarLock] ❌❌❌ LICENÇA INVÁLIDA OU EXPIRADA! ❌❌❌");
			Print("[CarLock] SERVIDOR SERÁ DESLIGADO EM 3 SEGUNDOS!");
			Print("[CarLock] Entre em contato para obter/renovar licença:");
			Print("[CarLock] - Coloque o arquivo carlock.key em:");
			Print("[CarLock]   $profile:SimpleModParty/carlock.key");
			Print("[CarLock] ⏰ DESLIGANDO SERVIDOR...");
			m_IsInitialized = false;
			
			// DESLIGAR O SERVIDOR!
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.ShutdownServer, 3000, false);
			return;
		}
		
		// Licença válida - continuar inicialização
		Print("[CarLock] ✅ Licença válida! Continuando inicialização...");
		m_IsInitialized = true;
		
		m_CarLockCore = NULL;
		GetCarLockCore();
		
		Print("[CarLock] ✅ MOD INICIADO COM SUCESSO!");
	}
	
	// Função para desligar o servidor
	void ShutdownServer()
	{
		Print("[CarLock] ❌ DESLIGANDO SERVIDOR - LICENÇA INVÁLIDA");
		
		// Desligar o servidor
		GetGame().RequestExit(IDC_MAIN_QUIT);
	}
	
	bool IsInitialized()
	{
		return m_IsInitialized;
	}

	void ~MissionServer() {
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		if (!m_IsInitialized) return; // Licença inválida - não processar
		
		super.InvokeOnConnect(player,identity);
		player.SetLowSteamUID();
		GetCarLockCore().SendConfigToClient(player);
	}
};
