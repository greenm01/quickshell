#include "connection.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
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
#include <qmetatype.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qstring.h>
#include <qtenvironmentvariables.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <qtpreprocessorsupport.h>
#include <qtypes.h>
#include <qvariant.h>

#include "../../core/logcat.hpp"
#include "output.hpp"
#include "window.hpp"
#include "workspace.hpp"

namespace qs::triad {

namespace {
QS_LOGGING_CATEGORY(logTriadIpc, "quickshell.triad.ipc", QtWarningMsg);
QS_LOGGING_CATEGORY(logTriadIpcEvents, "quickshell.triad.ipc.events", QtWarningMsg);

constexpr auto TRIAD_IPC_VERSION = 1;
#ifdef QS_TEST
constexpr auto REQUEST_TIMEOUT_MS = 250;
#else
constexpr auto REQUEST_TIMEOUT_MS = 5000;
#endif

QJsonObject triadPayload(const QString& request) {
	return {{"version", TRIAD_IPC_VERSION}, {"request", request}};
}

QJsonObject triadRoot(QJsonObject payload) {
	payload.insert("version", TRIAD_IPC_VERSION);
	return {{"triad", payload}};
}

QByteArray encoded(QJsonObject payload) {
	return QJsonDocument(triadRoot(std::move(payload))).toJson(QJsonDocument::Compact) + '\n';
}

qint32 intOrInvalid(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	return value.isValid() && !value.isNull() ? value.toInt() : -1;
}

quint32 uintOrNone(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	if (!value.isValid() || value.isNull()) return 0;
	if (!value.canConvert<qulonglong>()) return 0;

	auto ok = false;
	auto parsed = value.toULongLong(&ok);
	if (!ok || parsed > std::numeric_limits<quint32>::max()) return 0;
	return static_cast<quint32>(parsed);
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

bool isIntegral(const QVariant& value) {
	auto type = value.metaType().id();
	return type == QMetaType::Int || type == QMetaType::UInt || type == QMetaType::LongLong
	    || type == QMetaType::ULongLong;
}

bool isNumeric(const QVariant& value) {
	return isIntegral(value) || value.metaType().id() == QMetaType::Double;
}

bool isPositiveUint(const QVariant& value) {
	if (value.metaType().id() == QMetaType::Double) {
		auto parsed = value.toDouble();
		return std::isfinite(parsed) && std::trunc(parsed) == parsed && parsed > 0
		    && parsed <= std::numeric_limits<quint32>::max();
	}
	if (!isIntegral(value)) return false;
	auto type = value.metaType().id();
	if (type == QMetaType::Int || type == QMetaType::LongLong) {
		auto ok = false;
		auto parsed = value.toLongLong(&ok);
		return ok && parsed > 0 && std::cmp_less_equal(parsed, std::numeric_limits<quint32>::max());
	}

	auto ok = false;
	auto parsed = value.toULongLong(&ok);
	return ok && parsed > 0 && parsed <= std::numeric_limits<quint32>::max();
}

bool isInteger(const QVariant& value) {
	if (value.metaType().id() == QMetaType::Double) {
		auto parsed = value.toDouble();
		return std::isfinite(parsed) && std::trunc(parsed) == parsed;
	}
	return isIntegral(value);
}

bool isString(const QVariant& value) { return value.metaType().id() == QMetaType::QString; }

bool isNonEmptyString(const QVariant& value) {
	return isString(value) && !value.toString().isEmpty();
}

bool isBool(const QVariant& value) { return value.metaType().id() == QMetaType::Bool; }

bool isNonEmptyStringList(const QVariant& value) {
	if (value.metaType().id() == QMetaType::QStringList) return !value.toStringList().isEmpty();
	if (value.metaType().id() != QMetaType::QVariantList) return false;

	auto list = value.toList();
	if (list.isEmpty()) return false;
	return std::ranges::all_of(list, [](const QVariant& item) { return isString(item); });
}

QString discoverSocketPath() {
	auto socket = qEnvironmentVariable("TRIAD_SOCKET");
	if (!socket.isEmpty()) return socket;

	auto runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
	if (!runtimeDir.isEmpty()) {
		socket = QDir(runtimeDir).filePath("triad.sock");
		if (QFileInfo::exists(socket)) return socket;
	}

	return "/tmp/triad.sock";
}

bool payloadMatchesShape(
    const QString& commandName,
    const QString& shape,
    const QVariantMap& payload
) {
	auto hasUint = [&payload](const QString& key) { return isPositiveUint(payload.value(key)); };
	auto hasInt = [&payload](const QString& key) { return isInteger(payload.value(key)); };
	auto hasNumber = [&payload](const QString& key) { return isNumeric(payload.value(key)); };
	auto hasNonEmptyString = [&payload](const QString& key) {
		return isNonEmptyString(payload.value(key));
	};
	auto optionalUint = [&payload, hasUint](const QString& key) {
		return !payload.contains(key) || hasUint(key);
	};
	auto optionalInt = [&payload, hasInt](const QString& key) {
		return !payload.contains(key) || hasInt(key);
	};
	auto optionalDouble = [&payload, hasNumber](const QString& key) {
		return !payload.contains(key) || hasNumber(key);
	};
	auto optionalString = [&payload](const QString& key) {
		return !payload.contains(key) || isString(payload.value(key));
	};
	auto optionalBool = [&payload](const QString& key) {
		return !payload.contains(key) || isBool(payload.value(key));
	};

	if (shape == "none") return true;
	if (shape == "optional-window-id") return optionalUint("id");
	if (shape == "required-window-id") return hasUint("id");
	if (shape == "window-tag-follow")
		return hasUint("id") && hasUint("tag") && optionalBool("follow");
	if (shape == "window-workspace-follow") {
		return hasUint("id") && hasUint("workspace_idx") && optionalBool("follow");
	}
	if (shape == "window-bool") return hasUint("id") && isBool(payload.value("value"));
	if (shape == "tag-layout") return hasUint("tag") && hasNonEmptyString("layout");
	if (shape == "required-tag") return hasUint("tag");
	if (shape == "required-workspace-idx") return hasUint("workspace_idx");
	if (shape == "required-name") return hasNonEmptyString("name");
	if (shape == "required-output") return hasNonEmptyString("output");
	if (shape == "required-float-delta") return hasNumber("delta");
	if (shape == "required-int-delta") return hasInt("delta");
	if (shape == "optional-float-delta") return optionalDouble("delta");
	if (shape == "optional-int-delta") return optionalInt("delta");
	if (shape == "required-float-value") {
		return hasNumber("value") || (commandName == "set-column-width" && hasNumber("width"));
	}
	if (shape == "required-int-count") return hasInt("count");
	if (shape == "move-delta") return hasInt("dx") && hasInt("dy");
	if (shape == "resize-delta") return hasInt("dw") && hasInt("dh");
	if (shape == "recent-advance") return optionalString("scope") && optionalString("filter");
	if (shape == "recent-scope") return hasNonEmptyString("scope");
	if (shape == "spawn-argv" || shape == "split-tree-mode-list") {
		return isNonEmptyStringList(payload.value("argv"));
	}
	if (shape == "warp-pointer") return hasInt("x") && hasInt("y");
	if (shape == "screenshot") {
		if (!optionalString("path") || !optionalBool("show_pointer") || !optionalBool("write_to_disk")
		    || !optionalBool("copy_to_clipboard"))
		{
			return false;
		}

		auto writeToDisk =
		    !payload.contains("write_to_disk") || payload.value("write_to_disk").toBool();
		auto copyToClipboard =
		    !payload.contains("copy_to_clipboard") || payload.value("copy_to_clipboard").toBool();
		return writeToDisk || copyToClipboard;
	}
	if (shape == "keyboard-layout-target") {
		if (!payload.contains("layout")) return true;
		auto layout = payload.value("layout");
		return isString(layout) || isInteger(layout);
	}

	qCWarning(logTriadIpc) << "Unknown Triad command arg_shape" << shape << "for" << commandName
	                       << "- allowing daemon-side validation.";
	return true;
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

	this->setSocketPath(discoverSocketPath());
	this->connectEventStream();
}

TriadIpc* TriadIpc::instance() {
	static TriadIpc* instance = nullptr; // NOLINT

	if (instance == nullptr) {
		instance = new TriadIpc();
	}

	return instance;
}

void TriadIpc::setSocketPath(const QString& path) {
	if (this->mSocketPath == path) return;
	this->mSocketPath = path;
	emit this->socketPathChanged();
}

void TriadIpc::connectEventStream() {
	if (this->connecting || this->eventSocket.state() != QLocalSocket::UnconnectedState) return;

	this->setSocketPath(discoverSocketPath());
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
	this->autoRefreshCommands();
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
	auto* timer = new QTimer(this);
	reader->setDevice(socket);
	timer->setSingleShot(true);
	timer->setInterval(REQUEST_TIMEOUT_MS);

	auto completed = std::make_shared<bool>(false);
	auto cleanup = [socket, reader, timer, completed]() {
		if (*completed) return false;
		*completed = true;
		timer->stop();
		timer->deleteLater();
		delete reader;
		socket->deleteLater();
		return true;
	};
	auto complete = [requestId,
	                 callback = std::move(callback),
	                 cleanup](bool ok, const QJsonObject& triad, const QString& error) {
		if (!cleanup()) return;
		if (callback) callback(requestId, ok, triad, error);
	};

	QObject::connect(socket, &QLocalSocket::connected, this, [socket, payload]() {
		socket->write(encoded(payload));
		socket->flush();
	});

	QObject::connect(socket, &QLocalSocket::readyRead, this, [this, socket, reader, complete]() {
		Q_UNUSED(socket);
		reader->startTransaction();
		auto line = reader->readUntil('\n');
		if (!reader->commitTransaction()) return;
		if (line.endsWith('\n')) line.chop(1);

		auto error = QJsonParseError();
		auto root = QJsonDocument::fromJson(line, &error).object();
		if (error.error != QJsonParseError::NoError) {
			qCWarning(logTriadIpc) << "Invalid Triad IPC response:" << error.errorString();
			complete(false, {}, error.errorString());
			return;
		}

		auto ok = root.value("ok").toBool();
		if (ok) this->handleLine(line);
		complete(ok, root.value("triad").toObject(), root.value("error").toString());
	});

	QObject::connect(
	    socket,
	    &QLocalSocket::errorOccurred,
	    this,
	    [payload, complete](QLocalSocket::LocalSocketError error) {
		    qCWarning(logTriadIpc) << "Error making Triad request:" << error << "request:" << payload;
		    complete(false, {}, QStringLiteral("socket error"));
	    }
	);

	QObject::connect(timer, &QTimer::timeout, this, [payload, complete]() {
		qCWarning(logTriadIpc) << "Timed out making Triad request:" << payload;
		complete(false, {}, QStringLiteral("timeout"));
	});

	socket->connectToServer(this->mSocketPath);
	timer->start();
	return requestId;
}

qint32 TriadIpc::makeTrackedRequest(const QJsonObject& payload) {
	return this->makeRequest(
	    payload,
	    [this](qint32 requestId, bool ok, const QJsonObject& triad, const QString& error) {
		    emit this->requestFinished(requestId, ok, triad.toVariantMap(), error);
	    }
	);
}

qint32 TriadIpc::refresh() { return this->makeTrackedRequest(triadPayload("state")); }

qint32 TriadIpc::refreshLayout() { return this->makeTrackedRequest(triadPayload("layout-state")); }

qint32 TriadIpc::refreshWindows() { return this->makeTrackedRequest(triadPayload("windows")); }

qint32 TriadIpc::refreshCapabilities() {
	return this->makeTrackedRequest(triadPayload("capabilities"));
}

qint32 TriadIpc::refreshWorkspaces() {
	return this->makeTrackedRequest(triadPayload("workspaces"));
}

qint32 TriadIpc::refreshOutputs() { return this->makeTrackedRequest(triadPayload("outputs")); }

qint32 TriadIpc::refreshFocusedWindow() {
	return this->makeTrackedRequest(triadPayload("focused-window"));
}

qint32 TriadIpc::refreshOverview() {
	return this->makeTrackedRequest(triadPayload("overview-state"));
}

qint32 TriadIpc::refreshKeyboardLayouts() {
	return this->makeTrackedRequest(triadPayload("keyboard-layouts"));
}

qint32 TriadIpc::refreshCommands() { return this->makeTrackedRequest(triadPayload("commands")); }

qint32 TriadIpc::sendRequest(const QString& request, const QVariantMap& payload) {
	auto object = QJsonObject::fromVariantMap(payload);
	object.insert("request", request);
	return this->makeTrackedRequest(object);
}

qint32 TriadIpc::sendAction(const QString& action, const QVariantMap& payload) {
	auto object = QJsonObject::fromVariantMap(payload);
	object.insert("request", "action");
	object.insert("action", action);
	return this->makeTrackedRequest(object);
}

qint32 TriadIpc::dispatch(const QString& action, const QVariantMap& payload) {
	return this->sendAction(action, payload);
}

qint32 TriadIpc::dispatchBinding(const QString& kind, const QString& binding, qint32 amount) {
	auto normalizedKind = kind.toLower();
	auto object = QJsonObject();
	object.insert("request", "dispatch-binding");
	object.insert("kind", normalizedKind);
	object.insert("binding", binding);
	if (normalizedKind == "axis" || normalizedKind == "wheel" || normalizedKind == "scroll") {
		object.insert("ticks", amount);
	} else if (normalizedKind == "gesture") {
		object.insert("fingers", amount);
	}
	return this->makeTrackedRequest(object);
}

qint32 TriadIpc::dispatchKeyBinding(const QString& binding) {
	return this->dispatchBinding("key", binding);
}

qint32 TriadIpc::dispatchPointerBinding(const QString& binding) {
	return this->dispatchBinding("pointer", binding);
}

qint32 TriadIpc::dispatchAxisBinding(const QString& binding, qint32 ticks) {
	return this->dispatchBinding("axis", binding, ticks);
}

qint32 TriadIpc::dispatchGestureBinding(const QString& binding, qint32 fingers) {
	return this->dispatchBinding("gesture", binding, fingers);
}

qint32 TriadIpc::focusWorkspace(qint32 workspaceIndex) {
	return this->dispatch("focus-workspace", {{"workspace_idx", workspaceIndex}});
}

qint32 TriadIpc::focusTag(quint32 tagId) { return this->dispatch("focus-tag", {{"tag", tagId}}); }

qint32 TriadIpc::focusWindow(quint32 windowId) {
	return this->dispatch("focus-window", {{"id", windowId}});
}

qint32 TriadIpc::closeWindow(quint32 windowId) {
	if (windowId > 0) return this->dispatch("close-window", {{"id", windowId}});
	return this->dispatch("close-window");
}

qint32 TriadIpc::switchLayout() { return this->makeTrackedRequest(triadPayload("switch-layout")); }

qint32 TriadIpc::setLayout(const QString& layoutId, const QVariantMap& target) {
	auto payload = triadPayload("set-layout");
	payload.insert("layout", layoutId);
	if (!target.isEmpty()) payload.insert("target", QJsonObject::fromVariantMap(target));
	return this->makeTrackedRequest(payload);
}

qint32 TriadIpc::spawn(const QStringList& argv) {
	return this->dispatch("spawn", {{"argv", argv}});
}

qint32 TriadIpc::switchKeyboardLayout(const QVariant& layout) {
	auto payload = QVariantMap();
	if (layout.isValid() && !layout.isNull()) payload.insert("layout", layout);
	return this->dispatch("switch-keyboard-layout", payload);
}

qint32 TriadIpc::powerOffMonitors() { return this->dispatch("power-off-monitors"); }

qint32 TriadIpc::powerOnMonitors() { return this->dispatch("power-on-monitors"); }

qint32 TriadIpc::powerOffMonitor(const QString& output) {
	return this->dispatch("power-off-monitor", {{"output", output}});
}

qint32 TriadIpc::powerOnMonitor(const QString& output) {
	return this->dispatch("power-on-monitor", {{"output", output}});
}

qint32 TriadIpc::toggleOverview() { return this->dispatch("toggle-overview"); }

qint32 TriadIpc::openOverview() { return this->dispatch("open-overview"); }

qint32 TriadIpc::closeOverview() { return this->dispatch("close-overview"); }

qint32 TriadIpc::toggleScratchpad() { return this->dispatch("toggle-scratchpad"); }

qint32 TriadIpc::toggleNamedScratchpad(const QString& name) {
	return this->dispatch("toggle-named-scratchpad", {{"name", name}});
}

qint32 TriadIpc::moveToScratchpad() { return this->dispatch("move-to-scratchpad"); }

qint32 TriadIpc::moveToNamedScratchpad(const QString& name) {
	return this->dispatch("move-to-named-scratchpad", {{"name", name}});
}

qint32 TriadIpc::toggleFloating() { return this->dispatch("toggle-floating"); }

qint32 TriadIpc::fullscreenWindow(quint32 windowId) {
	auto payload = QVariantMap();
	if (windowId != 0) payload.insert("id", windowId);
	return this->dispatch("fullscreen-window", payload);
}

qint32 TriadIpc::toggleMaximized() { return this->dispatch("toggle-maximized"); }

qint32 TriadIpc::minimize() { return this->dispatch("minimize"); }

qint32 TriadIpc::moveToTag(quint32 tagId) {
	return this->dispatch("move-to-tag", {{"tag", tagId}});
}

qint32 TriadIpc::moveToWorkspace(qint32 workspaceIndex) {
	return this->dispatch("move-to-workspace", {{"workspace_idx", workspaceIndex}});
}

qint32 TriadIpc::moveWindowToTag(quint32 windowId, quint32 tagId, bool follow) {
	return this->dispatch(
	    "move-window-to-tag",
	    {{"id", windowId}, {"tag", tagId}, {"follow", follow}}
	);
}

qint32 TriadIpc::moveWindowToWorkspace(quint32 windowId, qint32 workspaceIndex, bool follow) {
	return this->dispatch(
	    "move-window-to-workspace",
	    {{"id", windowId}, {"workspace_idx", workspaceIndex}, {"follow", follow}}
	);
}

qint32 TriadIpc::focusOutput(const QString& output) {
	return this->dispatch("focus-output", {{"output", output}});
}

qint32 TriadIpc::moveWorkspaceToOutput(const QString& output) {
	return this->dispatch("move-workspace-to-output", {{"output", output}});
}

qint32 TriadIpc::moveToOutput(const QString& output) {
	return this->dispatch("move-to-output", {{"output", output}});
}

qint32 TriadIpc::newWorkspace() { return this->dispatch("new-workspace"); }

QVariantMap TriadIpc::commandSpec(const QString& name) const {
	for (const auto& value: this->mCommandsCatalog.value("commands").toList()) {
		auto command = value.toMap();
		if (command.value("name").toString() == name) return command;

		for (const auto& alias: command.value("aliases").toList()) {
			if (alias.toString() == name) return command;
		}
	}

	return {};
}

bool TriadIpc::hasCommand(const QString& name) const { return !this->commandSpec(name).isEmpty(); }

bool TriadIpc::validateAction(const QString& action, const QVariantMap& payload) const {
	auto command = this->commandSpec(action);
	if (command.isEmpty()) return false;
	return payloadMatchesShape(
	    command.value("name").toString(),
	    command.value("arg_shape").toString(),
	    payload
	);
}

qint32 TriadIpc::sendValidatedAction(const QString& action, const QVariantMap& payload) {
	if (!this->validateAction(action, payload)) return -1;
	return this->sendAction(action, payload);
}

void TriadIpc::autoRefreshCommands() {
	if (this->commandsAutoRefreshRequested) return;
	this->commandsAutoRefreshRequested = true;
	this->makeRequest(
	    triadPayload("commands"),
	    [this](qint32, bool, const QJsonObject&, const QString&) {
		    this->commandsAutoRefreshRequested = false;
	    }
	);
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

		if (eventName == "state-changed") {
			this->handleState(triad.value("state").toObject());
		} else if (eventName == "layout-state-changed") {
			this->handleLayoutState(triad.value("state").toObject());
		} else if (eventName == "window-changed") {
			this->handleWindow(triad.value("window").toObject());
			this->updateDerivedState();
		}
		return;
	}

	auto type = triad.value("type").toString();
	if (type == "state") {
		this->handleState(triad.value("state").toObject());
	} else if (type == "layout-state") {
		this->handleLayoutState(triad.value("state").toObject());
	} else if (type == "workspaces") {
		this->handleWorkspaces(triad.value("workspaces").toArray());
	} else if (type == "outputs") {
		this->handleOutputs(triad.value("outputs").toArray());
	} else if (type == "windows") {
		this->handleWindows(triad.value("windows").toArray());
		this->updateDerivedState();
	} else if (type == "focused-window") {
		if (triad.value("window").isObject()) {
			this->focusedWindowExplicitlyNull = false;
			this->handleWindow(triad.value("window").toObject());
		} else {
			this->focusedWindowExplicitlyNull = true;
			this->bFocusedWindow = nullptr;
		}
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
	this->bActiveTag = uintOrNone(state.toVariantMap(), "active_tag");
	this->bActiveWorkspaceIndex = intOrInvalid(state, "active_workspace_idx");
	this->handleWorkspaces(state.value("workspaces").toArray());
	this->updateDerivedState();
}

void TriadIpc::handleOverview(const QJsonObject& overview) {
	this->bOverviewOpen = overview.value("is_open").toBool();
	this->bOverviewSelectedWindowId = uintOrNone(overview.toVariantMap(), "selected_window_id");
}

void TriadIpc::handleWorkspaces(const QJsonArray& workspaces) {
	auto newValues = QList<TriadWorkspace*>();
	auto seen = QHash<quint32, bool>();

	for (const auto& value: workspaces) {
		auto object = value.toObject().toVariantMap();
		auto tagId = uintOrNone(object, "tag_id");
		if (tagId == 0) continue;

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
	auto seen = QHash<quint32, bool>();
	this->outputsByName.clear();

	for (const auto& value: outputs) {
		auto object = value.toObject().toVariantMap();
		auto id = uintOrNone(object, "id");
		if (id == 0) continue;

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
	this->focusedWindowExplicitlyNull = false;
	auto newValues = QList<TriadWindow*>();
	auto seen = QHash<quint32, bool>();

	for (const auto& value: windows) {
		auto object = value.toObject();
		auto id = uintOrNone(object.toVariantMap(), "id");
		if (id == 0) continue;

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
	auto id = uintOrNone(object.toVariantMap(), "id");
	if (id == 0) return;

	auto* window = this->windowsById.value(id);
	if (window == nullptr) {
		window = new TriadWindow(this);
		this->windowsById.insert(id, window);
		this->mWindows.insertObject(window);
	}

	window->updateFromObject(object.toVariantMap());
	if (window->bindableFocused().value()) this->focusedWindowExplicitlyNull = false;
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
	if (!this->focusedWindowExplicitlyNull) {
		for (auto* window: this->mWindows.valueList()) {
			if (window->bindableFocused().value()) {
				focusedWindow = window;
				break;
			}
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

TriadWorkspace* TriadIpc::workspaceByTag(quint32 tagId) const {
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

TriadWindow* TriadIpc::windowById(quint32 id) const { return this->windowsById.value(id); }

} // namespace qs::triad
