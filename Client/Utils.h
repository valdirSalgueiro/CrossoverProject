#pragma once

class Utils
{
public:
	static std::wstring getComputerGUID() {
		HKEY hKey;
		std::wstring clientGUID;
		LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Cryptography", 0, KEY_READ, &hKey);
		bool bExistsAndSuccess(lRes == ERROR_SUCCESS);
		bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
		getStringRegKey(hKey, L"MachineGuid", clientGUID, L"bad");
		return clientGUID;
	}

private:
	

	static LONG getStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
	{
		strValue = strDefaultValue;
		WCHAR szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError;
		DWORD dataType = REG_SZ;
		nError = RegQueryValueExW(hKey, strValueName.c_str(), NULL, &dataType, (LPBYTE)szBuffer, &dwBufferSize);
		if (ERROR_SUCCESS == nError)
		{
			strValue = szBuffer;
		}
		return nError;
	}
};
