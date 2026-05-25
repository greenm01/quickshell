#include "connection.hpp"
#include <algorithm>
#include <memory>
#include <utility>

#include <qcontainerfwd.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qlocalsocket.h>
#include <qlogging.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <qtenvironmentvariables.h>
#include <qtimer.h>
#include <qtypes.h>
#include <qvariant.h>

#include "../core/logcat.hpp"
#include "output.hpp"
#include "window.hpp"
#include "workspace.hpp"

namespace qs::triad {

namespace {
QS_LOGGING_CATEGORY(logTriadIpc, "quickshell.triad.ipc", QtWarningMsg);
QS_LOGGING_CATEGORY(logTriadIpcEvents, "quickshell.triad.ipc.events", QtWarningMsg);

constexpr auto TriadIpcVersion = 1;

QJsonObject triadPayload(const QString& request) {
	return {{"version", TriadIpcVersion}, {"request", request}};
}

QJsonObject triadRoot(QJsonObject payload) {
	payload.insert("version", TriadIpcVersion);
	return {{"triad", payload}};
}

QByteArray encoded(QJsonObject payload) {
	return QJsonDocument(triadRoot(std::move(payload))).toJson(QJsonDocument::Compact) + '\n';
}

qint32 intOrInvalid(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	return value.isValid() && !value.isNull() ? value.toInt() : -1;
}

QStringList stringListFromVariant(const QVariant& value) {
	auto result = QStringList();
	for (const auto& item: value.toList()) {
		result.push_back(item.toString());
	}
	return result;
}

QStringList stringListFromJson(const QJsonValue& value) {
	auto result = QStringList();
	for (const auto& item: value.toArray()) {
		result.push_back(item.toString());
	}
	return result;
}

qint32 intOrInvalid(const QJsonObject& object, const QString& key) {
	auto value = object.value(key);
	return value.isUndefined() || value.isNull() ? -1 : value.toInt(-1);
}
} // namespace

TriadIpc::TriadIpc() {
	this->reconnectTimer.setInterval(1000);
	this->reconnectTimer.setSingleShot(true);

	// clang-format off
	QObject::connect(&this->eventSocket, &QLocalSocket::connected, this, &TriadIpc::eventSocketConnected);
	QObject::connect(&this->eventSocket, &QLocalSocket::errorOccurred, this, &TriadIpc::eventSocketError);
	QObject::connect(&this->eventSocket, &QLocalSocket::stateChanged, this, &TriadIpc::eventSocketStateChanged);
	QObject::connect(&this->eventSocket, &QLocalSocket::readyRead, this, &TriadIpc::eventSocketReady);
	QObject::connect(&this->reconnectTimer, &QTimer::timeout, this, &TriadIpc::reconnect);
	// clang-format on

	this->setSocketPath(this->discoverSocketPath());
	this->connectEventStream();
}

TriadIpc* TriadIpc::instance() {
	static TriadIpc* instance = nullptr; // NOLINT

	if (instance == nullptr) {
		instance = new TriadIpc();
	}

	return instance;
}

QString TriadIpc::discoverSocketPath() const {
	auto socket = qEnvironmentVariable("TRIAD_SOCKET");
	if (!socket.isEmpty()) return socket;

	auto runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
	if (!runtimeDir.isEmpty()) {
		socket = QDir(runtimeDir).filePath("triad.sock");
		if (QFileInfo::exists(socket)) return socket;
	}

	return "/tmp/triad.sock";
}

void TriadIpc::setSocketPath(const QString& path) {
	if (this->mSocketPath == path) return;
	this->mSocketPath = path;
	emit this->socketPathChanged();
}

void TriadIpc::connectEventStream() {
	if (this->connecting || this->eventSocket.state() != QLocalSocket::UnconnectedState) return;

	this->setSocketPath(this->discoverSocketPath());
	if (this->mSocketPath.isEmpty()) {
		this->reconnectTimer.start();
		return;
	}

	this->connecting = true;
	qCDebug(logTriadIpc) << "Connecting to Triad IPC socket" << this->mSocketPath;
	this->eventSocket.connectToServer(this->mSocketPath, QLocalSocket::ReadWrite);
}

void TriadIpc::reconnect() {
	this->connecting = false;
	this->connectEventStream();
}

void TriadIpc::eventSocketError(QLocalSocket::LocalSocketError error) {
	qCWarning(logTriadIpc) << "Triad IPC socket error:" << error;
	this->connecting = false;
	if (!this->bConnected.value()) this->reconnectTimer.start();
}

void TriadIpc::eventSocketConnected() {
	this->connecting = false;
	this->eventReader.setDevice(&this->eventSocket);
	this->bConnected = true;

	auto payload = triadPayload("event-stream");
	payload.insert("events", QJsonArray({"state", "layout", "window"}));
	this->eventSocket.write(encoded(payload));
	this->eventSocket.flush();
	qCInfo(logTriadIpc) << "Triad IPC event stream connected.";
}

void TriadIpc::eventSocketStateChanged(QLocalSocket::LocalSocketState state) {
	if (state == QLocalSocket::UnconnectedState) {
		this->connecting = false;
		this->eventReader.reset();
		if (this->bConnected.value()) {
			qCWarning(logTriadIpc) << "Triad IPC event stream disconnected.";
			this->bConnected = false;
		}
		this->reconnectTimer.start();
	}
}

void TriadIpc::eventSocketReady() {
	while (true) {
		this->eventReader.startTransaction();
		auto line = this->eventReader.readUntil('\n');
		if (!this->eventReader.commitTransaction()) return;
		if (line.endsWith('\n')) line.chop(1);
		this->handleLine(line);
	}
}

qint32 TriadIpc::makeRequest(const QJsonObject& payload, RequestCallback callback) {
	auto requestId = this->nextRequestId++;
	auto* socket = new QLocalSocket(this);
	auto* reader = new StreamReader();
	reader->setDevice(socket);

	auto cleaned = std::make_shared<bool>(false);
	auto cleanup = [socket, reader, cleaned]() {
		if (*cleaned) return;
		*cleaned = true;
		delete reader;
		socket->deleteLater();
	};

	QObject::connect(socket, &QLocalSocket::connected, this, [socket, payload]() {
		socket->write(encoded(payload));
		socket->flush();
	});

	QObject::connect(
	    socket,
	    &QLocalSocket::readyRead,
	    this,
	    [this, socket, reader, callback, cleanup]() {
		    Q_UNUSED(socket);
		    reader->startTransaction();
		    auto line = reader->readUntil('\n');
		    if (!reader->commitTransaction()) return;
		    if (line.endsWith('\n')) line.chop(1);

		    auto error = QJsonParseError();
		    auto root = QJsonDocument::fromJson(line, &error).object();
		    if (error.error != QJsonParseError::NoError) {
			    qCWarning(logTriadIpc) << "Invalid Triad IPC response:" << error.errorString();
			    if (callback) callback(false, {}, error.errorString());
			    cleanup();
			    return;
		    }

		    auto ok = root.value("ok").toBool();
		    if (ok) this->handleLine(line);
		    if (callback) callback(ok, root.value("triad").toObject(), root.value("error").toString());
		    cleanup();
	    }
	);

	QObject::connect(
	    socket,
	    &QLocalSocket::errorOccurred,
	    this,
	    [payload, callback, cleanup](QLocalSocket::LocalSocketError error) {
		    qCWarning(logTriadIpc) << "Error making Triad request:" << error << "request:" << payload;
		    if (callback) callback(false, {}, QStringLiteral("socket error"));
		    cleanup();
	    }
	);

	socket->connectToServer(this->mSocketPath);
	return requestId;
}

void TriadIpc::refresh() { this->makeRequest(triadPayload("state")); }

void TriadIpc::refreshLayout() { this->makeRequest(triadPayload("layout-state")); }

void TriadIpc::refreshWindows() { this->makeRequest(triadPayload("windows")); }

qint32 TriadIpc::sendRequest(const QString& request, const QVariantMap& payload) {
	auto object = QJsonObject::fromVariantMap(payload);
	object.insert("request", request);
	auto requestIdPtr = std::make_shared<qint32>(-1);
	*requestIdPtr = this->makeRequest(
	    object,
	    [this, requestIdPtr](bool ok, const QJsonObject& triad, const QString& error) {
		    emit this->requestFinished(*requestIdPtr, ok, triad.toVariantMap(), error);
	    }
	);
	return *requestIdPtr;
}

qint32 TriadIpc::sendAction(const QString& action, const QVariantMap& payload) {
	auto object = QJsonObject::fromVariantMap(payload);
	object.insert("request", "action");
	object.insert("action", action);
	auto requestIdPtr = std::make_shared<qint32>(-1);
	*requestIdPtr = this->makeRequest(
	    object,
	    [this, requestIdPtr](bool ok, const QJsonObject& triad, const QString& error) {
		    emit this->requestFinished(*requestIdPtr, ok, triad.toVariantMap(), error);
	    }
	);
	return *requestIdPtr;
}

void TriadIpc::dispatch(const QString& action, const QVariantMap& payload) {
	auto object = QJsonObject::fromVariantMap(payload);
	object.insert("request", "action");
	object.insert("action", action);
	this->makeRequest(object);
}

void TriadIpc::focusWorkspace(qint32 workspaceIndex) {
	this->dispatch("focus-workspace", {{"workspace_idx", workspaceIndex}});
}

void TriadIpc::focusTag(qint32 tagId) { this->dispatch("focus-tag", {{"tag", tagId}}); }

void TriadIpc::focusWindow(qint32 windowId) { this->dispatch("focus-window", {{"id", windowId}}); }

void TriadIpc::closeWindow(qint32 windowId) {
	if (windowId > 0) this->dispatch("close-window", {{"id", windowId}});
	else this->dispatch("close-window");
}

void TriadIpc::switchLayout() { this->makeRequest(triadPayload("switch-layout")); }

void TriadIpc::setLayout(const QString& layoutId, const QVariantMap& target) {
	auto payload = triadPayload("set-layout");
	payload.insert("layout", layoutId);
	if (!target.isEmpty()) payload.insert("target", QJsonObject::fromVariantMap(target));
	this->makeRequest(payload);
}

void TriadIpc::handleLine(const QByteArray& line) {
	if (line.trimmed().isEmpty()) return;

	auto error = QJsonParseError();
	auto root = QJsonDocument::fromJson(line, &error).object();
	if (error.error != QJsonParseError::NoError) {
		qCWarning(logTriadIpc) << "Invalid Triad IPC JSON:" << error.errorString();
		return;
	}

	if (!root.contains("triad")) return;
	this->handleTriadObject(root.value("triad").toObject());
}

void TriadIpc::handleTriadObject(const QJsonObject& triad) {
	auto eventName = triad.value("event").toString();
	if (!eventName.isEmpty()) {
		qCDebug(logTriadIpcEvents) << "Received Triad event:" << eventName;
		this->event.name = eventName;
		this->event.data = triad.toVariantMap();
		emit this->rawEvent(&this->event);

		if (eventName == "state-changed") this->handleState(triad.value("state").toObject());
		else if (eventName == "layout-state-changed") {
			this->handleLayoutState(triad.value("state").toObject());
		} else if (eventName == "window-changed") {
			this->handleWindow(triad.value("window").toObject());
			this->updateDerivedState();
		}
		return;
	}

	auto type = triad.value("type").toString();
	if (type == "state") this->handleState(triad.value("state").toObject());
	else if (type == "layout-state") this->handleLayoutState(triad.value("state").toObject());
	else if (type == "workspaces") this->handleWorkspaces(triad.value("workspaces").toArray());
	else if (type == "outputs") this->handleOutputs(triad.value("outputs").toArray());
	else if (type == "windows") {
		this->handleWindows(triad.value("windows").toArray());
		this->updateDerivedState();
	} else if (type == "focused-window") {
		if (triad.value("window").isObject()) this->handleWindow(triad.value("window").toObject());
		else this->bFocusedWindow = nullptr;
		this->updateDerivedState();
	} else if (type == "capabilities") {
		this->mCapabilities = triad.value("capabilities").toObject().toVariantMap();
		emit this->capabilitiesChanged();
	} else if (type == "overview-state") {
		this->handleOverview(triad.value("overview").toObject());
	} else if (type == "keyboard-layouts") {
		auto layouts = triad.value("keyboard_layouts").toObject().toVariantMap();
		this->mKeyboardLayouts = stringListFromVariant(layouts.value("names"));
		this->bCurrentKeyboardLayoutIndex = intOrInvalid(layouts, "current_idx");
		emit this->keyboardLayoutsChanged();
	} else if (type == "commands") {
		this->mCommandsCatalog = triad.value("catalog").toObject().toVariantMap();
		emit this->commandsCatalogChanged();
	}
}

void TriadIpc::handleState(const QJsonObject& state) {
	this->mCapabilities = state.value("capabilities").toObject().toVariantMap();
	emit this->capabilitiesChanged();

	this->handleOverview(state.value("overview").toObject());

	this->mKeyboardLayouts = stringListFromVariant(state.value("keyboard_layouts").toVariant());
	this->bCurrentKeyboardLayoutIndex = state.value("current_keyboard_layout_idx").toInt(-1);
	emit this->keyboardLayoutsChanged();

	this->handleLayoutState(state.value("layout").toObject());
	this->handleOutputs(state.value("outputs").toArray());
	this->handleWindows(state.value("windows").toArray());
	this->updateDerivedState();
}

void TriadIpc::handleLayoutState(const QJsonObject& state) {
	this->mLayouts = state.value("layouts").toVariant().toList();
	emit this->layoutsChanged();
	this->mLayoutCycle = stringListFromJson(state.value("layout_cycle"));
	emit this->layoutCycleChanged();
	this->mLayoutCycleEntries = state.value("layout_cycle_entries").toVariant().toList();
	emit this->layoutCycleEntriesChanged();
	this->bActiveTag = intOrInvalid(state, "active_tag");
	this->bActiveWorkspaceIndex = intOrInvalid(state, "active_workspace_idx");
	this->handleWorkspaces(state.value("workspaces").toArray());
	this->updateDerivedState();
}

void TriadIpc::handleOverview(const QJsonObject& overview) {
	this->bOverviewOpen = overview.value("is_open").toBool();
	this->bOverviewSelectedWindowId = intOrInvalid(overview, "selected_window_id");
}

void TriadIpc::handleWorkspaces(const QJsonArray& workspaces) {
	auto newValues = QList<TriadWorkspace*>();
	auto seen = QHash<qint32, bool>();

	for (const auto& value: workspaces) {
		auto object = value.toObject().toVariantMap();
		auto tagId = intOrInvalid(object, "tag_id");
		if (tagId < 0) continue;

		auto* workspace = this->workspacesByTag.value(tagId);
		if (workspace == nullptr) {
			workspace = new TriadWorkspace(this);
			this->workspacesByTag.insert(tagId, workspace);
		}

		workspace->updateFromObject(object);
		newValues.push_back(workspace);
		seen.insert(tagId, true);
	}

	auto oldValues = this->mWorkspaces.valueList();
	this->mWorkspaces.diffUpdate(newValues);

	for (auto* workspace: oldValues) {
		auto tagId = workspace->bindableTagId().value();
		if (!seen.contains(tagId)) {
			this->workspacesByTag.remove(tagId);
			delete workspace;
		}
	}
}

void TriadIpc::handleOutputs(const QJsonArray& outputs) {
	auto newValues = QList<TriadOutput*>();
	auto seen = QHash<qint32, bool>();
	this->outputsByName.clear();

	for (const auto& value: outputs) {
		auto object = value.toObject().toVariantMap();
		auto id = intOrInvalid(object, "id");
		if (id < 0) continue;

		auto* output = this->outputsById.value(id);
		if (output == nullptr) {
			output = new TriadOutput(this);
			this->outputsById.insert(id, output);
		}

		output->updateFromObject(object);
		this->outputsByName.insert(output->bindableName().value(), output);
		newValues.push_back(output);
		seen.insert(id, true);
	}

	auto oldValues = this->mOutputs.valueList();
	this->mOutputs.diffUpdate(newValues);

	for (auto* output: oldValues) {
		auto id = output->bindableId().value();
		if (!seen.contains(id)) {
			this->outputsById.remove(id);
			delete output;
		}
	}
}

void TriadIpc::handleWindows(const QJsonArray& windows) {
	auto newValues = QList<TriadWindow*>();
	auto seen = QHash<qint32, bool>();

	for (const auto& value: windows) {
		auto object = value.toObject();
		auto id = object.value("id").toInt(-1);
		if (id < 0) continue;

		auto* window = this->windowsById.value(id);
		if (window == nullptr) {
			window = new TriadWindow(this);
			this->windowsById.insert(id, window);
		}

		window->updateFromObject(object.toVariantMap());
		newValues.push_back(window);
		seen.insert(id, true);
	}

	auto oldValues = this->mWindows.valueList();
	this->mWindows.diffUpdate(newValues);

	for (auto* window: oldValues) {
		auto id = window->bindableId().value();
		if (!seen.contains(id)) {
			this->windowsById.remove(id);
			delete window;
		}
	}
}

void TriadIpc::handleWindow(const QJsonObject& object) {
	if (object.isEmpty()) return;
	auto id = object.value("id").toInt(-1);
	if (id < 0) return;

	auto* window = this->windowsById.value(id);
	if (window == nullptr) {
		window = new TriadWindow(this);
		this->windowsById.insert(id, window);
		this->mWindows.insertObject(window);
	}

	window->updateFromObject(object.toVariantMap());
}

void TriadIpc::updateDerivedState() {
	TriadWorkspace* focusedWorkspace = nullptr;
	for (auto* workspace: this->mWorkspaces.valueList()) {
		auto* focusedWindow = this->windowById(workspace->bindableFocusedWindowId().value());
		workspace->setFocusedWindow(focusedWindow);
		if (workspace->bindableActive().value()) focusedWorkspace = workspace;
	}

	this->bFocusedWorkspace = focusedWorkspace;

	TriadWindow* focusedWindow = nullptr;
	for (auto* window: this->mWindows.valueList()) {
		if (window->bindableFocused().value()) {
			focusedWindow = window;
			break;
		}
	}
	this->bFocusedWindow = focusedWindow;

	TriadOutput* focusedOutput = nullptr;
	for (auto* output: this->mOutputs.valueList()) {
		output->setActiveWorkspace(nullptr);
		output->setFocused(false);
	}

	for (auto* workspace: this->mWorkspaces.valueList()) {
		auto* output = this->outputByName(workspace->bindableOutputName().value());
		if (output != nullptr && workspace->bindableOutputVisible().value()) {
			output->setActiveWorkspace(workspace);
		}
	}

	if (focusedWorkspace != nullptr) {
		focusedOutput = this->outputByName(focusedWorkspace->bindableOutputName().value());
		if (focusedOutput != nullptr) focusedOutput->setFocused(true);
	}
	this->bFocusedOutput = focusedOutput;
}

TriadWorkspace* TriadIpc::workspaceByTag(qint32 tagId) const {
	return this->workspacesByTag.value(tagId);
}

TriadWorkspace* TriadIpc::workspaceByIndex(qint32 workspaceIndex) const {
	for (auto* workspace: this->mWorkspaces.valueList()) {
		if (workspace->bindableWorkspaceIndex().value() == workspaceIndex) return workspace;
	}
	return nullptr;
}

TriadOutput* TriadIpc::outputByName(const QString& name) const {
	return this->outputsByName.value(name);
}

TriadWindow* TriadIpc::windowById(qint32 id) const { return this->windowsById.value(id); }

} // namespace qs::triad
