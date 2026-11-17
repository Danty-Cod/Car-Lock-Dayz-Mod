// ============================================
// CarLock - Sistema de Validação de Licença FORTE
// Valida licença antes de iniciar o mod
// Sistema de segurança robusto com múltiplas camadas
// ============================================

class CarLockLicenseValidator
{
    protected static ref CarLockLicenseValidator s_Instance;
    protected static string LICENSE_PATH = "$profile:SimpleModParty/carlock.key";
    
    // CHAVE SECRETA - NUNCA COMPARTILHAR!
    // Esta chave é usada para validar a integridade da licença
    protected static string SECRET_KEY = "CarLock_2026_ULTRA_SECRET_KEY_NEVER_SHARE_OR_MODIFY";
    
    protected bool m_IsValid = false;
    protected bool m_IsChecked = false;
    protected string m_ServerID = "";
    protected int m_ExpiryTimestamp = 0;
    
    void CarLockLicenseValidator()
    {
        Print("[CarLock] Inicializando validador de licença FORTE...");
    }
    
    static CarLockLicenseValidator GetInstance()
    {
        if (!s_Instance)
        {
            s_Instance = new CarLockLicenseValidator();
        }
        return s_Instance;
    }
    
    // ============================================
    // VALIDAÇÃO FORTE DE LICENÇA
    // Sistema de múltiplas camadas de segurança
    // ============================================
    bool ValidateLicense()
    {
        if (m_IsChecked)
            return m_IsValid;
        
        m_IsChecked = true;
        
        Print("[CarLock] VALIDADOR FORTE DE LICENÇA");
        Print("[CarLock] Verificando licença do servidor...");
        
        // CAMADA 1: Verificar se arquivo existe
        if (!FileExist(LICENSE_PATH))
        {
            Print("[CarLock] ❌ FALHA CAMADA 1: Arquivo não encontrado!");
            Print("[CarLock] Caminho: " + LICENSE_PATH);
            Print("[CarLock] ❌ MOD BLOQUEADO - LICENÇA AUSENTE");
            m_IsValid = false;
            return false;
        }
        
        Print("[CarLock] ✅ CAMADA 1: Arquivo encontrado");
        
        // CAMADA 2: Ler e validar conteúdo
        string licenseContent = ReadLicenseFile();
        if (licenseContent == "" || licenseContent.Length() < 50)
        {
            Print("[CarLock] ❌ FALHA CAMADA 2: Conteúdo inválido!");
            Print("[CarLock] Tamanho: " + licenseContent.Length().ToString() + " (mínimo: 50)");
            Print("[CarLock] ❌ MOD BLOQUEADO - LICENÇA CORROMPIDA");
            m_IsValid = false;
            return false;
        }
        
        Print("[CarLock] ✅ CAMADA 2: Conteúdo válido (" + licenseContent.Length().ToString() + " chars)");
        
        // CAMADA 3: Decodificar Base64
        string decoded = DecodeBase64Simple(licenseContent);
        if (decoded == "" || decoded.Length() < 20)
        {
            Print("[CarLock] ❌ FALHA CAMADA 3: Decodificação falhou!");
            Print("[CarLock] ❌ MOD BLOQUEADO - LICENÇA INVÁLIDA");
            m_IsValid = false;
            return false;
        }
        
        Print("[CarLock] ✅ CAMADA 3: Decodificação OK");
        
        // CAMADA 4: Separar e validar partes
        array<string> parts = new array<string>;
        decoded.Split("|", parts);
        
        if (parts.Count() != 4)
        {
            Print("[CarLock] ❌ FALHA CAMADA 4: Formato incorreto!");
            Print("[CarLock] Partes encontradas: " + parts.Count().ToString() + " (esperado: 4)");
            Print("[CarLock] ❌ MOD BLOQUEADO - FORMATO INVÁLIDO");
            m_IsValid = false;
            return false;
        }
        
        string serverID = parts[0];
        string expiryStr = parts[1];
        string salt = parts[2];
        string providedHash = parts[3];
        
        Print("[CarLock] ✅ CAMADA 4: Formato correto");
        Print("[CarLock] ID da Licença: " + serverID);
        
        // CAMADA 5: Validar hash de integridade
        string dataToHash = serverID + "|" + expiryStr + "|" + salt + "|" + SECRET_KEY;
        string calculatedHash = CalculateStrongHash(dataToHash);
        
        if (calculatedHash != providedHash)
        {
            Print("[CarLock] ❌ FALHA CAMADA 5: Hash não corresponde!");
            Print("[CarLock] Licença adulterada ou falsificada!");
            Print("[CarLock] ❌ MOD BLOQUEADO - LICENÇA FALSIFICADA");
            m_IsValid = false;
            return false;
        }
        
        Print("[CarLock] ✅ CAMADA 5: Integridade verificada");
        
        // CAMADA 6: Verificar expiração
        int expiryTimestamp = expiryStr.ToInt();
        int currentTimestamp = GetCurrentTimestamp();
        
        if (currentTimestamp > expiryTimestamp)
        {
            int daysExpired = (currentTimestamp - expiryTimestamp) / 86400;
            Print("[CarLock] ❌ FALHA CAMADA 6: Licença expirada!");
            Print("[CarLock] Expirou há " + daysExpired.ToString() + " dias");
            Print("[CarLock] ❌ MOD BLOQUEADO - LICENÇA EXPIRADA");
            m_IsValid = false;
            return false;
        }
        
        int daysRemaining = (expiryTimestamp - currentTimestamp) / 86400;
        Print("[CarLock] ✅ CAMADA 6: Validade OK (" + daysRemaining.ToString() + " dias)");
        
        // TODAS AS CAMADAS PASSARAM!
        m_IsValid = true;
        m_ServerID = serverID;
        m_ExpiryTimestamp = expiryTimestamp;
        
        Print("[CarLock] ✅✅✅ LICENÇA VÁLIDA! ✅✅✅");
        Print("[CarLock] ID da Licença: " + serverID);
        Print("[CarLock] Dias restantes: " + daysRemaining.ToString());
        Print("[CarLock] ✅ MOD INICIADO COM SUCESSO");
        
        // Aviso se estiver perto de expirar
        if (daysRemaining <= 7)
        {
            Print("[CarLock] ⚠️ ATENÇÃO: Licença expira em " + daysRemaining.ToString() + " dias!");
        }
        
        return true;
    }
    
    bool IsLicenseValid()
    {
        return m_IsValid;
    }
    
    string GetServerID()
    {
        return m_ServerID;
    }
    
    int GetDaysRemaining()
    {
        if (!m_IsValid)
            return 0;
        
        int currentTimestamp = GetCurrentTimestamp();
        int daysRemaining = (m_ExpiryTimestamp - currentTimestamp) / 86400;
        return daysRemaining;
    }
    
    // Lê o arquivo de licença
    protected string ReadLicenseFile()
    {
        FileHandle file = OpenFile(LICENSE_PATH, FileMode.READ);
        if (!file)
        {
            Print("[CarLock] ERRO: Não foi possível abrir o arquivo!");
            return "";
        }
        
        string content = "";
        string line = "";
        
        while (FGets(file, line) > 0)
        {
            content += line;
        }
        
        CloseFile(file);
        
        // Remover espaços e quebras de linha
        content.Trim();
        content.Replace("\n", "");
        content.Replace("\r", "");
        content.Replace(" ", "");
        content.Replace("\t", "");
        
        return content;
    }
    
    // Decodifica Base64 SIMPLIFICADO mas FUNCIONAL
    protected string DecodeBase64Simple(string encoded)
    {
        string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        string decoded = "";
        
        int len = encoded.Length();
        int i = 0;
        
        while (i < len)
        {
            // Ler 4 caracteres Base64
            if (i >= len) break;
            int b1 = base64Chars.IndexOf(encoded.Get(i));
            i++;
            
            if (i >= len) break;
            int b2 = base64Chars.IndexOf(encoded.Get(i));
            i++;
            
            int b3 = -1;
            int b4 = -1;
            
            if (i < len && encoded.Get(i) != "=")
            {
                b3 = base64Chars.IndexOf(encoded.Get(i));
            }
            i++;
            
            if (i < len && encoded.Get(i) != "=")
            {
                b4 = base64Chars.IndexOf(encoded.Get(i));
            }
            i++;
            
            if (b1 == -1 || b2 == -1)
                break;
            
            // Decodificar
            int c1 = (b1 << 2) | (b2 >> 4);
            decoded += c1.AsciiToString();
            
            if (b3 != -1)
            {
                int c2 = ((b2 & 15) << 4) | (b3 >> 2);
                decoded += c2.AsciiToString();
            }
            
            if (b4 != -1)
            {
                int c3 = ((b3 & 3) << 6) | b4;
                decoded += c3.AsciiToString();
            }
        }
        
        return decoded;
    }
    
    // Calcula hash FORTE usando múltiplos passes
    protected string CalculateStrongHash(string input)
    {
        // HASH FORTE com múltiplas iterações
        string hash = input;
        
        // Pass 1: Hash básico
        int h1 = 5381;
        for (int i = 0; i < hash.Length(); i++)
        {
            int c1 = hash.Get(i).ToAscii();
            h1 = ((h1 << 5) + h1) + c1;
        }
        
        // Pass 2: Hash reverso
        int h2 = 7919;
        for (int j = hash.Length() - 1; j >= 0; j--)
        {
            int c2 = hash.Get(j).ToAscii();
            h2 = ((h2 << 3) + h2) + c2;
        }
        
        // Pass 3: Hash com salt
        int h3 = 104729;
        for (int k = 0; k < hash.Length(); k++)
        {
            int c3 = hash.Get(k).ToAscii();
            h3 = ((h3 << 4) - h3) + c3;
        }
        
        // Pass 4: Combinar todos os hashes
        int h4 = 524287;
        h4 = h4 ^ h1;
        h4 = h4 ^ h2;
        h4 = h4 ^ h3;
        
        // Converter para positivo se negativo
        if (h1 < 0) h1 = -h1;
        if (h2 < 0) h2 = -h2;
        if (h3 < 0) h3 = -h3;
        if (h4 < 0) h4 = -h4;
        
        // Criar hash final combinando todos
        string finalHash = h1.ToString() + "-" + h2.ToString() + "-" + h3.ToString() + "-" + h4.ToString();
        
        return finalHash;
    }
    
    // Obtém timestamp Unix atual
    protected int GetCurrentTimestamp()
    {
        int year, month, day, hour, minute, second;
        GetYearMonthDay(year, month, day);
        GetHourMinuteSecond(hour, minute, second);
        
        // Calcular dias desde 1970
        int totalDays = 0;
        
        int y = 1970;
        while (y < year)
        {
            if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
                totalDays += 366;
            else
                totalDays += 365;
            y++;
        }
        
        int m = 1;
        while (m < month)
        {
            if (m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
                totalDays += 31;
            else if (m == 4 || m == 6 || m == 9 || m == 11)
                totalDays += 30;
            else if (m == 2)
            {
                if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
                    totalDays += 29;
                else
                    totalDays += 28;
            }
            m++;
        }
        
        totalDays += day - 1;
        
        int totalSeconds = (totalDays * 86400) + (hour * 3600) + (minute * 60) + second;
        
        return totalSeconds;
    }
}

// Função global para obter o validador
static ref CarLockLicenseValidator g_CarLockLicenseValidator;
CarLockLicenseValidator GetCarLockLicenseValidator()
{
    Print("[CarLock] GetCarLockLicenseValidator() chamado");
    Print("[CarLock] IsServer: " + GetGame().IsServer().ToString());
    Print("[CarLock] g_CarLockLicenseValidator existe: " + (g_CarLockLicenseValidator != null).ToString());
    
    if (!g_CarLockLicenseValidator && GetGame().IsServer())
    {
        Print("[CarLock] Criando nova instância do validador...");
        g_CarLockLicenseValidator = CarLockLicenseValidator.GetInstance();
        Print("[CarLock] Instância criada: " + (g_CarLockLicenseValidator != null).ToString());
    }
    
    return g_CarLockLicenseValidator;
}
