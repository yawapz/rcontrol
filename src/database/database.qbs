import qbs
import QbsUtl

Product {
    name: "Database"
    targetName: "database"
    condition: true

    type: "staticlibrary"

    Depends { name: "cpp" }
    Depends { name: "SharedLib" }
    Depends { name: "Yaml" }
    Depends { name: "Qt"; submodules: ["core", "sql", "network"] }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags //.concat(["-fPIC"])
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    property var includePaths: [
        "./",
        "/usr/include/postgresql"
    ]
    cpp.includePaths: includePaths

    // Декларация нужна для подавления Qt warning-ов
    cpp.systemIncludePaths: Qt.core.cpp.includePaths

    files: [
        "database/connect_pool.h",
//        "database/mssql_driver.cpp",
//        "database/mssql_driver.h",
//        "database/mssql_pool.cpp",
//        "database/mssql_pool.h",
        "database/postgres_driver.cpp",
        "database/postgres_driver.h",
        "database/postgres_pool.cpp",
        "database/postgres_pool.h",
        "database/sql_func.cpp",
        "database/sql_func.h",
        "database/sqlcachedresult.cpp",
        "database/sqlcachedresult.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: exportingProduct.includePaths
    }
}
