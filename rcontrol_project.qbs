import qbs
import "rcontrol_base.qbs" as RemoteControlBase

RemoteControlBase {
    name: "RControl (Project)"

    references: [
        "src/commands/commands.qbs",
        "src/database/database.qbs",
        "src/pproto/pproto.qbs",
        "src/rapidjson/rapidjson.qbs",
        "src/shared/shared.qbs",
        "src/rcontrolctl/rcontrolctl.qbs",
        "src/rcontrolclient/rcontrolclient.qbs",
        "src/rcontrolagent/rcontrolagent.qbs",
        "src/yaml/yaml.qbs",
    ]
}
