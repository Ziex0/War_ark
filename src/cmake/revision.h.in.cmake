#ifndef __REVISION_H__
#define __REVISION_H__
 #define _HASH                      "@rev_hash@"
 #define _FULLHASH                  "@rev_hash_full@"
 #define _DATE                      "@rev_date@"
 #define _BRANCH                    "@rev_branch@"
 #define _URL_ORIGIN                "@rev_url_origin@"
 #define _CMAKE_COMMAND             R"(@CMAKE_COMMAND@)"
 #define _CMAKE_VERSION             R"(@CMAKE_VERSION@)"
 #define _CMAKE_HOST_SYSTEM         R"(@CMAKE_HOST_SYSTEM_NAME@ @CMAKE_HOST_SYSTEM_VERSION@)"
 #define _SOURCE_DIRECTORY          R"(@CMAKE_SOURCE_DIR@)"
 #define _BUILD_DIRECTORY           R"(@BUILDDIR@)"
 #define _MYSQL_EXECUTABLE          R"(@MYSQL_EXECUTABLE@)"
 #define VER_COMPANYNAME_STR        "KontolCore"
 #define VER_LEGALCOPYRIGHT_STR     "(c)2020-@rev_year@ KontolCore"
 #define VER_FILEVERSION            0,0,0
 #define VER_FILEVERSION_STR        "@rev_hash@ @rev_date@ (@rev_branch@ branch)"
 #define VER_PRODUCTVERSION         VER_FILEVERSION
 #define VER_PRODUCTVERSION_STR     VER_FILEVERSION_STR
#endif // __REVISION_H__
