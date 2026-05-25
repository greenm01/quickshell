#include <qbytearray.h>
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <qobject.h>
#include <qqml.h>
#include <qqmlcomponent.h>
#include <qqmlengine.h>
#include <qqmlextensionplugin.h>
#include <qtemporarydir.h>
#include <qtest.h>
#include <qurl.h>
#include <qvariant.h>

Q_IMPORT_QML_PLUGIN(QuickshellPlugin);
Q_IMPORT_QML_PLUGIN(Quickshell_TriadPlugin);

namespace {
QByteArray stateEvent() {
	return R"({"triad":{"version":1,"event":"state-changed","state":{"version":1,)"
	       R"("capabilities":{"event_stream":true,"state":true},)"
	       R"("overview":{"is_open":true,"selected_window_id":7},)"
	       R"("keyboard_layouts":["us"],"current_keyboard_layout_idx":0,)"
	       R"("layout":{"version":1,"layouts":[{"kind":"builtin","id":"scroller"}],)"
	       R"("layout_cycle":["scroller","grid"],)"
	       R"("layout_cycle_entries":[{"kind":"builtin","id":"scroller"}],)"
	       R"("active_tag":1,"active_workspace_idx":1,"workspaces":[)"
	       R"({"tag_id":1,"workspace_idx":1,"name":"main","output":"DP-1",)"
	       R"("layout":"scroller","layout_kind":"columns","runtime_kind":"scroller",)"
	       R"("layout_source":"core","fallback_layout":"","is_configured":true,)"
	       R"("is_active":true,"is_output_visible":true,"is_urgent":false,)"
	       R"("occupied":true,"focused_window_id":7,"columns":[{"idx":1,"windows":[7]}],)"
	       R"("frames":[],"bsp_nodes":[],"split_nodes":[],"master_count":1,"master_split_ratio":0.5,)"
	       R"("viewport":{"target_x":1,"current_x":0.5,"target_y":0,"current_y":0}}]},)"
	       R"("outputs":[{"id":1,"name":"DP-1","connected":true,"is_primary":true,)"
	       R"("refresh_rate":60000,"physical_width":600,"physical_height":340,)"
	       R"("scale":1,"transform":"Normal","geometry":{"x":0,"y":0,"width":1920,"height":1080}}],)"
	       R"("windows":[{"id":7,"pid":1234,"parent_id":null,"title":"Terminal",)"
	       R"("app_id":"kitty","tag_id":1,"workspace_idx":1,"output":"DP-1",)"
	       R"("position":{"column_idx":1,"window_idx":1},"is_focused":true,)"
	       R"("is_floating":false,"is_maximized":false,"is_minimized":false,)"
	       R"("is_sticky":false,"is_overlay":false,"is_unmanaged_global":false,)"
	       R"("is_fullscreen":false,"fullscreen_output":1,"width_proportion":0.75,)"
	       R"("height_proportion":0.5,"actual_size":{"width":100,"height":90},)"
	       R"("floating_geometry":{"x":10,"y":20,"width":300,"height":200},)"
	       R"("keyboard_shortcuts_inhibit":false,"idle_inhibit":"none",)"
	       R"("is_terminal":true,"allow_swallow":true,"swallowed_by":8,"swallowing":9}]}}})"
	       "\n";
}

QByteArray ack() {
	return R"({"ok":true,"triad":{"version":1,"type":"ack"}})"
	       "\n";
}

QByteArray commandsReply() {
	return R"({"ok":true,"triad":{"version":1,"type":"commands","catalog":{"version":1,"commands":[{"name":"focus-window","usage":"focus-window <window-id>","arg_shape":"required-window-id","aliases":[]}],"special_requests":[]}}})"
	       "\n";
}

QByteArray handleRequest(const QByteArray& request) {
	auto root = QJsonDocument::fromJson(request.trimmed()).object();
	auto triad = root.value("triad").toObject();
	if (triad.value("request").toString() == "commands") return commandsReply();
	if (triad.value("request").toString() != "set-layout") return ack();

	auto target = triad.value("target").toObject();
	if (target.contains("tag") || target.contains("workspace_idx")) return ack();
	return R"({"ok":false,"error":"target must contain tag or workspace_idx"})"
	       "\n";
}
} // namespace

class TestTriadQml: public QObject {
	Q_OBJECT;

private slots:
	void initTestCase() {
		QVERIFY(this->dir.isValid());
		this->socketPath = this->dir.filePath("triad.sock");
		QFile::remove(this->socketPath);
		qputenv("TRIAD_SOCKET", this->socketPath.toUtf8());

		QVERIFY(this->server.listen(this->socketPath));
		QObject::connect(&this->server, &QLocalServer::newConnection, this, [this]() {
			while (auto* client = this->server.nextPendingConnection()) {
				client->setParent(this);
				QObject::connect(client, &QLocalSocket::readyRead, this, [client]() {
					const auto request = client->readAll();
					if (request.contains("event-stream")) {
						client->write(ack());
						client->write(stateEvent());
					} else {
						client->write(handleRequest(request));
					}
					client->flush();
				});
			}
		});
	}

	void exposesTriadSingletonToQml() {
		auto engine = QQmlEngine();
		engine.addImportPath(QStringLiteral(QS_TEST_QML_IMPORT_PATH));

		auto component = QQmlComponent(&engine);
		component.setData(
		    R"(
import QtQml
import Quickshell.Triad

QtObject {
	property bool triadConnected: Triad.connected
	property int workspaceCount: Triad.workspaces.values.length
	property int outputCount: Triad.outputs.values.length
	property int windowCount: Triad.windows.values.length
	property string focusedTitle: Triad.focusedWindow ? Triad.focusedWindow.title : ""
	property string focusedLayout: Triad.focusedWorkspace ? Triad.focusedWorkspace.layout : ""
	property string firstWorkspaceName: Triad.workspaces.values.length > 0 ? Triad.workspaces.values[0].name : ""
	property string firstOutputName: Triad.outputs.values.length > 0 ? Triad.outputs.values[0].name : ""
	property int activeTag: Triad.activeTag
	property int overviewSelectedWindowId: Triad.overviewSelectedWindowId
	property int layoutCount: Triad.layouts.length
	property string layoutCycleSecond: Triad.layoutCycle.length > 1 ? Triad.layoutCycle[1] : ""
	property int firstOutputPhysicalWidth: Triad.outputs.values.length > 0 ? Triad.outputs.values[0].physicalWidth : -1
	property int focusedWindowFloatingX: Triad.focusedWindow ? Triad.focusedWindow.floatingX : -1
	property int focusedWindowSwallowedBy: Triad.focusedWindow ? Triad.focusedWindow.swallowedBy : -1
	property int focusedWorkspaceColumnCount: Triad.focusedWorkspace ? Triad.focusedWorkspace.columns.length : -1
	property int commandCount: Triad.commandsCatalog.commands ? Triad.commandsCatalog.commands.length : 0
	property bool helperIdsReturned: false
	property bool commandValidationWorks: false

	function exerciseActions() {
		commandValidationWorks = Triad.hasCommand("focus-window") && Triad.validateAction("focus-window", {"id": 7})
		const ids = [
			Triad.refreshCapabilities(),
			Triad.refreshWorkspaces(),
			Triad.refreshOutputs(),
			Triad.refreshOverview(),
			Triad.refreshKeyboardLayouts(),
			Triad.refreshCommands(),
			Triad.focusWorkspace(1),
			Triad.focusTag(1),
			Triad.focusWindow(7),
			Triad.closeWindow(7),
			Triad.switchLayout(),
			Triad.setLayout("grid", {"tag": 1}),
			Triad.dispatch("noop", {}),
			Triad.dispatchBinding("key", "Super+h"),
			Triad.sendValidatedAction("focus-window", {"id": 7})
		]
		helperIdsReturned = true
		for (let i = 0; i < ids.length; i++) {
			if (ids[i] <= 0) helperIdsReturned = false
		}
	}
}
)",
		    QUrl(QStringLiteral("qrc:/triad-qml-test.qml"))
		);

		QVERIFY2(component.isReady(), qPrintable(component.errorString()));

		auto* object = component.create();
		QVERIFY2(object != nullptr, qPrintable(component.errorString()));

		QTRY_VERIFY(object->property("triadConnected").toBool());
		QTRY_COMPARE(object->property("workspaceCount").toInt(), 1);
		QTRY_COMPARE(object->property("outputCount").toInt(), 1);
		QTRY_COMPARE(object->property("windowCount").toInt(), 1);
		QCOMPARE(object->property("focusedTitle").toString(), QStringLiteral("Terminal"));
		QCOMPARE(object->property("focusedLayout").toString(), QStringLiteral("scroller"));
		QCOMPARE(object->property("firstWorkspaceName").toString(), QStringLiteral("main"));
		QCOMPARE(object->property("firstOutputName").toString(), QStringLiteral("DP-1"));
		QCOMPARE(object->property("activeTag").toInt(), 1);
		QCOMPARE(object->property("overviewSelectedWindowId").toInt(), 7);
		QCOMPARE(object->property("layoutCount").toInt(), 1);
		QCOMPARE(object->property("layoutCycleSecond").toString(), QStringLiteral("grid"));
		QCOMPARE(object->property("firstOutputPhysicalWidth").toInt(), 600);
		QCOMPARE(object->property("focusedWindowFloatingX").toInt(), 10);
		QCOMPARE(object->property("focusedWindowSwallowedBy").toInt(), 8);
		QCOMPARE(object->property("focusedWorkspaceColumnCount").toInt(), 1);
		QTRY_COMPARE(object->property("commandCount").toInt(), 1);
		QVERIFY(QMetaObject::invokeMethod(object, "exerciseActions"));
		QVERIFY(object->property("commandValidationWorks").toBool());
		QVERIFY(object->property("helperIdsReturned").toBool());

		delete object;
	}

private:
	QTemporaryDir dir;
	QString socketPath;
	QLocalServer server;
};

QTEST_MAIN(TestTriadQml);
#include "triad_qml.moc"
