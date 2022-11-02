void ShowError(long lerr)
{
    LPVOID lpMsgBuf;
    if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        lerr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL))
    {
        return;
    }
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION);
    LocalFree(lpMsgBuf);
}
void CieproxyDlg::OnBnClickedOk()
{//set proxy server
    UpdateData();
    GetDlgItemText(IDC_EDIT1, m_sIEProxy);
    HKEY hk;
    LONG lret = RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        NULL, KEY_WRITE | KEY_SET_VALUE, &hk);
    if (lret == ERROR_SUCCESS && NULL != hk)
    {
        TCHAR* pbuf = m_sIEProxy.GetBuffer(1);
        lret = RegSetValueEx(hk, "ProxyServer", NULL, REG_SZ, pbuf, m_sIEProxy.GetLength());
        DWORD dwenable = 1;
        lret = RegSetValueEx(hk, "ProxyEnable", NULL, REG_DWORD,
            (LPBYTE)&dwenable, sizeof(dwenable));
        RegCloseKey(hk);
    }
    const TCHAR* keyname3 = _T(
        "software\\Microsoft\\windows\\currentversion\\Internet Settings\\Connections");
    lret = RegOpenKeyEx(HKEY_CURRENT_USER, keyname3, NULL,
        KEY_READ | KEY_WRITE | KEY_SET_VALUE, &hk);
    if (lret == ERROR_SUCCESS && NULL != hk)
    {
        DWORD dwtype;
        char pbuf[256];
        DWORD dwlen = sizeof(pbuf);
        const char* valname = "Connection to adsl3";
        lret = RegQueryValueEx(hk, valname, NULL, &dwtype, pbuf, &dwlen);
        if (lret != ERROR_SUCCESS)
        {
            ShowError(lret);
        }
        pbuf[8] = 3;//enable proxy
        pbuf[4] = pbuf[4] + 1;
        const char* p = m_sIEProxy.GetBuffer(1);
        memcpy(pbuf + 16, p, m_sIEProxy.GetLength());
        char c = 0;
        for (int i = m_sIEProxy.GetLength(); i < 20; i++)
            pbuf[16 + i] = c;
        m_sIEProxy.ReleaseBuffer();
        lret = RegSetValueEx(hk, valname, NULL, REG_BINARY, pbuf, dwlen);
        RegCloseKey(hk);
    }
    DWORD dwret;
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, NULL,
        SMTO_NORMAL, 1000, &dwret);
}

void CieproxyDlg::OnBnClickedDisableProxy()
{
    UpdateData();
    GetDlgItemText(IDC_EDIT1, m_sIEProxy);
    HKEY hk;
    LONG lret = RegOpenKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
        NULL, KEY_WRITE | KEY_SET_VALUE, &hk);
    if (lret == ERROR_SUCCESS && NULL != hk)
    {
        DWORD dwenable = 0;
        lret = RegSetValueEx(hk, "ProxyEnable", NULL, REG_DWORD,
            (LPBYTE)&dwenable, sizeof(dwenable));
        RegCloseKey(hk);
    }
    const TCHAR* keyname3 = _T(
        "software\\Microsoft\\windows\\currentversion\\Internet Settings\\Connections");
    lret = RegOpenKeyEx(HKEY_CURRENT_USER, keyname3,
        NULL, KEY_READ | KEY_WRITE | KEY_SET_VALUE, &hk);
    if (lret == ERROR_SUCCESS && NULL != hk)
    {
        DWORD dwtype;
        char pbuf[256];
        DWORD dwlen = sizeof(pbuf);
        const char* valname = "Connection to adsl3";
        lret = RegQueryValueEx(hk, valname, NULL, &dwtype, pbuf, &dwlen);
        if (lret != ERROR_SUCCESS)
        {
            ShowError(lret);
        }
        pbuf[8] = 1;//enable proxy
        pbuf[4] = pbuf[4] + 1;
        lret = RegSetValueEx(hk, valname, NULL, REG_BINARY, pbuf, dwlen);
        RegCloseKey(hk);
    }
    DWORD dwret;
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, NULL, SMTO_NORMAL,
        1000, &dwret);
}

BOOL SetConnectionOptions(LPCTSTR conn_name, LPCTSTR proxy_full_addr)
{
    //conn_name: active connection name. 
    //proxy_full_addr : eg "210.78.22.87:8000"
    INTERNET_PER_CONN_OPTION_LIST list;
    BOOL    bReturn;
    DWORD   dwBufSize = sizeof(list);
    // Fill out list struct.
    list.dwSize = sizeof(list);
    // NULL == LAN, otherwise connectoid name.
    list.pszConnection = conn_name;
    // Set three options.
    list.dwOptionCount = 3;
    list.pOptions = new INTERNET_PER_CONN_OPTION[3];
    // Make sure the memory was allocated.
    if (NULL == list.pOptions)
    {
        // Return FALSE if the memory wasn't allocated.
        OutputDebugString("failed to allocat memory in SetConnectionOptions()");
        return FALSE;
    }
    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT |
        PROXY_TYPE_PROXY;

    // Set proxy name.
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[1].Value.pszValue = proxy_full_addr;//"http://proxy:80";

    // Set proxy override.
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[2].Value.pszValue = "local";

    // Set the options on the connection.
    bReturn = InternetSetOption(NULL,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

    // Free the allocated memory.
    delete[] list.pOptions;
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
    return bReturn;
}
BOOL DisableConnectionProxy(LPCTSTR conn_name)
{
    //conn_name: active connection name. 
    INTERNET_PER_CONN_OPTION_LIST list;
    BOOL    bReturn;
    DWORD   dwBufSize = sizeof(list);
    // Fill out list struct.
    list.dwSize = sizeof(list);
    // NULL == LAN, otherwise connectoid name.
    list.pszConnection = conn_name;
    // Set three options.
    list.dwOptionCount = 1;
    list.pOptions = new INTERNET_PER_CONN_OPTION[list.dwOptionCount];
    // Make sure the memory was allocated.
    if (NULL == list.pOptions)
    {
        // Return FALSE if the memory wasn't allocated.
        OutputDebugString("failed to allocat memory in DisableConnectionProxy()");
        return FALSE;
    }
    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;
    // Set the options on the connection.
    bReturn = InternetSetOption(NULL,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);
    // Free the allocated memory.
    delete[] list.pOptions;
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
    return bReturn;
}