#include <utility>

#include <qbytearray.h>
#include <qcontainerfwd.h>
#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qlist.h>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qsignalspy.h>
#include <qtemporarydir.h>
#include <qtenvironmentvariables.h>
#include <qtest.h>
#include <qtestcase.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>

#include "../connection.hpp"
#include "../output.hpp"
#include "../window.hpp"
#include "../workspace.hpp"

using namespace qs::triad;

namespace {
constexpr quint32 HIGH_WINDOW_ID = 3000000000U;

QByteArray ackReply() {
	return R"({"ok":true,"triad":{"version":1,"type":"ack"}})"
	       "\n";
}

QByteArray errorReply(const char* error) {
	return QByteArray(R"({"ok":false,"error":")") + error + "\"}\n";
}

QByteArray commandsReply() {
	return R"({"ok":true,"triad":{"version":1,"type":"commands","catalog":{"version":1,"commands":[)"
	       R"({"name":"focus-next","usage":"focus-next","arg_shape":"none","aliases":[]},)"
	       R"({"name":"close-window","usage":"close-window [window-id]","arg_shape":"optional-window-id","aliases":["kill-window"]},)"
	       R"({"name":"focus-window","usage":"focus-window <window-id>","arg_shape":"required-window-id","aliases":[]},)"
	       R"({"name":"move-window-to-tag","usage":"move-window-to-tag <window-id> <tag> [follow]","arg_shape":"window-tag-follow","aliases":[]},)"
	       R"({"name":"move-window-to-workspace","usage":"move-window-to-workspace <window-id> <workspace-idx> [follow]","arg_shape":"window-workspace-follow","aliases":[]},)"
	       R"({"name":"set-window-floating","usage":"set-window-floating <window-id> true|false","arg_shape":"window-bool","aliases":[]},)"
	       R"({"name":"set-layout-for-workspace","usage":"set-layout-for-workspace <tag> <layout>","arg_shape":"tag-layout","aliases":[]},)"
	       R"({"name":"layout-custom","usage":"layout-custom <name>","arg_shape":"required-name","aliases":[]},)"
	       R"({"name":"power-off-monitor","usage":"power-off-monitor <output>","arg_shape":"required-output","aliases":[]},)"
	       R"({"name":"power-on-monitor","usage":"power-on-monitor <output>","arg_shape":"required-output","aliases":[]},)"
	       R"({"name":"adjust-master-ratio","usage":"adjust-master-ratio <delta>","arg_shape":"required-float-delta","aliases":[]},)"
	       R"({"name":"master-ratio","usage":"master-ratio <value>","arg_shape":"required-float-value","aliases":[]},)"
	       R"({"name":"set-column-width","usage":"set-column-width <value>","arg_shape":"required-float-value","aliases":[]},)"
	       R"({"name":"master-count","usage":"master-count <count>","arg_shape":"required-int-count","aliases":[]},)"
	       R"({"name":"adjust-master-count","usage":"adjust-master-count <delta>","arg_shape":"required-int-delta","aliases":[]},)"
	       R"({"name":"switch-proportion-preset","usage":"switch-proportion-preset [delta]","arg_shape":"optional-int-delta","aliases":[]},)"
	       R"({"name":"move-floating","usage":"move-floating <dx> <dy>","arg_shape":"move-delta","aliases":[]},)"
	       R"({"name":"resize-floating","usage":"resize-floating <dw> <dh>","arg_shape":"resize-delta","aliases":[]},)"
	       R"({"name":"recent-window-next","usage":"recent-window-next [--scope] [--filter]","arg_shape":"recent-advance","aliases":[]},)"
	       R"({"name":"recent-window-scope","usage":"recent-window-scope all|workspace|output","arg_shape":"recent-scope","aliases":[]},)"
	       R"({"name":"spawn","usage":"spawn <argv...>","arg_shape":"spawn-argv","aliases":[]},)"
	       R"({"name":"warp-pointer","usage":"warp-pointer <x> <y>","arg_shape":"warp-pointer","aliases":[]},)"
	       R"({"name":"screenshot","usage":"screenshot [--path <path>]","arg_shape":"screenshot","aliases":[]},)"
	       R"({"name":"split-tree-layout-cycle","usage":"split-tree-layout-cycle <modes...>","arg_shape":"split-tree-mode-list","aliases":[]},)"
	       R"({"name":"frame-resize-left","usage":"frame-resize-left [delta]","arg_shape":"optional-float-delta","aliases":[]},)"
	       R"({"name":"switch-keyboard-layout","usage":"switch-keyboard-layout [next|prev|index]","arg_shape":"keyboard-layout-target","aliases":[]},)"
	       R"({"name":"power-off-monitors","usage":"power-off-monitors","arg_shape":"none","aliases":[]},)"
	       R"({"name":"power-on-monitors","usage":"power-on-monitors","arg_shape":"none","aliases":[]},)"
	       R"({"name":"toggle-overview","usage":"toggle-overview","arg_shape":"none","aliases":[]},)"
	       R"({"name":"open-overview","usage":"open-overview","arg_shape":"none","aliases":[]},)"
	       R"({"name":"close-overview","usage":"close-overview","arg_shape":"none","aliases":[]},)"
	       R"({"name":"toggle-scratchpad","usage":"toggle-scratchpad","arg_shape":"none","aliases":[]},)"
	       R"({"name":"toggle-named-scratchpad","usage":"toggle-named-scratchpad <name>","arg_shape":"required-name","aliases":[]},)"
	       R"({"name":"move-to-scratchpad","usage":"move-to-scratchpad","arg_shape":"none","aliases":[]},)"
	       R"({"name":"move-to-named-scratchpad","usage":"move-to-named-scratchpad <name>","arg_shape":"required-name","aliases":[]},)"
	       R"({"name":"toggle-floating","usage":"toggle-floating","arg_shape":"none","aliases":[]},)"
	       R"({"name":"fullscreen-window","usage":"fullscreen-window [window-id]","arg_shape":"optional-window-id","aliases":["toggle-fullscreen"]},)"
	       R"({"name":"maximize-window-to-edges","usage":"maximize-window-to-edges","arg_shape":"none","aliases":["toggle-maximized"]},)"
	       R"({"name":"minimize","usage":"minimize","arg_shape":"none","aliases":[]},)"
	       R"({"name":"move-to-tag","usage":"move-to-tag <tag>","arg_shape":"required-tag","aliases":[]},)"
	       R"({"name":"move-to-workspace","usage":"move-to-workspace <workspace-idx>","arg_shape":"required-workspace-idx","aliases":[]},)"
	       R"({"name":"focus-output","usage":"focus-output <output>","arg_shape":"required-output","aliases":[]},)"
	       R"({"name":"move-workspace-to-output","usage":"move-workspace-to-output <output>","arg_shape":"required-output","aliases":[]},)"
	       R"({"name":"move-to-output","usage":"move-to-output <output>","arg_shape":"required-output","aliases":[]},)"
	       R"({"name":"new-workspace","usage":"new-workspace","arg_shape":"none","aliases":[]})"
	       R"(,{"name":"future-command","usage":"future-command <value>","arg_shape":"future-shape","aliases":[]})"
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
	       R"("focused_window_id":3000000000,"columns":[{"idx":1,"windows":[3000000000]}],)"
	       R"("frames":[{"id":1,"kind":"leaf","focused":true}],)"
	       R"("bsp_nodes":[{"id":1,"kind":"leaf","window_id":3000000000}],)"
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
	       R"("overview":{"is_open":true,"selected_window_id":3000000000},)"
	       R"("keyboard_layouts":["us"],"current_keyboard_layout_idx":0,)"
	       R"("layout":{"version":1,"layouts":[{"kind":"builtin","id":"scroller"}],)"
	       R"("layout_cycle":["scroller","grid"],)"
	       R"("layout_cycle_entries":[{"kind":"builtin","id":"scroller"}],)"
	       R"("active_tag":1,"active_workspace_idx":1,"workspaces":[)"
	       R"({"tag_id":1,"workspace_idx":1,"name":"main","output":"DP-1",)"
	       R"("layout":"scroller","layout_kind":"columns","runtime_kind":"scroller",)"
	       R"("layout_source":"core","fallback_layout":"","is_configured":true,)"
	       R"("is_active":true,"is_output_visible":true,"is_urgent":false,)"
	       R"("occupied":true,"focused_window_id":3000000000,"columns":[{"idx":1,"windows":[3000000000]}],)"
	       R"("frames":[{"id":1,"kind":"leaf","focused":true}],)"
	       R"("bsp_nodes":[{"id":1,"kind":"leaf","window_id":3000000000}],)"
	       R"("split_nodes":[{"id":2,"kind":"leaf"}],"master_count":1,"master_split_ratio":0.5,)"
	       R"("viewport":{"target_x":1,"current_x":0.5,"target_y":2,"current_y":1.5}}]},)"
	       R"("outputs":[{"id":1,"name":"DP-1","connected":true,"is_primary":true,)"
	       R"("refresh_rate":60000,"physical_width":600,"physical_height":340,)"
	       R"("scale":1,"transform":"Normal","geometry":{"x":0,"y":0,"width":1920,"height":1080}}],)"
	       R"("windows":[{"id":3000000000,"pid":1234,"parent_id":null,"title":"Terminal",)"
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

// Qt test macros are intentionally instance-oriented and confuse generic clang-tidy checks.
// NOLINTBEGIN(misc-include-cleaner, misc-const-correctness, misc-use-internal-linkage, readability-convert-member-functions-to-static)
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
					} else if (requestName == "commands" && this->holdCommandReplies) {
						this->pendingCommandClients.push_back(client);
						return;
					} else if (requestName == "hang") {
						return;
					} else {
						client->write(handleRequest(request));
					}
					client->flush();
				});
			}
		});
	}

	void typedHelpersDoNotWaitForCommandCatalog() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY(ipc->bindableConnected().value());
		QTRY_COMPARE(this->pendingCommandClients.size(), 1);
		QVERIFY(ipc->commandsCatalog().value("commands").toList().isEmpty());

		QSignalSpy spy(ipc, &TriadIpc::requestFinished);
		auto requestId = ipc->spawn(QStringList({"sh", "-lc", "true"}));
		QVERIFY(requestId > 0);
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), requestId);
		QCOMPARE(spy.at(0).at(1).toBool(), true);
		QCOMPARE(this->lastRequestPayload("action").value("action").toString(), QString("spawn"));

		this->releaseCommandReplies();
	}

	void loadsInitialState() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY(ipc->bindableConnected().value());
		QTRY_COMPARE(ipc->workspaces()->valueList().size(), 1);
		QTRY_COMPARE(ipc->outputs()->valueList().size(), 1);
		QTRY_COMPARE(ipc->windows()->valueList().size(), 1);
		QTRY_VERIFY(ipc->commandsCatalog().value("commands").toList().size() >= 40);

		QCOMPARE(ipc->bindableFocusedWorkspace().value()->bindableTagId().value(), 1);
		QCOMPARE(ipc->bindableActiveTag().value(), 1);
		QCOMPARE(ipc->bindableActiveWorkspaceIndex().value(), 1);
		QCOMPARE(ipc->bindableOverviewOpen().value(), true);
		QCOMPARE(ipc->bindableOverviewSelectedWindowId().value(), HIGH_WINDOW_ID);
		QCOMPARE(ipc->layouts().size(), 1);
		QCOMPARE(ipc->layoutCycle(), QStringList({"scroller", "grid"}));
		QCOMPARE(ipc->layoutCycleEntries().size(), 1);

		auto* output = ipc->bindableFocusedOutput().value();
		QCOMPARE(output->bindableName().value(), QString("DP-1"));
		QCOMPARE(output->bindablePhysicalWidth().value(), 600);
		QCOMPARE(output->bindablePhysicalHeight().value(), 340);

		auto* window = ipc->bindableFocusedWindow().value();
		QCOMPARE(window->bindableId().value(), HIGH_WINDOW_ID);
		QCOMPARE(window->bindableTitle().value(), QString("Terminal"));
		QCOMPARE(window->bindableParentId().value(), 0U);
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
		QVERIFY(ipc->commandsCatalog().value("commands").toList().size() >= 40);

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

		auto pointerId = ipc->dispatchPointerBinding("Super+middle");
		QTRY_COMPARE(spy.size(), 4);
		QCOMPARE(spy.at(3).at(0).toInt(), pointerId);
		QCOMPARE(
		    this->lastRequestPayload("dispatch-binding").value("kind").toString(),
		    QString("pointer")
		);

		auto keyHelperId = ipc->dispatchKeyBinding("Super+Return");
		QTRY_COMPARE(spy.size(), 5);
		QCOMPARE(spy.at(4).at(0).toInt(), keyHelperId);
		QCOMPARE(this->lastRequestPayload("dispatch-binding").value("kind").toString(), QString("key"));

		auto axisHelperId = ipc->dispatchAxisBinding("Super+wheel-down", -2);
		QTRY_COMPARE(spy.size(), 6);
		QCOMPARE(spy.at(5).at(0).toInt(), axisHelperId);
		QCOMPARE(this->lastRequestPayload("dispatch-binding").value("ticks").toInt(), -2);

		auto gestureHelperId = ipc->dispatchGestureBinding("Super+swipe-right", 4);
		QTRY_COMPARE(spy.size(), 7);
		QCOMPARE(spy.at(6).at(0).toInt(), gestureHelperId);
		QCOMPARE(this->lastRequestPayload("dispatch-binding").value("fingers").toInt(), 4);
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

	void refreshesCommandCatalogOnReconnect() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY(ipc->bindableConnected().value());
		QVERIFY(this->eventClient != nullptr);

		auto streamBefore = this->requestCount("event-stream");
		auto commandsBefore = this->requestCount("commands");
		this->eventClient->disconnectFromServer();

		QTRY_VERIFY(this->requestCount("event-stream") > streamBefore);
		QTRY_VERIFY(this->requestCount("commands") > commandsBefore);
		QVERIFY(ipc->commandsCatalog().value("commands").toList().size() >= 40);
	}

	void validatesCommandCatalogActions() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY(ipc->hasCommand("focus-window"));
		QVERIFY(ipc->hasCommand("kill-window"));
		QCOMPARE(ipc->commandSpec("kill-window").value("name").toString(), QString("close-window"));

		QVERIFY(ipc->validateAction("focus-window", {{"id", HIGH_WINDOW_ID}}));
		QVERIFY(!ipc->validateAction("focus-window", {{"id", QString("bad")}}));
		QVERIFY(!ipc->validateAction("focus-window", {{"id", 0}}));
		QVERIFY(!ipc->validateAction("focus-window", {{"id", -1}}));
		QVERIFY(!ipc->validateAction("focus-window", {{"id", 7.5}}));
		QVERIFY(ipc->validateAction(
		    "move-window-to-tag",
		    {{"id", HIGH_WINDOW_ID}, {"tag", 2U}, {"follow", true}}
		));
		QVERIFY(ipc->validateAction(
		    "move-window-to-workspace",
		    {{"id", HIGH_WINDOW_ID}, {"workspace_idx", 2U}}
		));
		QVERIFY(ipc->validateAction("set-window-floating", {{"id", HIGH_WINDOW_ID}, {"value", false}}));
		QVERIFY(
		    ipc->validateAction("set-layout-for-workspace", {{"tag", 2U}, {"layout", QString("grid")}})
		);
		QVERIFY(ipc->validateAction("layout-custom", {{"name", QString("notion")}}));
		QVERIFY(!ipc->validateAction("layout-custom", {{"name", QString()}}));
		QVERIFY(ipc->validateAction("power-off-monitor", {{"output", QString("DP-1")}}));
		QVERIFY(!ipc->validateAction("power-off-monitor", {{"output", QString()}}));
		QVERIFY(ipc->validateAction("adjust-master-ratio", {{"delta", -0.05}}));
		QVERIFY(ipc->validateAction("master-ratio", {{"value", 0.5}}));
		QVERIFY(ipc->validateAction("set-column-width", {{"width", 0.5}}));
		QVERIFY(ipc->validateAction("master-count", {{"count", 2}}));
		QVERIFY(!ipc->validateAction("master-count", {{"count", 2.5}}));
		QVERIFY(ipc->validateAction("adjust-master-count", {{"delta", -1}}));
		QVERIFY(ipc->validateAction("switch-proportion-preset", {}));
		QVERIFY(ipc->validateAction("switch-proportion-preset", {{"delta", -1}}));
		QVERIFY(ipc->validateAction("move-floating", {{"dx", 12}, {"dy", -34}}));
		QVERIFY(ipc->validateAction("resize-floating", {{"dw", 12}, {"dh", -34}}));
		QVERIFY(ipc->validateAction(
		    "recent-window-next",
		    {{"scope", QString("output")}, {"filter", QString("app-id")}}
		));
		QVERIFY(ipc->validateAction("recent-window-scope", {{"scope", QString("workspace")}}));
		QVERIFY(ipc->validateAction("spawn", {{"argv", QStringList({"kitty"})}}));
		QVERIFY(!ipc->validateAction("spawn", {{"argv", QStringList()}}));
		QVERIFY(!ipc->validateAction("spawn", {{"argv", QVariantList({QString("sh"), 1})}}));
		QVERIFY(ipc->validateAction("warp-pointer", {{"x", 12}, {"y", 34}}));
		QVERIFY(!ipc->validateAction("warp-pointer", {{"x", 12.5}, {"y", 34}}));
		QVERIFY(ipc->validateAction("switch-keyboard-layout", {{"layout", QString("next")}}));
		QVERIFY(ipc->validateAction("switch-keyboard-layout", {{"layout", 1}}));
		QVERIFY(!ipc->validateAction("switch-keyboard-layout", {{"layout", 1.5}}));
		QVERIFY(ipc->validateAction(
		    "split-tree-layout-cycle",
		    {{"argv", QStringList({"splith", "stacking"})}}
		));
		QVERIFY(ipc->validateAction("frame-resize-left", {}));
		QVERIFY(ipc->validateAction("frame-resize-left", {{"delta", 0.05}}));
		QVERIFY(
		    ipc->validateAction("screenshot", {{"path", QString("/tmp/a.png")}, {"show_pointer", true}})
		);
		QVERIFY(
		    !ipc->validateAction("screenshot", {{"write_to_disk", false}, {"copy_to_clipboard", false}})
		);
		QVERIFY(ipc->validateAction("future-command", {{"value", QString("daemon-validated")}}));

		QSignalSpy spy(ipc, &TriadIpc::requestFinished);
		QCOMPARE(ipc->sendValidatedAction("focus-window", {{"id", QString("bad")}}), -1);
		QCOMPARE(spy.size(), 0);
		auto requestId = ipc->sendValidatedAction("focus-window", {{"id", HIGH_WINDOW_ID}});
		QTRY_COMPARE(spy.size(), 1);
		QCOMPARE(spy.at(0).at(0).toInt(), requestId);
		QCOMPARE(
		    this->lastRequestPayload("action").value("id").toDouble(),
		    static_cast<double>(HIGH_WINDOW_ID)
		);

		auto futureId =
		    ipc->sendValidatedAction("future-command", {{"value", QString("daemon-validated")}});
		QTRY_COMPARE(spy.size(), 2);
		QCOMPARE(spy.at(1).at(0).toInt(), futureId);
		QCOMPARE(
		    this->lastRequestPayload("action").value("action").toString(),
		    QString("future-command")
		);
	}

	void emitsTimeoutForHungRequest() {
		auto* ipc = TriadIpc::instance();
		QSignalSpy spy(ipc, &TriadIpc::requestFinished);

		auto requestId = ipc->sendRequest("hang");
		QTRY_COMPARE_WITH_TIMEOUT(spy.size(), 1, 1000);
		QCOMPARE(spy.at(0).at(0).toInt(), requestId);
		QCOMPARE(spy.at(0).at(1).toBool(), false);
		QCOMPARE(spy.at(0).at(3).toString(), QString("timeout"));
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
	[[nodiscard]] qsizetype requestCount(const QString& name) const {
		return this->requestNames.count(name);
	}

	void releaseCommandReplies() {
		this->holdCommandReplies = false;
		for (auto* client: std::as_const(this->pendingCommandClients)) {
			client->write(commandsReply());
			client->flush();
		}
		this->pendingCommandClients.clear();
	}

	[[nodiscard]] QJsonObject lastRequestPayload(const QString& name) const {
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
	bool holdCommandReplies = true;
	QList<QLocalSocket*> pendingCommandClients;
};
// NOLINTEND(misc-include-cleaner, misc-const-correctness, misc-use-internal-linkage, readability-convert-member-functions-to-static)

QTEST_MAIN(TestTriadIpc);
#include "triad_ipc.moc"
