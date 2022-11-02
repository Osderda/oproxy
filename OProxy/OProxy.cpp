// OProxy.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.
//

#include <iostream>
#include <ostream>
#include <Windows.h>
#include <wininet.h>
int main()
{
    std::cout << "Hello World!\n";
    auto proxyName= L"http=183.220.6.198;https=9091";
   
    INTERNET_PER_CONN_OPTION_LIST OptionList;
    INTERNET_PER_CONN_OPTION Option[3];
    unsigned long listSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
    Option[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    Option[1].dwOption = INTERNET_PER_CONN_FLAGS;
    Option[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    OptionList.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
    OptionList.pszConnection = NULL;
    OptionList.dwOptionCount = 3;
    OptionList.dwOptionError = 0;

    DWORD proxyType = PROXY_TYPE_DIRECT; // this proxy type disables any proxy server

    if (proxyName) {
        if (proxyName[0]) {
            proxyType = PROXY_TYPE_PROXY; // a name has been passed, so choose the correct proxy type for enabling the proxy server
        }
    }

    Option[0].Value.pszValue = (LPWSTR)proxyName;
    Option[1].Value.dwValue = proxyType;
 
        Option[2].Value.pszValue = (LPWSTR)L"";
   
    OptionList.pOptions = Option;

    InternetSetOption(0, INTERNET_OPTION_REFRESH, NULL, NULL);
}
