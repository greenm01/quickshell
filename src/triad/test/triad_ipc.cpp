#include <qbytearray.h>
#include <qcontainerfwd.h>
#include <qcoreapplication.h>
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qlist.h>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <qobject.h>
#include <qsignalspy.h>
#include <qtemporarydir.h>
#include <qtest.h>

#include "../connection.hpp"
#include "../output.hpp"
#include "../window.hpp"
#include "../workspace.hpp"

using namespace qs::triad;

namespace {
QByteArray eventStreamAck() {
	return R"({"ok":true,"triad":{"version":1,"type":"ack"}})"
	       "\n";
}

QByteArray layoutEvent(const char* layout = "scroller") {
	return QByteArray(
	           R"({"triad":{"version":1,"event":"layout-state-changed","state":{)"
	           R"("version":1,"layouts":[],"layout_cycle":[],"layout_cycle_entries":[],)"
	           R"("active_tag":1,"active_workspace_idx":1,"workspaces":[)"
	           R"({"tag_id":1,"workspace_idx":1,"name":"main","output":"DP-1",)"
	       )
	     + R"("layout":")" + layout
	     + R"(","layout_kind":"columns","runtime_kind":"scroller","layout_source":"core",)"
	       R"("fallback_layout":"","is_configured":true,"is_active":true,)"
	       R"("is_output_visible":true,"is_urgent":false,"occupied":true,)"
	       R"("focused_window_id":7,"columns":[],"frames":[],"bsp_nodes":[],)"
	       R"("split_nodes":[],"master_count":1,"master_split_ratio":0.5},)"
	       R"({"tag_id":2,"workspace_idx":2,"name":null,"output":"DP-2",)"
	       R"("layout":"grid","layout_kind":"grid","runtime_kind":"grid","layout_source":"core",)"
	       R"("fallback_layout":"","is_configured":true,"is_active":false,)"
	       R"("is_output_visible":true,"is_urgent":false,"occupied":false,)"
	       R"("focused_window_id":null,"columns":[],"frames":[],"bsp_nodes":[],)"
	       R"("split_nodes":[],"master_count":1,"master_split_ratio":0.5}]}}})"
	       "\n";
}

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
} // namespace

class TestTriadIpc: public QObject {
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
				this->clients.push_back(client);

				QObject::connect(client, &QLocalSocket::readyRead, this, [this, client]() {
					auto request = client->readAll();
					if (!request.contains("event-stream")) return;

					this->eventClient = client;
					client->write(eventStreamAck());
					client->write(layoutEvent());
					client->write(stateEvent());
					client->flush();
				});
			}
		});
	}

	void loadsInitialState() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY(ipc->bindableConnected().value());
		QTRY_COMPARE(ipc->workspaces()->valueList().size(), 1);
		QTRY_COMPARE(ipc->outputs()->valueList().size(), 1);
		QTRY_COMPARE(ipc->windows()->valueList().size(), 1);

		QCOMPARE(ipc->bindableFocusedWorkspace().value()->bindableTagId().value(), 1);
		QCOMPARE(ipc->bindableFocusedOutput().value()->bindableName().value(), QString("DP-1"));
		QCOMPARE(ipc->bindableFocusedWindow().value()->bindableTitle().value(), QString("Terminal"));
		QCOMPARE(
		    ipc->bindableFocusedWorkspace().value()->bindableFocusedWindow().value(),
		    ipc->bindableFocusedWindow().value()
		);
	}

	void preservesWorkspaceIdentity() {
		auto* ipc = TriadIpc::instance();
		auto* workspace = ipc->bindableFocusedWorkspace().value();
		QVERIFY(workspace != nullptr);
		QVERIFY(this->eventClient != nullptr);

		this->eventClient->write(layoutEvent("grid"));
		this->eventClient->flush();

		QTRY_COMPARE(workspace->bindableLayout().value(), QString("grid"));
		QCOMPARE(ipc->bindableFocusedWorkspace().value(), workspace);
	}

private:
	QTemporaryDir dir;
	QString socketPath;
	QLocalServer server;
	QList<QLocalSocket*> clients;
	QLocalSocket* eventClient = nullptr;
};

QTEST_MAIN(TestTriadIpc);
#include "triad_ipc.moc"
