// Relogio REAL do servidor em segundos (epoch). Precisamos disso porque o
// destravamento temporario do lockpick tem que sobreviver a restart - GetTime()
// zera a cada boot e serviria pra nada. Mesmo algoritmo usado no DantyzShop.
class CarLockTime
{
	// Dias desde 1970-01-01 (algoritmo de Howard Hinnant).
	static int DaysFromCivil(int y, int m, int d)
	{
		if (m <= 2)
			y = y - 1;

		int era;
		if (y >= 0)
			era = y / 400;
		else
			era = (y - 399) / 400;

		int yoe = y - era * 400;

		int mp;
		if (m > 2)
			mp = m - 3;
		else
			mp = m + 9;

		int doy = (153 * mp + 2) / 5 + d - 1;
		int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
		return era * 146097 + doe - 719468;
	}

	static int NowEpoch()
	{
		int y, mo, d, h, mi, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, mi, s);
		int days = DaysFromCivil(y, mo, d);
		return ((days * 24 + h) * 60 + mi) * 60 + s;
	}
}

class CarLockSettings
{
	ref   TStringArray Admins;

	// ========================================================================
	// LOCKPICK: destrava TEMPORARIO em vez de arrancar o CarLock.
	// ------------------------------------------------------------------------
	// Antes o lockpick removia o CarLock do carro. Como o CarLock agora tambem
	// e' a apolice do SEGURO, arrancar o cadeado apagava o seguro do dono - um
	// jeito barato de sacanear alguem. Agora o lockpick deixa o carro ABERTO por
	// um tempo e depois ele tranca sozinho, sem tirar cadeado nem seguro.
	// ========================================================================
	int   LockpickBypassSeconds;   // quanto tempo o carro fica aberto (1800 = 30 min)
	int   LockpickDamage;          // dano aplicado ao carro ao arrombar
	float LockpickBreakChance;     // chance (0.0-1.0) de QUEBRAR o lockpick (1 = sempre)
	int   LockpickDamageToPick;    // dano no lockpick quando ele NAO quebra

	// Duracao das ACOES (segundos). O config e' enviado pro cliente no connect
	// (CarLockCore.SendConfigToClient), entao os dois lados contam igual.
	int   LockpickActionSeconds;   // quanto tempo leva pra arrombar com o lockpick
	int   AttachCarLockSeconds;    // quanto tempo leva pra COLOCAR o CarLock no carro

    void CarLockSettings()
	{
		Admins = new TStringArray;
		LockpickBypassSeconds = 1800;
		LockpickDamage        = 500;
		LockpickBreakChance   = 1.0;
		LockpickDamageToPick  = 50;
		LockpickActionSeconds = 150;
		AttachCarLockSeconds  = 8;
    }

	void CarLockDefaultSettings()
    {
		Admins.Insert("76561198019185282");

		JsonFileLoader<CarLockSettings>.JsonSaveFile(CARLOCK_CONFIG, this);
    }

    static ref CarLockSettings Load()	
	{
		CarLockSettings settings = new CarLockSettings;

		if (!FileExist("$profile:SimpleModParty\\"))
		{
			MakeDirectory("$profile:SimpleModParty\\");
		}

		if (!FileExist(CARLOCK_CONFIG_ROOT_SERVER))
		{
			MakeDirectory(CARLOCK_CONFIG_ROOT_SERVER);
			if (!FileExist(CARLOCK_CONFIG_DIR_SERVER))
				MakeDirectory(CARLOCK_CONFIG_DIR_SERVER);
		}

		if (FileExist(CARLOCK_CONFIG)) 
		{
			JsonFileLoader<CarLockSettings>.JsonLoadFile(CARLOCK_CONFIG, settings);
			Print("file found !");

			// Config de servidor ANTIGO nao tem os campos do lockpick. O
			// construtor ja' deixou os defaults, aqui so' garantimos que ninguem
			// salvou zero/negativo e regravamos pra o admin ver as chaves novas.
			bool needRewrite = false;

			if (settings.LockpickBypassSeconds <= 0)
			{
				settings.LockpickBypassSeconds = 1800;
				needRewrite = true;
			}
			if (settings.LockpickDamage < 0)
			{
				settings.LockpickDamage = 500;
				needRewrite = true;
			}
			if (settings.LockpickBreakChance < 0 || settings.LockpickBreakChance > 1)
			{
				settings.LockpickBreakChance = 1.0;
				needRewrite = true;
			}
			if (settings.LockpickDamageToPick < 0)
			{
				settings.LockpickDamageToPick = 50;
				needRewrite = true;
			}
			if (settings.LockpickActionSeconds <= 0)
			{
				settings.LockpickActionSeconds = 150;
				needRewrite = true;
			}
			if (settings.AttachCarLockSeconds <= 0)
			{
				settings.AttachCarLockSeconds = 8;
				needRewrite = true;
			}

			if (needRewrite)
			{
				JsonFileLoader<CarLockSettings>.JsonSaveFile(CARLOCK_CONFIG, settings);
				Print("[CarLock] Config atualizado com as chaves do lockpick temporario.");
			}
		}
		else {
			settings.CarLockDefaultSettings();
		}

		return settings;
	}
}
