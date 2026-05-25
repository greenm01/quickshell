#pragma once

#include <qobject.h>
#include <qproperty.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>

namespace qs::triad {

class TriadIpc;
class TriadWindow;

///! Triad workspace state
class TriadWorkspace: public QObject {
	Q_OBJECT;
	// clang-format off
	/// Stable Triad tag ID for this workspace.
	Q_PROPERTY(quint32 tagId READ default NOTIFY tagIdChanged BINDABLE bindableTagId);
	/// Compact 1-based workspace index displayed to users.
	Q_PROPERTY(qint32 workspaceIndex READ default NOTIFY workspaceIndexChanged BINDABLE bindableWorkspaceIndex);
	/// Optional workspace name.
	Q_PROPERTY(QString name READ default NOTIFY nameChanged BINDABLE bindableName);
	/// Name of the output currently associated with this workspace.
	Q_PROPERTY(QString outputName READ default NOTIFY outputNameChanged BINDABLE bindableOutputName);
	/// Current layout ID.
	Q_PROPERTY(QString layout READ default NOTIFY layoutChanged BINDABLE bindableLayout);
	/// High-level layout kind.
	Q_PROPERTY(QString layoutKind READ default NOTIFY layoutKindChanged BINDABLE bindableLayoutKind);
	/// Runtime layout kind currently used by Triad.
	Q_PROPERTY(QString runtimeKind READ default NOTIFY runtimeKindChanged BINDABLE bindableRuntimeKind);
	/// Source of the current layout.
	Q_PROPERTY(QString layoutSource READ default NOTIFY layoutSourceChanged BINDABLE bindableLayoutSource);
	/// Fallback layout ID used by custom/native layouts.
	Q_PROPERTY(QString fallbackLayout READ default NOTIFY fallbackLayoutChanged BINDABLE bindableFallbackLayout);
	/// True when this workspace is configured.
	Q_PROPERTY(bool configured READ default NOTIFY configuredChanged BINDABLE bindableConfigured);
	/// True when this workspace is the focused Triad workspace.
	Q_PROPERTY(bool active READ default NOTIFY activeChanged BINDABLE bindableActive);
	/// True when this workspace is visible on an output.
	Q_PROPERTY(bool outputVisible READ default NOTIFY outputVisibleChanged BINDABLE bindableOutputVisible);
	/// True when this workspace contains windows.
	Q_PROPERTY(bool occupied READ default NOTIFY occupiedChanged BINDABLE bindableOccupied);
	/// True when this workspace has urgent state.
	Q_PROPERTY(bool urgent READ default NOTIFY urgentChanged BINDABLE bindableUrgent);
	/// Focused window on this workspace, or null.
	Q_PROPERTY(qs::triad::TriadWindow* focusedWindow READ default NOTIFY focusedWindowChanged BINDABLE bindableFocusedWindow);
	/// ID of the focused window on this workspace, or 0.
	Q_PROPERTY(quint32 focusedWindowId READ default NOTIFY focusedWindowIdChanged BINDABLE bindableFocusedWindowId);
	/// Master area window count for layouts that expose one.
	Q_PROPERTY(qint32 masterCount READ default NOTIFY masterCountChanged BINDABLE bindableMasterCount);
	/// Master area split ratio for layouts that expose one.
	Q_PROPERTY(qreal masterSplitRatio READ default NOTIFY masterSplitRatioChanged BINDABLE bindableMasterSplitRatio);
	/// Scroller column data for this workspace.
	Q_PROPERTY(QVariantList columns READ columns NOTIFY columnsChanged);
	/// Frame-tree data for this workspace.
	Q_PROPERTY(QVariantList frames READ frames NOTIFY framesChanged);
	/// BSP-tree data for this workspace.
	Q_PROPERTY(QVariantList bspNodes READ bspNodes NOTIFY bspNodesChanged);
	/// Split-tree data for this workspace.
	Q_PROPERTY(QVariantList splitNodes READ splitNodes NOTIFY splitNodesChanged);
	/// Viewport offsets for layouts that expose them.
	Q_PROPERTY(QVariantMap viewport READ viewport NOTIFY viewportChanged);
	/// Last JSON object received for this workspace, as a JavaScript object.
	Q_PROPERTY(QVariantMap lastIpcObject READ lastIpcObject NOTIFY lastIpcObjectChanged);
	// clang-format on
	QML_ELEMENT;
	QML_UNCREATABLE("TriadWorkspaces must be retrieved from the Triad object.");

public:
	explicit TriadWorkspace(TriadIpc* ipc);

	void updateFromObject(const QVariantMap& object);
	void setFocusedWindow(TriadWindow* window);

	/// Focus this workspace.
	Q_INVOKABLE void activate();
	/// Set this workspace's layout.
	Q_INVOKABLE void setLayout(const QString& layoutId);

	[[nodiscard]] QBindable<quint32> bindableTagId() { return &this->bTagId; }
	[[nodiscard]] QBindable<qint32> bindableWorkspaceIndex() { return &this->bWorkspaceIndex; }
	[[nodiscard]] QBindable<QString> bindableName() { return &this->bName; }
	[[nodiscard]] QBindable<QString> bindableOutputName() { return &this->bOutputName; }
	[[nodiscard]] QBindable<QString> bindableLayout() { return &this->bLayout; }
	[[nodiscard]] QBindable<QString> bindableLayoutKind() { return &this->bLayoutKind; }
	[[nodiscard]] QBindable<QString> bindableRuntimeKind() { return &this->bRuntimeKind; }
	[[nodiscard]] QBindable<QString> bindableLayoutSource() { return &this->bLayoutSource; }
	[[nodiscard]] QBindable<QString> bindableFallbackLayout() { return &this->bFallbackLayout; }
	[[nodiscard]] QBindable<bool> bindableConfigured() { return &this->bConfigured; }
	[[nodiscard]] QBindable<bool> bindableActive() { return &this->bActive; }
	[[nodiscard]] QBindable<bool> bindableOutputVisible() { return &this->bOutputVisible; }
	[[nodiscard]] QBindable<bool> bindableOccupied() { return &this->bOccupied; }
	[[nodiscard]] QBindable<bool> bindableUrgent() { return &this->bUrgent; }
	[[nodiscard]] QBindable<TriadWindow*> bindableFocusedWindow() { return &this->bFocusedWindow; }
	[[nodiscard]] QBindable<quint32> bindableFocusedWindowId() { return &this->bFocusedWindowId; }
	[[nodiscard]] QBindable<qint32> bindableMasterCount() { return &this->bMasterCount; }
	[[nodiscard]] QBindable<qreal> bindableMasterSplitRatio() { return &this->bMasterSplitRatio; }
	[[nodiscard]] QVariantList columns() const;
	[[nodiscard]] QVariantList frames() const;
	[[nodiscard]] QVariantList bspNodes() const;
	[[nodiscard]] QVariantList splitNodes() const;
	[[nodiscard]] QVariantMap viewport() const;
	[[nodiscard]] QVariantMap lastIpcObject() const;

signals:
	void tagIdChanged();
	void workspaceIndexChanged();
	void nameChanged();
	void outputNameChanged();
	void layoutChanged();
	void layoutKindChanged();
	void runtimeKindChanged();
	void layoutSourceChanged();
	void fallbackLayoutChanged();
	void configuredChanged();
	void activeChanged();
	void outputVisibleChanged();
	void occupiedChanged();
	void urgentChanged();
	void focusedWindowChanged();
	void focusedWindowIdChanged();
	void masterCountChanged();
	void masterSplitRatioChanged();
	void columnsChanged();
	void framesChanged();
	void bspNodesChanged();
	void splitNodesChanged();
	void viewportChanged();
	void lastIpcObjectChanged();

private:
	TriadIpc* ipc;
	QVariantMap mLastIpcObject;
	QVariantList mColumns;
	QVariantList mFrames;
	QVariantList mBspNodes;
	QVariantList mSplitNodes;
	QVariantMap mViewport;

	// clang-format off
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWorkspace, quint32, bTagId, 0, &TriadWorkspace::tagIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWorkspace, qint32, bWorkspaceIndex, -1, &TriadWorkspace::workspaceIndexChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bName, &TriadWorkspace::nameChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bOutputName, &TriadWorkspace::outputNameChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bLayout, &TriadWorkspace::layoutChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bLayoutKind, &TriadWorkspace::layoutKindChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bRuntimeKind, &TriadWorkspace::runtimeKindChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bLayoutSource, &TriadWorkspace::layoutSourceChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, QString, bFallbackLayout, &TriadWorkspace::fallbackLayoutChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, bool, bConfigured, &TriadWorkspace::configuredChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, bool, bActive, &TriadWorkspace::activeChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, bool, bOutputVisible, &TriadWorkspace::outputVisibleChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, bool, bOccupied, &TriadWorkspace::occupiedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, bool, bUrgent, &TriadWorkspace::urgentChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, TriadWindow*, bFocusedWindow, &TriadWorkspace::focusedWindowChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWorkspace, quint32, bFocusedWindowId, 0, &TriadWorkspace::focusedWindowIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, qint32, bMasterCount, &TriadWorkspace::masterCountChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWorkspace, qreal, bMasterSplitRatio, &TriadWorkspace::masterSplitRatioChanged);
	// clang-format on
};

} // namespace qs::triad
