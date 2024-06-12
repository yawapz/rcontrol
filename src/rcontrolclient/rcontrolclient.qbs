import qbs
import QbsUtl

Product {
    name: "RControlClient"
    targetName: "rcontrolclient"
    condition: true

    type: "application"
    destinationDirectory: "./bin"

    Depends { name: "cpp" }
    Depends { name: "Commands" }
    Depends { name: "PProto" }
    Depends { name: "RapidJson" }
    Depends { name: "SharedLib" }
    Depends { name: "Yaml" }
  //Depends { name: "Qt"; submodules: ["core", "network", "widgets",
  //                                   "multimedia", "multimediawidgets"] }
    Depends { name: "Qt"; submodules: ["core", "network", "widgets"] }

    cpp.defines: project.cppDefines
    cpp.cxxFlags: project.cxxFlags
    cpp.cxxLanguageVersion: project.cxxLanguageVersion

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.driverLinkerFlags: ["--machine-windows"]
    }

    cpp.includePaths: [
        "./",
        "./widgets",
    ]

    Group {
        name: "resources"
        files: [
            "rcontrolclient.qrc",
            "rcontrolclient.rc",
        ]
    }

    Group {
        name: "widgets"
        prefix: "widgets/"
        files: [
            "edit_window.cpp",
            "edit_window.h",
            "edit_window.ui",
            "gpu_item.cpp",
            "gpu_item.h",
            "gpu_item.ui",
            "gpu_settings.cpp",
            "gpu_settings.h",
            "gpu_settings.ui",
            "login_window.cpp",
            "login_window.h",
            "login_window.ui",
            "main_window.cpp",
            "main_window.h",
            "main_window.ui",
            "registration_window.cpp",
            "registration_window.h",
            "registration_window.ui",
            "settings_window.cpp",
            "settings_window.h",
            "settings_window.ui",
            "worker_item.cpp",
            "worker_item.h",
            "worker_item.ui",
            "worker_widget.cpp",
            "worker_widget.h",
            "worker_widget.ui",
        ]
    }

    cpp.dynamicLibraries: {
        var libs = [
            "pthread"
        ]
        if (qbs.toolchain.contains("mingw"))
            libs.push("ws2_32");

        return libs;
    }


    files: [
        "rcontrolclient.cpp",
        "message_box.h",
        "message_box.cpp",
    ]

} // Product
