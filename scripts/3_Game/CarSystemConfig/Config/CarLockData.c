class CarLockData
{
	ref TIntArray CarLockPasswords;

	void CarLockData()
	{
		CarLockPasswords = new TIntArray;
	}

	void Save(string id)
	{
		string Path = CARLOCK_DATA + "_" + id + ".json";

		//then it saves the file
		JsonFileLoader<CarLockData>.JsonSaveFile(Path, this);
	}

	static ref CarLockData Load(string id)	
	{
	  CarLockData settings = new CarLockData;

	  string path = string.Format("%1_%2.json", CARLOCK_DATA, id);

	  if (FileExist(path))
			JsonFileLoader<CarLockData>.JsonLoadFile(path, settings);

		return settings;
	}
}
