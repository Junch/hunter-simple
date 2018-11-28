#include <future>
#include <gtest/gtest.h>
#include <ldap.h>
#include <string>
#include <thread>
#include <yaml-cpp/yaml.h>

// http://techsmruti.com/online-ldap-test-server/
// https://github.com/inspircd/inspircd/blob/master/src/modules/extra/m_ldap.cpp
// https://docs.oracle.com/cd/E19957-01/817-6707/index.html

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define HOSTNAME "ldap://ldap.forumsys.com"
#define PORTNUMBER LDAP_PORT
#define BASEDN "dc=example,dc=com"
#define SCOPE LDAP_SCOPE_SUBTREE
#define FILTER "(&(objectClass=person)(cn=*E*))"
#define BINDDN "cn=read-only-admin,dc=example,dc=com"
#define PASSWORD "password"

static int search_s(LDAP *ld, const char *searchBase, int scope, const char *filter, char **attrs)
{
    LDAPMessage *searchResult;
    struct timeval timeOut = {10, 0}; /* 10 second connection/search timeout */

    auto start = std::chrono::high_resolution_clock::now();

    int rc = ldap_search_ext_s(ld,             /* LDAP session handle */
                               searchBase,     /* container to search */
                               scope,          /* scope to search */
                               filter,         /* filter */
                               attrs,          /* attrs, return all attributes */
                               0,              /* attrsonly, return attributes and values */
                               NULL,           /* server controls */
                               NULL,           /* client controls */
                               &timeOut,       /* search timeout */
                               LDAP_NO_LIMIT,  /* sizelimit, no size limit */
                               &searchResult); /* returned results */

    if (rc != LDAP_SUCCESS && rc != LDAP_SIZELIMIT_EXCEEDED)
    {
        ldap_msgfree(searchResult);
        ldap_unbind_s(ld);
        return rc;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << "seconds = " << delta.count() << '\n';

    const char *sortAttribute = "sn";
    char *dn;
    ldap_sort_entries(ld, &searchResult, sortAttribute, strcmp);

    /* Go through the search results by checking entries */
    for (LDAPMessage *entry = ldap_first_entry(ld, searchResult); entry != NULL; entry = ldap_next_entry(ld, entry))
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

TEST(ldap, synchronous)
{
    int version = LDAP_VERSION3;
    struct timeval timeOut = {10, 0}; /* 10 second connection timeout */
    ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    LDAP *ld;
    const char *ldap_host = HOSTNAME;
    ldap_initialize(&ld, ldap_host);

    const char *binddn = BINDDN;
    struct berval passwd;
    char password[] = PASSWORD;
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
    rc = search_s(ld, BASEDN, SCOPE, FILTER, NULL);
    if (rc != LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ld, NULL, NULL);
        FAIL() << "search failed: " << ldap_err2string(rc);
        return;
    }

    ldap_unbind_ext_s(ld, NULL, NULL);
}

static int result(LDAP *ld, int msgid)
{
    /* Poll the server for the results of the search operation.
        Passing LDAP_MSG_ONE indicates that you want to receive
        the entries one at a time, as they come in. If the next
        entry that you retrieve is NULL, there are no more entries. */
    bool finished = false;
    struct timeval zerotime = {0, 0};
    int num_entries = 0;
    int num_refs = 0;
    while (!finished)
    {
        LDAPMessage *res = NULL;
        BerElement *ber = NULL;

        int rc = ldap_result(ld, msgid, LDAP_MSG_ONE, &zerotime, &res);
        /* The server can return three types of results back to the client,
           and the return value of ldap_result() indicates the result type:
           LDAP_RES_SEARCH_ENTRY identifies an entry found by the search,
           LDAP_RES_SEARCH_REFERENCE identifies a search reference returned
           by the server, and LDAP_RES_SEARCH_RESULT is the last result
           sent from the server to the client after the operation completes.
           You need to check for each of these types of results. */

        switch (rc)
        {
        case -1:
            /* An error occurred. */
            fprintf(stderr, "ldap_result: %s\n", ldap_err2string(rc));
            ldap_unbind(ld);
            return 1;

        case 0:
            /* The timeout period specified by zerotime was exceeded.
               This means that the server has still not yet sent the
               results of the search operation back to your client.
               Break out of this switch statement, and continue calling
               ldap_result() to poll for results. */
            break;

        case LDAP_RES_SEARCH_ENTRY:
        {
            /* The server sent one of the entries found by the search operation. Print the DN,
             * attributes, and values of the entry. */
            /* Keep track of the number of entries found. */
            num_entries++;
            char *dn = NULL;

            /* Get and print the DN of the entry. */
            if ((dn = ldap_get_dn(ld, res)) != NULL)
            {
                printf("dn: %s\n", dn);
                ldap_memfree(dn);
            }

            /* Iterate through each attribute in the entry. */
            for (char *a = ldap_first_attribute(ld, res, &ber); a != NULL; a = ldap_next_attribute(ld, res, ber))
            {
                /* Get and print all values for each attribute. */
                char **vals = NULL;
                if ((vals = ldap_get_values(ld, res, a)) != NULL)
                {
                    for (int i = 0; vals[i] != NULL; i++)
                    {
                        printf("%s: %s\n", a, vals[i]);
                    }

                    ldap_value_free(vals);
                }

                ldap_memfree(a);
            }

            if (ber != NULL)
            {
                ber_free(ber, 0);
            }

            printf("\n");
            ldap_msgfree(res);
            break;
        }

        case LDAP_RES_SEARCH_REFERENCE:
        {
            /* The server sent a search reference encountered during the search operation. */
            /* Keep track of the number of search references returned from the server. */
            num_refs++;
            char **referrals;

            /* Parse the result and print the search references. Ideally, rather than print them
             * out, you would follow the references. */
            int parse_rc = ldap_parse_reference(ld, res, &referrals, NULL, 1);
            if (parse_rc != LDAP_SUCCESS)
            {
                fprintf(stderr, "ldap_parse_reference: %s\n", ldap_err2string(parse_rc));
                ldap_unbind(ld);
                return 1;
            }

            if (referrals != NULL)
            {
                for (int i = 0; referrals[i] != NULL; i++)
                {
                    printf("Search reference: %s\n\n", referrals[i]);
                }

                ldap_value_free(referrals);
            }
            break;
        }

        case LDAP_RES_SEARCH_RESULT:
        {
            /* Parse the final result received from the server. Note the last
               argument is a non-zero value, which indicates that the
               LDAPMessage structure will be freed when done. (No need
               to call ldap_msgfree().) */

            finished = true;
            char *matched_msg = NULL, *error_msg = NULL;
            LDAPControl **serverctrls;

            int parse_rc = ldap_parse_result(ld, res, &rc, &matched_msg, &error_msg, NULL, &serverctrls, 1);
            if (parse_rc != LDAP_SUCCESS)
            {
                fprintf(stderr, "ldap_parse_result: %s\n", ldap_err2string(parse_rc));
                ldap_unbind(ld);
                return 1;
            }

            /* Check the results of the LDAP search operation. */
            if (rc != LDAP_SUCCESS)
            {
                fprintf(stderr, "ldap_search_ext: %s\n", ldap_err2string(rc));

                if (matched_msg != NULL && *matched_msg != '\0')
                {
                    fprintf(stderr, "Part of the DN that matches an existing entry: %s\n", matched_msg);
                }
            }
            else
            {
                printf("Search completed successfully.\n"
                       "Entries found: %d\n"
                       "Search references returned: %d\n",
                       num_entries, num_refs);
            }
            break;
        }

        default:
            break;
        }
    }

    return LDAP_SUCCESS;
}

static int search(LDAP *ld, const char *searchBase, int scope, const char *filter, char **attrs, int *msgid)
{
    int rc = ldap_search_ext(ld, searchBase, scope, filter, attrs, 0, NULL, NULL, NULL, LDAP_NO_LIMIT, msgid);
    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_search_ext: %s\n", ldap_err2string(rc));
        ldap_unbind(ld);
        return 1;
    }

    return rc;
}

TEST(ldap, asynchronous)
{
    int version = LDAP_VERSION3;
    struct timeval timeOut = {10, 0}; /* 10 second connection timeout */
    ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    LDAP *ld;
    const char *ldap_host = HOSTNAME;
    ldap_initialize(&ld, ldap_host);

    const char *binddn = BINDDN;
    struct berval passwd;
    char password[] = PASSWORD;
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

    int msgid;
    rc = search(ld, BASEDN, SCOPE, FILTER, NULL, &msgid);
    if (rc != LDAP_SUCCESS)
    {
        return;
    }

    // std::thread result_task(result, ld, msgid);
    // result_task.join();
    // https://eli.thegreenplace.net/2016/the-promises-and-challenges-of-stdasync-task-based-parallelism-in-c11/
    std::future<int> res = std::async(std::launch::async, result, ld, msgid);
    ASSERT_EQ(0, res.get());

    ldap_unbind_ext_s(ld, NULL, NULL);
}

TEST(ldap, annonymous)
{
    // http://www.andrew.cmu.edu/course/15-123-kesden/applications/labs/labA/

    int version = LDAP_VERSION3;
    struct timeval timeOut = {10, 0}; /* 10 second connection timeout */
    ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    LDAP *ld;
    const char *ldap_host = "ldap://ldap.andrew.cmu.edu";
    ldap_initialize(&ld, ldap_host);

    int rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS)
    {
        printf("ldap_sasl_bind_s: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    printf("bind successful\n");
    rc = search_s(ld, "dc=andrew,dc=cmu,dc=edu", SCOPE, "(cmuSpamFlag=FALSE)", NULL);
    if (rc != LDAP_SUCCESS)
    {
        ldap_unbind_ext_s(ld, NULL, NULL);
        FAIL() << "search failed: " << ldap_err2string(rc);
        return;
    }

    ldap_unbind_ext_s(ld, NULL, NULL);
}

// https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.1.0/com.ibm.zos.v2r1.glpa100/createpagec.htm
// ldap_create_page_control

struct LdapConfiguration
{
    std::string mPrimaryServer;
    std::string mSearchBase;
    std::string mUsername;
    std::string mPassword;
    std::string mFilter;
    std::vector<std::string> mAttributes;
    bool mUseSSL;
};

namespace YAML
{
template <> struct convert<LdapConfiguration>
{
    static bool decode(const Node &node, LdapConfiguration &rhs)
    {
        rhs.mPrimaryServer = node["PrimaryServerName"].as<std::string>();
        rhs.mSearchBase = node["SearchBase"].as<std::string>();
        rhs.mUsername = node["Username"].as<std::string>();
        rhs.mPassword = node["Password"].as<std::string>();
        rhs.mFilter = node["Filter"].as<std::string>();
        rhs.mUseSSL = node["UseSSL"].as<bool>();
        rhs.mAttributes = node["Attributes"].as<std::vector<std::string>>();
        return true;
    }
};
} // namespace YAML

int findNode(const YAML::Node &doc)
{
    std::string case_name = ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
    std::string test_name = case_name + "." + ::testing::UnitTest::GetInstance()->current_test_info()->name();

    for (unsigned i = 0; i < doc.size(); i++)
    {
        if (doc[i]["id"].as<std::string>() == test_name)
        {
            return i;
        }
    }

    return -1;
}

char **getAttributes(const std::vector<std::string> &v)
{
    char **attrs = (char **)malloc((v.size() + 1) * sizeof(char *)); // the last slot for NULL
    int index = 0;
    for (size_t i = 0; i < v.size(); ++i)
    {
        size_t len = v[i].length();
        attrs[i] = (char *)malloc(len + 1);
        strncpy(attrs[i], v[i].c_str(), len);
        attrs[i][len] = '\0';
    }
    attrs[v.size()] = NULL;
    return attrs;
}

TEST(ldap, config)
{
    YAML::Node doc = YAML::LoadFile("./data/hunter.yaml");
    int index = findNode(doc);
    if (-1 == index)
    {
        FAIL() << "config for the test is not found";
    }
    LdapConfiguration config = doc[index].as<LdapConfiguration>();

    int version = LDAP_VERSION3;
    struct timeval timeOut = {10, 0}; /* 10 second connection timeout */
    ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    std::string url;
    if (config.mUseSSL)
    {
        url = "ldaps://" + config.mPrimaryServer;
    }
    else
    {
        url = "ldap://" + config.mPrimaryServer;
    }

    LDAP *ld;
    ldap_initialize(&ld, url.c_str());

    const char *binddn = config.mUsername.c_str();
    struct berval cred;
    char *password = const_cast<char *>(config.mPassword.c_str());
    cred.bv_val = password;
    cred.bv_len = config.mPassword.length();

    int rc = ldap_sasl_bind_s(ld, binddn, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
    if (rc != LDAP_SUCCESS)
    {
        printf("ldap_sasl_bind_s: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    printf("bind successful\n");
    char **attrs = getAttributes(config.mAttributes);

    rc = search_s(ld, config.mSearchBase.c_str(), LDAP_SCOPE_SUBTREE, config.mFilter.c_str(), attrs);

    // int msgid;
    // rc = search(ld, config.mSearchBase.c_str(), LDAP_SCOPE_SUBTREE, config.mFilter.c_str(), attrs,
    //             &msgid);
    // // https://eli.thegreenplace.net/2016/the-promises-and-challenges-of-stdasync-task-based-parallelism-in-c11/
    // std::future<int> res = std::async(std::launch::async, result, ld, msgid);
    // ASSERT_EQ(0, res.get());

    ldap_unbind_ext_s(ld, NULL, NULL);
}

TEST(ldap, performance)
{
    YAML::Node doc = YAML::LoadFile("./data/hunter.yaml");
    int index = findNode(doc);
    if (-1 == index)
    {
        FAIL() << "config for the test is not found";
    }
    LdapConfiguration config = doc[index].as<LdapConfiguration>();
    std::vector<std::string> filters = doc[index]["Filters"].as<std::vector<std::string>>();

    int version = LDAP_VERSION3;
    struct timeval timeOut = {10, 0}; /* 10 second connection timeout */
    ldap_set_option(NULL, LDAP_OPT_PROTOCOL_VERSION, &version);
    ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);

    std::string url;
    if (config.mUseSSL)
    {
        url = "ldaps://" + config.mPrimaryServer;
    }
    else
    {
        url = "ldap://" + config.mPrimaryServer;
    }

    LDAP *ld;
    ldap_initialize(&ld, url.c_str());

    const char *binddn = config.mUsername.c_str();
    struct berval cred;
    char *password = const_cast<char *>(config.mPassword.c_str());
    cred.bv_val = password;
    cred.bv_len = config.mPassword.length();

    int rc = ldap_sasl_bind_s(ld, binddn, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL);
    if (rc != LDAP_SUCCESS)
    {
        printf("ldap_sasl_bind_s: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    printf("bind successful\n");
    char **attrs = getAttributes(config.mAttributes);

    for (const auto &filter : filters)
    {
        std::cout << filter << '\n';
        search_s(ld, config.mSearchBase.c_str(), LDAP_SCOPE_SUBTREE, filter.c_str(), attrs);
    }

    ldap_unbind_ext_s(ld, NULL, NULL);
}
