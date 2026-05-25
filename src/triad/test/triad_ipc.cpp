#include <qbytearray.h>
#include <qcontainerfwd.h>
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
#include <qvariant.h>

#include "../connection.hpp"
#include "../output.hpp"
#include "../window.hpp"
#include "../workspace.hpp"

using namespace qs::triad;

namespace {
QByteArray ackReply() {
	return R"({"ok":true,"triad":{"version":1,"type":"ack"}})"
	       "\n";
}

QByteArray errorReply(const char* error) {
	return QByteArray(R"({"ok":false,"error":")") + error + "\"}\n";
}

QByteArray commandsReply() {
	return R"({"ok":true,"triad":{"version":1,"type":"commands","catalog":{"version":1,"commands":[)"
	       R"({"name":"focus-window","usage":"focus-window <window-id>","arg_shape":"required-window-id","aliases":[]},)"
	       R"({"name":"close-window","usage":"close-window [window-id]","arg_shape":"optional-window-id","aliases":["kill-window"]},)"
	       R"({"name":"move-window-to-tag","usage":"move-window-to-tag <window-id> <tag> [follow]","arg_shape":"window-tag-follow","aliases":[]},)"
	       R"({"name":"spawn","usage":"spawn <argv...>","arg_shape":"spawn-argv","aliases":[]},)"
	       R"({"name":"switch-keyboard-layout","usage":"switch-keyboard-layout [next|prev|index]","arg_shape":"keyboard-layout-target","aliases":[]},)"
	       R"({"name":"screenshot","usage":"screenshot [--path <path>]","arg_shape":"screenshot","aliases":[]})"
	       R"(],"special_requests":[{"name":"layout-state"}]}}})"
	       "\n";
}

QByteArray focusedWindowNullReply() {
	return R"({"ok":true,"triad":{"version":1,"type":"focused-window","window":null}})"
	       "\n";
}

QByteArray layoutEvent(const char* layout = "scroller") {
	return QByteArray(
	           R"({"triad":{"version":1,"event":"layout-state-changed","state":{)"
	           R"("version":1,"layouts":[{"kind":"builtin","id":"scroller"}],)"
	           R"("layout_cycle":["scroller","grid"],)"
	           R"("layout_cycle_entries":[{"kind":"builtin","id":"scroller"}],)"
	           R"("active_tag":1,"active_workspace_idx":1,"workspaces":[)"
	           R"({"tag_id":1,"workspace_idx":1,"name":"main","output":"DP-1",)"
	       )
	     + R"("layout":")" + layout
	     + R"(","layout_kind":"columns","runtime_kind":"scroller","layout_source":"core",)"
	       R"("fallback_layout":"","is_configured":true,"is_active":true,)"
	       R"("is_output_visible":true,"is_urgent":false,"occupied":true,)"
	       R"("focused_window_id":7,"columns":[{"idx":1,"windows":[7]}],)"
	       R"("frames":[{"id":1,"kind":"leaf","focused":true}],)"
	       R"("bsp_nodes":[{"id":1,"kind":"leaf","window_id":7}],)"
	       R"("split_nodes":[{"id":2,"kind":"leaf"}],)"
	       R"("master_count":1,"master_split_ratio":0.5,)"
	       R"("viewport":{"target_x":1,"current_x":0.5,"target_y":2,"current_y":1.5}},)"
	       R"({"tag_id":2,"workspace_idx":2,"name":null,"output":"DP-2",)"
	       R"("layout":"grid","layout_kind":"grid","runtime_kind":"grid","layout_source":"core",)"
	       R"("fallback_layout":"","is_configured":true,"is_active":false,)"
	       R"("is_output_visible":true,"is_urgent":false,"occupied":false,)"
	       R"("focused_window_id":null,"columns":[],"frames":[],"bsp_nodes":[],)"
	       R"("split_nodes":[],"master_count":1,"master_split_ratio":0.5,)"
	       R"("viewport":{"target_x":0,"current_x":0,"target_y":0,"current_y":0}}]}}})"
	       "\n";
}

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
	       R"("frames":[{"id":1,"kind":"leaf","focused":true}],)"
	       R"("bsp_nodes":[{"id":1,"kind":"leaf","window_id":7}],)"
	       R"("split_nodes":[{"id":2,"kind":"leaf"}],"master_count":1,"master_split_ratio":0.5,)"
	       R"("viewport":{"target_x":1,"current_x":0.5,"target_y":2,"current_y":1.5}}]},)"
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

QByteArray handleRequest(const QByteArray& request) {
	auto root = QJsonDocument::fromJson(request.trimmed()).object();
	auto triad = root.value("triad").toObject();
	auto requestName = triad.value("request").toString();
	if (requestName == "commands") return commandsReply();
	if (requestName == "focused-window") return focusedWindowNullReply();

	if (requestName == "set-layout") {
		auto target = triad.value("target").toObject();
		if (target.contains("tag") || target.contains("workspace_idx")) return ackReply();
		return errorReply("target must contain tag or workspace_idx");
	}

	if (requestName == "action" && triad.value("action").toString() == "focus-window") {
		if (triad.value("id").isDouble()) return ackReply();
		return errorReply("unknown action or bad parameters: focus-window");
	}

	if (requestName == "dispatch-binding") {
		if (triad.value("kind").isString() && triad.value("binding").isString()) return ackReply();
		return errorReply("invalid dispatch-binding request");
	}

	return ackReply();
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
					auto root = QJsonDocument::fromJson(request.trimmed()).object();
					auto triad = root.value("triad").toObject();
					auto requestName = triad.value("request").toString();
					if (!requestName.isEmpty()) this->requestNames.push_back(requestName);
					this->requestPayloads.push_back(triad);
					if (request.contains("event-stream")) {
						this->eventClient = client;
						client->write(ackReply());
						client->write(layoutEvent());
						client->write(stateEvent());
					} else {
						client->write(handleRequest(request));
					}
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
		QTRY_COMPARE(ipc->commandsCatalog().value("commands").toList().size(), 6);

		QCOMPARE(ipc->bindableFocusedWorkspace().value()->bindableTagId().value(), 1);
		QCOMPARE(ipc->bindableActiveTag().value(), 1);
		QCOMPARE(ipc->bindableActiveWorkspaceIndex().value(), 1);
		QCOMPARE(ipc->bindableOverviewOpen().value(), true);
		QCOMPARE(ipc->bindableOverviewSelectedWindowId().value(), 7);
		QCOMPARE(ipc->layouts().size(), 1);
		QCOMPARE(ipc->layoutCycle(), QStringList({"scroller", "grid"}));
		QCOMPARE(ipc->layoutCycleEntries().size(), 1);

		auto* output = ipc->bindableFocusedOutput().value();
		QCOMPARE(output->bindableName().value(), QString("DP-1"));
		QCOMPARE(output->bindablePhysicalWidth().value(), 600);
		QCOMPARE(output->bindablePhysicalHeight().value(), 340);

		auto* window = ipc->bindableFocusedWindow().value();
		QCOMPARE(window->bindableTitle().value(), QString("Terminal"));
		QCOMPARE(window->bindableParentId().value(), -1);
		QCOMPARE(window->bindableColumnIndex().value(), 1);
		QCOMPARE(window->bindableWindowIndex().value(), 1);
		QCOMPARE(window->bindableFullscreenOutput().value(), 1);
		QCOMPARE(window->bindableWidthProportion().value(), 0.75);
		QCOMPARE(window->bindableHeightProportion().value(), 0.5);
		QCOMPARE(window->bindableActualWidth().value(), 100);
		QCOMPARE(window->bindableActualHeight().value(), 90);
		QCOMPARE(window->bindableFloatingX().value(), 10);
		QCOMPARE(window->bindableFloatingY().value(), 20);
		QCOMPARE(window->bindableFloatingWidth().value(), 300);
		QCOMPARE(window->bindableFloatingHeight().value(), 200);
		QCOMPARE(window->bindableKeyboardShortcutsInhibit().value(), false);
		QCOMPARE(window->bindableIdleInhibit().value(), QString("none"));
		QCOMPARE(window->bindableAllowSwallow().value(), true);
		QCOMPARE(window->bindableSwallowedBy().value(), 8);
		QCOMPARE(window->bindableSwallowing().value(), 9);

		auto* workspace = ipc->bindableFocusedWorkspace().value();
		QCOMPARE(workspace->bindableLayoutSource().value(), QString("core"));
		QCOMPARE(workspace->bindableConfigured().value(), true);
		QCOMPARE(workspace->columns().size(), 1);
		QCOMPARE(workspace->frames().size(), 1);
		QCOMPARE(workspace->bspNodes().size(), 1);
		QCOMPARE(workspace->splitNodes().size(), 1);
		QCOMPARE(workspace->viewport().value("target_x").toReal(), 1.0);
		QCOMPARE(workspace->bindableFocusedWindow().value(), window);
	}

	void emitsRequestResults() {
		auto* ipc = TriadIpc::instance();
		QSignalSpy spy(ipc, &TriadIpc::requestFinished);
		auto requestId = ipc->sendRequest("commands");
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), requestId);
		QCOMPARE(spy.at(0).at(1).toBool(), true);
		QCOMPARE(ipc->commandsCatalog().value("commands").toList().size(), 6);

		auto badId = ipc->sendAction("focus-window", {{"id", QString("bad")}});
		QTRY_COMPARE(spy.size(), 2);
		QCOMPARE(spy.at(1).at(0).toInt(), badId);
		QCOMPARE(spy.at(1).at(1).toBool(), false);
		QVERIFY(spy.at(1).at(3).toString().contains("focus-window"));
	}

	void exposesTrackedConvenienceRequests() {
		auto* ipc = TriadIpc::instance();
		QSignalSpy spy(ipc, &TriadIpc::requestFinished);

		auto workspaceId = ipc->focusWorkspace(2);
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), workspaceId);
		QCOMPARE(spy.at(0).at(1).toBool(), true);

		auto layoutId = ipc->setLayout("grid", {{"tag", 1}});
		QTRY_COMPARE(spy.size(), 2);
		QCOMPARE(spy.at(1).at(0).toInt(), layoutId);
		QCOMPARE(spy.at(1).at(1).toBool(), true);
	}

	void sendsDispatchBindingRequests() {
		auto* ipc = TriadIpc::instance();
		QSignalSpy spy(ipc, &TriadIpc::requestFinished);

		auto keyId = ipc->dispatchBinding("key", "Super+h");
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), keyId);
		QCOMPARE(this->lastRequestPayload("dispatch-binding").value("kind").toString(), QString("key"));

		auto axisId = ipc->dispatchBinding("axis", "Super+wheel-up", 2);
		QTRY_COMPARE(spy.size(), 2);
		QCOMPARE(spy.at(1).at(0).toInt(), axisId);
		QCOMPARE(this->lastRequestPayload("dispatch-binding").value("ticks").toInt(), 2);

		auto gestureId = ipc->dispatchBinding("gesture", "Super+swipe-left", 3);
		QTRY_COMPARE(spy.size(), 3);
		QCOMPARE(spy.at(2).at(0).toInt(), gestureId);
		QCOMPARE(this->lastRequestPayload("dispatch-binding").value("fingers").toInt(), 3);
	}

	void exposesRefreshHelpers() {
		auto* ipc = TriadIpc::instance();

		auto before = this->requestCount("capabilities");
		ipc->refreshCapabilities();
		QTRY_COMPARE(this->requestCount("capabilities"), before + 1);

		before = this->requestCount("workspaces");
		ipc->refreshWorkspaces();
		QTRY_COMPARE(this->requestCount("workspaces"), before + 1);

		before = this->requestCount("outputs");
		ipc->refreshOutputs();
		QTRY_COMPARE(this->requestCount("outputs"), before + 1);

		before = this->requestCount("overview-state");
		ipc->refreshOverview();
		QTRY_COMPARE(this->requestCount("overview-state"), before + 1);

		before = this->requestCount("keyboard-layouts");
		ipc->refreshKeyboardLayouts();
		QTRY_COMPARE(this->requestCount("keyboard-layouts"), before + 1);

		before = this->requestCount("commands");
		ipc->refreshCommands();
		QTRY_COMPARE(this->requestCount("commands"), before + 1);
	}

	void validatesCommandCatalogActions() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY(ipc->hasCommand("focus-window"));
		QVERIFY(ipc->hasCommand("kill-window"));
		QCOMPARE(ipc->commandSpec("kill-window").value("name").toString(), QString("close-window"));

		QVERIFY(ipc->validateAction("focus-window", {{"id", 7}}));
		QVERIFY(!ipc->validateAction("focus-window", {{"id", QString("bad")}}));
		QVERIFY(ipc->validateAction("move-window-to-tag", {{"id", 7}, {"tag", 2}, {"follow", true}}));
		QVERIFY(ipc->validateAction("spawn", {{"argv", QStringList({"kitty"})}}));
		QVERIFY(ipc->validateAction("switch-keyboard-layout", {{"layout", QString("next")}}));
		QVERIFY(ipc->validateAction("screenshot", {{"path", QString("/tmp/a.png")}, {"show_pointer", true}}));

		QSignalSpy spy(ipc, &TriadIpc::requestFinished);
		QCOMPARE(ipc->sendValidatedAction("focus-window", {{"id", QString("bad")}}), -1);
		QCOMPARE(spy.size(), 0);
		auto requestId = ipc->sendValidatedAction("focus-window", {{"id", 7}});
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), requestId);
	}

	void clearsExplicitlyNullFocusedWindow() {
		auto* ipc = TriadIpc::instance();
		QVERIFY(ipc->bindableFocusedWindow().value() != nullptr);

		QSignalSpy spy(ipc, &TriadIpc::requestFinished);
		auto requestId = ipc->refreshFocusedWindow();
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), requestId);
		QCOMPARE(ipc->bindableFocusedWindow().value(), nullptr);
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
	int requestCount(const QString& name) const { return this->requestNames.count(name); }

	QJsonObject lastRequestPayload(const QString& name) const {
		for (auto i = this->requestPayloads.size() - 1; i >= 0; i--) {
			if (this->requestPayloads.at(i).value("request").toString() == name) {
				return this->requestPayloads.at(i);
			}
		}
		return {};
	}

	QTemporaryDir dir;
	QString socketPath;
	QLocalServer server;
	QList<QLocalSocket*> clients;
	QList<QString> requestNames;
	QList<QJsonObject> requestPayloads;
	QLocalSocket* eventClient = nullptr;
};

QTEST_MAIN(TestTriadIpc);
#include "triad_ipc.moc"
