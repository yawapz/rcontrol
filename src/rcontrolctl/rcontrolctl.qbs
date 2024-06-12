import qbs
import QbsUtl
import ProbExt

Product {
    name: "RControlCtl"
    targetName: "rcontrolctl"
    condition: true

    type: "application"
    destinationDirectory: "./bin"

    Depends { name: "cpp" }
    Depends { name: "Commands" }
    Depends { name: "Database" }
    Depends { name: "PProto" }
    Depends { name: "RapidJson" }
    Depends { name: "SharedLib" }
    Depends { name: "Yaml" }
    Depends { name: "Qt"; submodules: ["core", "network", "sql"] }

    cpp.defines: project.cppDefines
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    cpp.includePaths: [
        "./",
        "../",
    ]

    cpp.systemIncludePaths: QbsUtl.concatPaths(
        Qt.core.cpp.includePaths // Декларация для подавления Qt warning-ов
    )

    cpp.dynamicLibraries: QbsUtl.concatPaths(
        "pthread"
       ,"pq"
    )

    files: [
        "rcontrolctl.cpp",
        "rcontrolctl_appl.cpp",
        "rcontrolctl_appl.h",
        "rcontrolctl_wdc.cpp",
        "rcontrolctl_wdc.h",
    ]

} // Product
