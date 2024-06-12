import qbs
import qbs.File
import qbs.TextFile
import "QbsExt/imports/QbsUtl/qbsutl.js" as QbsUtl

Project {
    minimumQbsVersion: "1.23.0"
    qbsSearchPaths: ["QbsExt"]

    property bool useSimd: true

    readonly property var projectVersion: projectProbe.projectVersion
    readonly property var databaseVersion: projectProbe.databaseVersion
    readonly property string projectGitRevision: projectProbe.projectGitRevision

    Probe {
        id: projectProbe
        property var projectVersion;
        property var databaseVersion;
        property string projectGitRevision;

        readonly property string projectBuildDirectory:  project.buildDirectory
        readonly property string projectSourceDirectory: project.sourceDirectory

        configure: {
            projectVersion = QbsUtl.getVersions(projectSourceDirectory + "/VERSION");
            projectGitRevision = QbsUtl.gitRevision(projectSourceDirectory);
        }
    }

    property var cppDefines: {
        var def = [
            "APPLICATION_NAME=\"Remote Control\"",
            "VERSION_PROJECT=\"" + projectVersion[0] + "\"",
            "VERSION_PROJECT_MAJOR=" + projectVersion[1],
            "VERSION_PROJECT_MINOR=" + projectVersion[2],
            "VERSION_PROJECT_PATCH=" + projectVersion[3],
            "VERSION_DATABASE=" + databaseVersion,
            "GIT_REVISION=\"" + projectGitRevision + "\"",
            "QDATASTREAM_VERSION=QDataStream::Qt_5_15",
            "PPROTO_VERSION_LOW=0",
            "PPROTO_VERSION_HIGH=0",
            "PPROTO_JSON_SERIALIZE",
            "PPROTO_QBINARY_SERIALIZE",
            "PPROTO_UDP_SIGNATURE=\"RControl\"",
            "DEFAULT_PORT=60011",
            "CONFIG_DIR=\"/etc/rcontrol\"",
            "VAROPT_DIR=\"/var/opt/rcontrol\"",
        ];

        if (useSimd === true)
            def.push("USE_SIMD");

        return def;
    }

    property var cxxFlags: [
        "-ggdb3",
        "-Wall",
        "-Wextra",
        "-Wswitch-enum",
        "-Wdangling-else",
        "-Wno-unused-parameter",
        "-Wno-variadic-macros",
        "-Wno-vla",
    ]
    property string cxxLanguageVersion: "c++17"
}
