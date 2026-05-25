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
	       R"("overview":{"is_open":false,"selected_window_id":null},)"
	       R"("keyboard_layouts":["us"],"current_keyboard_layout_idx":0,)"
	       R"("layout":{"version":1,"layouts":[],"layout_cycle":[],"layout_cycle_entries":[],)"
	       R"("active_tag":1,"active_workspace_idx":1,"workspaces":[)"
	       R"({"tag_id":1,"workspace_idx":1,"name":"main","output":"DP-1",)"
	       R"("layout":"scroller","layout_kind":"columns","runtime_kind":"scroller",)"
	       R"("layout_source":"core","fallback_layout":"","is_configured":true,)"
	       R"("is_active":true,"is_output_visible":true,"is_urgent":false,)"
	       R"("occupied":true,"focused_window_id":7,"columns":[],"frames":[],)"
	       R"("bsp_nodes":[],"split_nodes":[],"master_count":1,"master_split_ratio":0.5}]},)"
	       R"("outputs":[{"id":1,"name":"DP-1","connected":true,"is_primary":true,)"
	       R"("refresh_rate":60000,"physical_width":600,"physical_height":340,)"
	       R"("scale":1,"transform":"Normal","geometry":{"x":0,"y":0,"width":1920,"height":1080}}],)"
	       R"("windows":[{"id":7,"pid":1234,"parent_id":null,"title":"Terminal",)"
	       R"("app_id":"kitty","tag_id":1,"workspace_idx":1,"output":"DP-1",)"
	       R"("position":{"column_idx":1,"window_idx":1},"is_focused":true,)"
	       R"("is_floating":false,"is_maximized":false,"is_minimized":false,)"
	       R"("is_sticky":false,"is_overlay":false,"is_unmanaged_global":false,)"
	       R"("is_fullscreen":false,"fullscreen_output":null,"width_proportion":1,)"
	       R"("height_proportion":1,"actual_size":{"width":100,"height":100},)"
	       R"("floating_geometry":{"x":0,"y":0,"width":100,"height":100},)"
	       R"("keyboard_shortcuts_inhibit":false,"idle_inhibit":"none",)"
	       R"("is_terminal":true,"allow_swallow":true,"swallowed_by":null,"swallowing":null}]}}})"
	       "\n";
}

QByteArray ack() {
	return R"({"ok":true,"triad":{"version":1,"type":"ack"}})"
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
						client->write(ack());
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

	function exerciseActions() {
		Triad.focusWorkspace(1)
		Triad.focusTag(1)
		Triad.focusWindow(7)
		Triad.closeWindow(7)
		Triad.switchLayout()
		Triad.setLayout("grid", {"tag_id": 1})
		Triad.dispatch("noop", {})
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
		QVERIFY(QMetaObject::invokeMethod(object, "exerciseActions"));

		delete object;
	}

private:
	QTemporaryDir dir;
	QString socketPath;
	QLocalServer server;
};

QTEST_MAIN(TestTriadQml);
#include "triad_qml.moc"
