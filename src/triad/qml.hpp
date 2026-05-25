#pragma once

#include <qobject.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qvariant.h>

#include "../core/doc.hpp"
#include "../core/model.hpp"
#include "connection.hpp"
#include "output.hpp"
#include "window.hpp"
#include "workspace.hpp"

namespace qs::triad {

///! Triad IPC integration
class TriadIpcQml: public QObject {
	Q_OBJECT;
	// clang-format off
	/// Path to the Triad IPC socket.
	Q_PROPERTY(QString socketPath READ socketPath NOTIFY socketPathChanged);
	/// True when the Triad event stream is connected.
	Q_PROPERTY(bool connected READ default NOTIFY connectedChanged BINDABLE bindableConnected);
	/// Native Triad IPC capabilities.
	Q_PROPERTY(QVariantMap capabilities READ capabilities NOTIFY capabilitiesChanged);
	/// All Triad workspaces.
	QSDOC_TYPE_OVERRIDE(ObjectModel<qs::triad::TriadWorkspace>*);
	Q_PROPERTY(UntypedObjectModel* workspaces READ workspaces CONSTANT);
	/// All Triad outputs.
	QSDOC_TYPE_OVERRIDE(ObjectModel<qs::triad::TriadOutput>*);
	Q_PROPERTY(UntypedObjectModel* outputs READ outputs CONSTANT);
	/// All Triad windows.
	QSDOC_TYPE_OVERRIDE(ObjectModel<qs::triad::TriadWindow>*);
	Q_PROPERTY(UntypedObjectModel* windows READ windows CONSTANT);
	/// Currently focused workspace, or null.
	Q_PROPERTY(qs::triad::TriadWorkspace* focusedWorkspace READ default NOTIFY focusedWorkspaceChanged BINDABLE bindableFocusedWorkspace);
	/// Output containing the focused workspace, or null.
	Q_PROPERTY(qs::triad::TriadOutput* focusedOutput READ default NOTIFY focusedOutputChanged BINDABLE bindableFocusedOutput);
	/// Currently focused window, or null.
	Q_PROPERTY(qs::triad::TriadWindow* focusedWindow READ default NOTIFY focusedWindowChanged BINDABLE bindableFocusedWindow);
	/// True when Triad's overview is open.
	Q_PROPERTY(bool overviewOpen READ default NOTIFY overviewOpenChanged BINDABLE bindableOverviewOpen);
	/// Window selected by Triad's overview, or -1.
	Q_PROPERTY(qint32 overviewSelectedWindowId READ default NOTIFY overviewSelectedWindowIdChanged BINDABLE bindableOverviewSelectedWindowId);
	/// Stable tag ID of the active workspace, or -1.
	Q_PROPERTY(qint32 activeTag READ default NOTIFY activeTagChanged BINDABLE bindableActiveTag);
	/// Compact workspace index of the active workspace, or -1.
	Q_PROPERTY(qint32 activeWorkspaceIndex READ default NOTIFY activeWorkspaceIndexChanged BINDABLE bindableActiveWorkspaceIndex);
	/// Layouts supported by the running Triad instance.
	Q_PROPERTY(QVariantList layouts READ layouts NOTIFY layoutsChanged);
	/// IDs in Triad's configured layout cycle.
	Q_PROPERTY(QStringList layoutCycle READ layoutCycle NOTIFY layoutCycleChanged);
	/// Rich layout cycle entries from Triad.
	Q_PROPERTY(QVariantList layoutCycleEntries READ layoutCycleEntries NOTIFY layoutCycleEntriesChanged);
	/// Keyboard layout names reported by Triad.
	Q_PROPERTY(QStringList keyboardLayouts READ keyboardLayouts NOTIFY keyboardLayoutsChanged);
	/// Current keyboard layout index, or -1.
	Q_PROPERTY(qint32 currentKeyboardLayoutIndex READ default NOTIFY currentKeyboardLayoutIndexChanged BINDABLE bindableCurrentKeyboardLayoutIndex);
	/// Triad command catalog from the `commands` request.
	Q_PROPERTY(QVariantMap commandsCatalog READ commandsCatalog NOTIFY commandsCatalogChanged);
	// clang-format on
	QML_NAMED_ELEMENT(Triad);
	QML_SINGLETON;

public:
	explicit TriadIpcQml();

	/// Refresh full Triad state.
	Q_INVOKABLE static void refresh();
	/// Refresh Triad layout/workspace state.
	Q_INVOKABLE static void refreshLayout();
	/// Refresh Triad window state.
	Q_INVOKABLE static void refreshWindows();
	/// Send a native Triad request. The returned ID is emitted by requestFinished.
	Q_INVOKABLE static qint32 sendRequest(const QString& request, const QVariantMap& payload = {});
	/// Send a native Triad action. The returned ID is emitted by requestFinished.
	Q_INVOKABLE static qint32 sendAction(const QString& action, const QVariantMap& payload = {});
	/// Dispatch a native Triad action.
	Q_INVOKABLE static void dispatch(const QString& action, const QVariantMap& payload = {});
	/// Focus a workspace by compact index.
	Q_INVOKABLE static void focusWorkspace(qint32 workspaceIndex);
	/// Focus a workspace by stable tag ID.
	Q_INVOKABLE static void focusTag(qint32 tagId);
	/// Focus a window by stable Triad window ID.
	Q_INVOKABLE static void focusWindow(qint32 windowId);
	/// Close the focused window, or a specific window when `windowId` is non-zero.
	Q_INVOKABLE static void closeWindow(qint32 windowId = 0);
	/// Advance the active workspace through Triad's configured layout cycle.
	Q_INVOKABLE static void switchLayout();
	/// Set a Triad layout, optionally with `{ tag: id }` or `{ workspace_idx: index }` target.
	Q_INVOKABLE static void setLayout(const QString& layoutId, const QVariantMap& target = {});

	[[nodiscard]] static QString socketPath();
	[[nodiscard]] static QVariantMap capabilities();
	[[nodiscard]] static QVariantMap commandsCatalog();
	[[nodiscard]] static QVariantList layouts();
	[[nodiscard]] static QStringList layoutCycle();
	[[nodiscard]] static QVariantList layoutCycleEntries();
	[[nodiscard]] static QStringList keyboardLayouts();
	[[nodiscard]] static ObjectModel<TriadWorkspace>* workspaces();
	[[nodiscard]] static ObjectModel<TriadOutput>* outputs();
	[[nodiscard]] static ObjectModel<TriadWindow>* windows();
	[[nodiscard]] static QBindable<bool> bindableConnected();
	[[nodiscard]] static QBindable<TriadWorkspace*> bindableFocusedWorkspace();
	[[nodiscard]] static QBindable<TriadOutput*> bindableFocusedOutput();
	[[nodiscard]] static QBindable<TriadWindow*> bindableFocusedWindow();
	[[nodiscard]] static QBindable<bool> bindableOverviewOpen();
	[[nodiscard]] static QBindable<qint32> bindableOverviewSelectedWindowId();
	[[nodiscard]] static QBindable<qint32> bindableActiveTag();
	[[nodiscard]] static QBindable<qint32> bindableActiveWorkspaceIndex();
	[[nodiscard]] static QBindable<qint32> bindableCurrentKeyboardLayoutIndex();

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
	/// Emitted for every native Triad event received from the event stream.
	void rawEvent(qs::triad::TriadIpcEvent* event);
	/// Emitted when a request sent via sendRequest or sendAction completes.
	void requestFinished(qint32 requestId, bool ok, QVariantMap triad, QString error);
};

} // namespace qs::triad
