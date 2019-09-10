#include <Windows.h>
#include <WinDNS.h>
#include <gtest/gtest.h>

#pragma comment(lib, "dnsapi.lib")

// https://stackoverflow.com/questions/122208/get-the-ip-address-of-local-computer
TEST(dns, get_host)
{
    WSADATA wsa_data;
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsa_data);

    char host_name[255];
    gethostname(host_name, 255);

    PDNS_RECORD pDnsRecord;
    DNS_STATUS statsus = DnsQuery(host_name, DNS_TYPE_A, DNS_QUERY_STANDARD, NULL, &pDnsRecord, NULL);

    IN_ADDR ipaddr;
    ipaddr.S_un.S_addr = (pDnsRecord->Data.A.IpAddress);
    printf("The IP address of the host %s is %s \n", host_name, inet_ntoa(ipaddr));

    DnsRecordListFree(pDnsRecord, DnsFreeRecordList);
    WSACleanup();
}
