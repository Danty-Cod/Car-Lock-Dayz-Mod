class CarLockMenu extends UIScriptedMenu
{
	const int MAX_PASSWORD_LENGTH = 8;

	private ButtonWidget wBttnOK;
	private ButtonWidget wBttnCancel;
	private EditBoxWidget wPasswordInput;
	private TextWidget wTextOutPut;
	private CarScript Vehicle;

	void CarLockMenu(CarScript vehicle)
	{
		Vehicle = vehicle;
	}

	void Show(bool rep)
	{
		if (rep)
		{
			GetGame().GetInput().ChangeGameFocus(1);
			GetGame().GetUIManager().ShowUICursor(true);
			GetGame().GetMission().GetHud().Show(false);
			OnShow();
		}
		else
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor(false);
			GetGame().GetMission().GetHud().Show(true);
			OnHide();
		}
		layoutRoot.Show(rep);
	}

	override void OnShow()
	{
		super.OnShow();
		PPEffects.SetBlurMenu(0.5);
		GetGame().GetInput().ChangeGameFocus(1);
		SetFocus(layoutRoot);
		UIHandle();
	}

	override void OnHide()
	{
		super.OnHide();
		PPEffects.SetBlurMenu(0);
		GetGame().GetInput().ResetGameFocus();
		Close();
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("CarLock/gui/CarLockMenu.layout");
		wBttnOK = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BttnOK"));
		wBttnCancel = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BttnCancel"));
		wPasswordInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("PasswordInput"));
		wTextOutPut = TextWidget.Cast(layoutRoot.FindAnyWidget("TextOutPut"));
		layoutRoot.Show(false);
		return layoutRoot;
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		// Limita o campo de senha a 8 dígitos
		if (w == wPasswordInput)
		{
			string currentText = wPasswordInput.GetText();
			
			// Se passou de 8 caracteres, corta
			if (currentText.Length() > MAX_PASSWORD_LENGTH)
			{
				string limitedText = currentText.Substring(0, MAX_PASSWORD_LENGTH);
				wPasswordInput.SetText(limitedText);
			}
			
			// Remove caracteres não numéricos em tempo real
			string cleanText = "";
			for (int i = 0; i < currentText.Length() && i < MAX_PASSWORD_LENGTH; i++)
			{
				string c = currentText[i];
				if (c == "0" || c == "1" || c == "2" || c == "3" || c == "4" || c == "5" || c == "6" || c == "7" || c == "8" || c == "9")
				{
					cleanText += c;
				}
			}
			
			// Se o texto foi modificado (tinha caracteres inválidos), atualiza
			if (cleanText != currentText && cleanText.Length() <= MAX_PASSWORD_LENGTH)
			{
				wPasswordInput.SetText(cleanText);
			}
			
			return true;
		}
		
		return super.OnChange(w, x, y, finished);
	}

	void UIHandle()
	{
		if (!Vehicle.IsPasswordDefined())
			wTextOutPut.SetText("DEFINE PASSWORD (8)");
		else
			wTextOutPut.SetText("ENTER PASSWORD (8)");
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == wBttnOK)
		{
			PasswordHandler();
			return true;
		}
		else if (w == wBttnCancel)
		{
			OnHide();
			return true;
		}
		return false;
	}

	bool HasCharacterInPassword(string pwd)
	{
		for (int i = 0; i < pwd.Length(); i++)
		{
			string c = pwd[i];
			if (c != "0" && c != "1" && c != "2" && c != "3" && c != "4" && c != "5" && c != "6" && c != "7" && c != "8" && c != "9")
				return true;
		}
		return false;
	}

	bool FailSafeOk(string password)
	{
		if (password.Length() < MAX_PASSWORD_LENGTH)
		{
			wTextOutPut.SetText("Too short!");
			return false;
		}
		
		if (password.Length() > MAX_PASSWORD_LENGTH)
		{
			wTextOutPut.SetText("Too long!");
			return false;
		}
		
		if (HasCharacterInPassword(password))
		{
			wTextOutPut.SetText("Number only !");
			return false;
		}

		return true;
	}

	void PasswordHandler()
	{
		string password = wPasswordInput.GetText();
		int passwordAttempt = password.ToInt();

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		if (!FailSafeOk(password))
			return;

		// CASO 1: DEFINIR senha.
		// Carro ainda sem dono, OU o proprio dono ainda nao definiu a senha.
		if ((Vehicle.CarLockOwner == -1) || (Vehicle.CarLockOwner == player.GetCarLockLowId() && Vehicle.CarLockPassword == -1))
		{
			GetRPCManager().SendRPC("CarLock", "CarLockPasswordRequest", new Param2<CarScript, int>(Vehicle, passwordAttempt), true, NULL);
			OnHide();
			return;
		}

		// CASO 2: TENTAR senha (qualquer outro player com a senha compartilhada).
		// A validacao e a gravacao da senha sao feitas NO SERVIDOR
		// (CarLockServer.CarLockTryPasswordRequest). Nao salvamos nada no
		// cliente porque o servidor e' quem confere o HasPassword.
		GetRPCManager().SendRPC("CarLock", "CarLockTryPasswordRequest", new Param2<CarScript, int>(Vehicle, passwordAttempt), true, NULL);
		OnHide();
	}
};
