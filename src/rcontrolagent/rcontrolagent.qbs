import qbs
import QbsUtl
import ProbExt

Product {
    name: "RControlAgent"
    targetName: "rcontrolagent"
    condition: true

    type: "application"
    destinationDirectory: "./bin"

    Depends { name: "cpp" }
    Depends { name: "Commands" }
    Depends { name: "PProto" }
    Depends { name: "RapidJson" }
    Depends { name: "SharedLib" }
    Depends { name: "Yaml" }
    Depends { name: "Qt"; submodules: ["core", "network"] }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: {
        var cxx = project.cxxFlags;
        var isArm = qbs.architecture.startsWith("arm");
        if (isArm === true) {
            //cxx.push("-mtune=cortex-a15.cortex-a7");
            //cxx.push("-mfloat-abi=hard");
            //cxx.push("-mfpu=neon");
        }
        if (!isArm && (project.useSimd === true)) {
            cxx.push("-mavx");
        }
        return cxx;
    }
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
        //"event_log.cpp",
        //"event_log.h",
        "rcontrolagent.cpp",
        "rcontrolagent_appl.cpp",
        "rcontrolagent_appl.h",
    ]

} // Product
