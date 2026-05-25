#pragma once

#include <functional>

#include <qhash.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qlocalsocket.h>
#include <qmetatype.h>
#include <qobject.h>
#include <qproperty.h>
#include <qqmlintegration.h>
#include <qtimer.h>
#include <qvariant.h>

#include "../core/model.hpp"
#include "../core/streamreader.hpp"

namespace qs::triad {

class TriadOutput;
class TriadWindow;
class TriadWorkspace;

} // namespace qs::triad

Q_DECLARE_OPAQUE_POINTER(qs::triad::TriadOutput*);
Q_DECLARE_OPAQUE_POINTER(qs::triad::TriadWindow*);
Q_DECLARE_OPAQUE_POINTER(qs::triad::TriadWorkspace*);

namespace qs::triad {

///! Live Triad IPC event
/// Emitted by @@Triad.rawEvent(s). Holding this object after the signal handler
/// exits is undefined as the event instance is reused.
class TriadIpcEvent: public QObject {
	Q_OBJECT;
	/// The Triad event name.
	Q_PROPERTY(QString name MEMBER name CONSTANT);
	/// The raw event object as a JavaScript object.
	Q_PROPERTY(QVariantMap data MEMBER data CONSTANT);
	QML_NAMED_ELEMENT(TriadEvent);
	QML_UNCREATABLE("TriadIpcEvents cannot be created.");

public:
	explicit TriadIpcEvent(QObject* parent): QObject(parent) {}

	QString name;
	QVariantMap data;
};

class TriadIpc: public QObject {
	Q_OBJECT;

public:
	static TriadIpc* instance();

	[[nodiscard]] QString socketPath() const { return this->mSocketPath; }

	qint32 refresh();
	qint32 refreshLayout();
	qint32 refreshWindows();
	qint32 refreshCapabilities();
	qint32 refreshWorkspaces();
	qint32 refreshOutputs();
	qint32 refreshFocusedWindow();
	qint32 refreshOverview();
	qint32 refreshKeyboardLayouts();
	qint32 refreshCommands();
	qint32 sendRequest(const QString& request, const QVariantMap& payload = {});
	qint32 sendAction(const QString& action, const QVariantMap& payload = {});
	qint32 dispatch(const QString& action, const QVariantMap& payload = {});
	qint32 dispatchBinding(const QString& kind, const QString& binding, qint32 amount = 1);
	qint32 focusWorkspace(qint32 workspaceIndex);
	qint32 focusTag(qint32 tagId);
	qint32 focusWindow(qint32 windowId);
	qint32 closeWindow(qint32 windowId = 0);
	qint32 switchLayout();
	qint32 setLayout(const QString& layoutId, const QVariantMap& target = {});
	[[nodiscard]] QVariantMap commandSpec(const QString& name) const;
	[[nodiscard]] bool hasCommand(const QString& name) const;
	[[nodiscard]] bool validateAction(const QString& action, const QVariantMap& payload) const;
	qint32 sendValidatedAction(const QString& action, const QVariantMap& payload = {});

	[[nodiscard]] ObjectModel<TriadWorkspace>* workspaces() { return &this->mWorkspaces; }
	[[nodiscard]] ObjectModel<TriadOutput>* outputs() { return &this->mOutputs; }
	[[nodiscard]] ObjectModel<TriadWindow>* windows() { return &this->mWindows; }

	[[nodiscard]] QBindable<bool> bindableConnected() { return &this->bConnected; }
	[[nodiscard]] QBindable<TriadWorkspace*> bindableFocusedWorkspace() {
		return &this->bFocusedWorkspace;
	}
	[[nodiscard]] QBindable<TriadOutput*> bindableFocusedOutput() { return &this->bFocusedOutput; }
	[[nodiscard]] QBindable<TriadWindow*> bindableFocusedWindow() { return &this->bFocusedWindow; }
	[[nodiscard]] QBindable<bool> bindableOverviewOpen() { return &this->bOverviewOpen; }
	[[nodiscard]] QBindable<qint32> bindableOverviewSelectedWindowId() {
		return &this->bOverviewSelectedWindowId;
	}
	[[nodiscard]] QBindable<qint32> bindableActiveTag() { return &this->bActiveTag; }
	[[nodiscard]] QBindable<qint32> bindableActiveWorkspaceIndex() {
		return &this->bActiveWorkspaceIndex;
	}
	[[nodiscard]] QBindable<qint32> bindableCurrentKeyboardLayoutIndex() {
		return &this->bCurrentKeyboardLayoutIndex;
	}

	[[nodiscard]] QVariantMap capabilities() const { return this->mCapabilities; }
	[[nodiscard]] QVariantMap commandsCatalog() const { return this->mCommandsCatalog; }
	[[nodiscard]] QVariantList layouts() const { return this->mLayouts; }
	[[nodiscard]] QStringList layoutCycle() const { return this->mLayoutCycle; }
	[[nodiscard]] QVariantList layoutCycleEntries() const { return this->mLayoutCycleEntries; }
	[[nodiscard]] QStringList keyboardLayouts() const { return this->mKeyboardLayouts; }

	TriadWorkspace* workspaceByTag(qint32 tagId) const;
	TriadWorkspace* workspaceByIndex(qint32 workspaceIndex) const;
	TriadOutput* outputByName(const QString& name) const;
	TriadWindow* windowById(qint32 id) const;

signals:
	void connectedChanged();
	void socketPathChanged();
	void capabilitiesChanged();
	void focusedWorkspaceChanged();
	void focusedOutputChanged();
	void focusedWindowChanged();
	void overviewOpenChanged();
	void overviewSelectedWindowIdChanged();
	void activeTagChanged();
	void activeWorkspaceIndexChanged();
	void layoutsChanged();
	void layoutCycleChanged();
	void layoutCycleEntriesChanged();
	void keyboardLayoutsChanged();
	void currentKeyboardLayoutIndexChanged();
	void commandsCatalogChanged();
	void rawEvent(qs::triad::TriadIpcEvent* event);
	void requestFinished(qint32 requestId, bool ok, QVariantMap triad, QString error);

private slots:
	void eventSocketConnected();
	void eventSocketError(QLocalSocket::LocalSocketError error);
	void eventSocketStateChanged(QLocalSocket::LocalSocketState state);
	void eventSocketReady();
	void reconnect();

private:
	explicit TriadIpc();

	using RequestCallback = std::function<void(qint32, bool, QJsonObject, QString)>;

	void connectEventStream();
	qint32 makeRequest(const QJsonObject& payload, RequestCallback callback = {});
	qint32 makeTrackedRequest(const QJsonObject& payload);
	void handleLine(const QByteArray& line);
	void handleTriadObject(const QJsonObject& triad);
	void handleState(const QJsonObject& state);
	void handleLayoutState(const QJsonObject& state);
	void handleOverview(const QJsonObject& overview);
	void handleWindows(const QJsonArray& windows);
	void handleWindow(const QJsonObject& object);
	void handleOutputs(const QJsonArray& outputs);
	void handleWorkspaces(const QJsonArray& workspaces);
	void updateDerivedState();
	void autoRefreshCommands();
	void setSocketPath(const QString& path);
	[[nodiscard]] bool payloadMatchesShape(const QString& shape, const QVariantMap& payload) const;
	[[nodiscard]] bool hasCommandPayloadField(
	    const QVariantMap& payload,
	    const QString& key,
	    QMetaType::Type type
	) const;
	[[nodiscard]] QString discoverSocketPath() const;

	QLocalSocket eventSocket;
	StreamReader eventReader;
	QTimer reconnectTimer;
	QString mSocketPath;
	bool connecting = false;
	qint32 nextRequestId = 1;

	ObjectModel<TriadWorkspace> mWorkspaces {this};
	ObjectModel<TriadOutput> mOutputs {this};
	ObjectModel<TriadWindow> mWindows {this};

	QHash<qint32, TriadWorkspace*> workspacesByTag;
	QHash<qint32, TriadWindow*> windowsById;
	QHash<qint32, TriadOutput*> outputsById;
	QHash<QString, TriadOutput*> outputsByName;

	QVariantMap mCapabilities;
	QVariantMap mCommandsCatalog;
	QVariantList mLayouts;
	QStringList mLayoutCycle;
	QVariantList mLayoutCycleEntries;
	QStringList mKeyboardLayouts;
	TriadIpcEvent event {this};
	bool focusedWindowExplicitlyNull = false;
	bool commandsAutoRefreshRequested = false;

	Q_OBJECT_BINDABLE_PROPERTY(TriadIpc, bool, bConnected, &TriadIpc::connectedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(
	    TriadIpc,
	    TriadWorkspace*,
	    bFocusedWorkspace,
	    &TriadIpc::focusedWorkspaceChanged
	);
	Q_OBJECT_BINDABLE_PROPERTY(
	    TriadIpc,
	    TriadOutput*,
	    bFocusedOutput,
	    &TriadIpc::focusedOutputChanged
	);
	Q_OBJECT_BINDABLE_PROPERTY(
	    TriadIpc,
	    TriadWindow*,
	    bFocusedWindow,
	    &TriadIpc::focusedWindowChanged
	);
	Q_OBJECT_BINDABLE_PROPERTY(TriadIpc, bool, bOverviewOpen, &TriadIpc::overviewOpenChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(
	    TriadIpc,
	    qint32,
	    bOverviewSelectedWindowId,
	    -1,
	    &TriadIpc::overviewSelectedWindowIdChanged
	);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(
	    TriadIpc,
	    qint32,
	    bActiveTag,
	    -1,
	    &TriadIpc::activeTagChanged
	);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(
	    TriadIpc,
	    qint32,
	    bActiveWorkspaceIndex,
	    -1,
	    &TriadIpc::activeWorkspaceIndexChanged
	);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(
	    TriadIpc,
	    qint32,
	    bCurrentKeyboardLayoutIndex,
	    -1,
	    &TriadIpc::currentKeyboardLayoutIndexChanged
	);
};

} // namespace qs::triad
