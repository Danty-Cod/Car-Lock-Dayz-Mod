const string CARLOCK_CONFIG_ROOT_CLIENT = "$saves:SimpleModParty\\CarLock\\";
const string CARLOCK_CONFIG_DIR_CLIENT = CARLOCK_CONFIG_ROOT_CLIENT + "Database\\";

const string CARLOCK_CONFIG_ROOT_SERVER = "$profile:SimpleModParty\\CarLock\\";
const string CARLOCK_CONFIG = CARLOCK_CONFIG_ROOT_SERVER + "CarLockSettings.json";
const string CARLOCK_CONFIG_DIR_SERVER = CARLOCK_CONFIG_ROOT_SERVER + "Database\\";

// Banco de senhas dos players. Fica no $profile (lado SERVIDOR), porque a
// verificacao de senha (HasPassword) e' feita no servidor, nas actions de
// trancar/destrancar. Antes apontava para $saves (lado CLIENTE), entao a
// senha que um amigo digitava era salva na maquina dele e o servidor nunca
// via -> dava "sem permissao" mesmo com a senha certa.
const string CARLOCK_DATA = CARLOCK_CONFIG_DIR_SERVER + "CarLock";
