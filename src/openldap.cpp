#include <gtest/gtest.h>
#include <ldap.h>
#include <string>

#ifdef __APPLE__
#ifdef TARGET_OS_MAC
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

static int search(LDAP *ld, const std::string &searchBase, const std::string &filter)
{
    LDAPMessage *searchResult;
    struct timeval timeOut = {10, 0}; /* 10 second connection/search timeout */

    int rc = ldap_search_ext_s(ld,                 /* LDAP session handle */
                               searchBase.c_str(), /* container to search */
                               LDAP_SCOPE_SUBTREE, /* */
                               filter.c_str(),     /* filter */
                               NULL,               /* return all attributes */
                               0,                  /* return attributes and values */
                               NULL,               /* server controls */
                               NULL,               /* client controls */
                               &timeOut,           /* search timeout */
                               LDAP_NO_LIMIT,      /* no size limit */
                               &searchResult);     /* returned results */

    if (rc != LDAP_SUCCESS)
    {
        ldap_msgfree(searchResult);
        ldap_unbind_s(ld);
        return rc;
    }

    const char *sortAttribute = "sn";
    char *dn;
    ldap_sort_entries(ld, &searchResult, sortAttribute, strcmp);

    /* Go through the search results by checking entries */
    for (LDAPMessage *entry = ldap_first_entry(ld, searchResult); entry != NULL;
         entry = ldap_next_entry(ld, entry))
    {
        if ((dn = ldap_get_dn(ld, entry)) != NULL)
        {
            printf("\tdn: %s\n", dn);
            ldap_memfree(dn);
        }
    }

    int entryCount = ldap_count_entries(ld, searchResult);
    printf("\n  Search completed successfully.\n  Entries  returned: %d\n", entryCount);
    ldap_msgfree(searchResult);
    return LDAP_SUCCESS;
}

TEST(ldap, simple)
{
    // http://techsmruti.com/online-ldap-test-server/

    int version = LDAP_VERSION3;
    struct timeval timeOut = {10, 0}; /* 10 second connection timeout */
    ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    LDAP *ld;
    const char *ldap_host = "ldap://ldap.forumsys.com";
    ldap_initialize(&ld, ldap_host);

    const char *binddn = "cn=read-only-admin,dc=example,dc=com";
    struct berval passwd;
    char password[] = "password";
    passwd.bv_val = password;
    passwd.bv_len = strlen(passwd.bv_val);

    int rc = ldap_sasl_bind_s(ld, binddn, LDAP_SASL_SIMPLE, &passwd, NULL, NULL, NULL);
    if (rc != LDAP_SUCCESS)
    {
        printf("ldap_sasl_bind_s: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    printf("bind successful\n");

    std::string searchBase = "dc=example,dc=com";
    std::string filter = "(&(objectClass=person)(cn=*E*))";
    rc = search(ld, searchBase, filter);
    if (rc != LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ld, NULL, NULL);
        FAIL() << "search failed: " << ldap_err2string(rc);
        return;
    }

    ldap_unbind_ext_s(ld, NULL, NULL);
}
