#pragma once

#include <qobject.h>
#include <qproperty.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>

namespace qs::triad {

class TriadIpc;

///! Triad window state
class TriadWindow: public QObject {
	Q_OBJECT;
	// clang-format off
	/// Stable Triad window ID.
	Q_PROPERTY(quint32 id READ default NOTIFY idChanged BINDABLE bindableId);
	/// Client process ID, or -1.
	Q_PROPERTY(qint32 pid READ default NOTIFY pidChanged BINDABLE bindablePid);
	/// Parent window ID, or 0.
	Q_PROPERTY(quint32 parentId READ default NOTIFY parentIdChanged BINDABLE bindableParentId);
	/// Window title.
	Q_PROPERTY(QString title READ default NOTIFY titleChanged BINDABLE bindableTitle);
	/// Application ID.
	Q_PROPERTY(QString appId READ default NOTIFY appIdChanged BINDABLE bindableAppId);
	/// Workspace tag ID, or 0.
	Q_PROPERTY(quint32 tagId READ default NOTIFY tagIdChanged BINDABLE bindableTagId);
	/// Compact workspace index, or -1.
	Q_PROPERTY(qint32 workspaceIndex READ default NOTIFY workspaceIndexChanged BINDABLE bindableWorkspaceIndex);
	/// Output name.
	Q_PROPERTY(QString outputName READ default NOTIFY outputNameChanged BINDABLE bindableOutputName);
	/// Column index for layouts that expose one, or -1.
	Q_PROPERTY(qint32 columnIndex READ default NOTIFY columnIndexChanged BINDABLE bindableColumnIndex);
	/// Window index inside its layout container, or -1.
	Q_PROPERTY(qint32 windowIndex READ default NOTIFY windowIndexChanged BINDABLE bindableWindowIndex);
	/// True when this window is focused.
	Q_PROPERTY(bool focused READ default NOTIFY focusedChanged BINDABLE bindableFocused);
	/// True when this window is floating.
	Q_PROPERTY(bool floating READ default NOTIFY floatingChanged BINDABLE bindableFloating);
	/// True when this window is maximized.
	Q_PROPERTY(bool maximized READ default NOTIFY maximizedChanged BINDABLE bindableMaximized);
	/// True when this window is minimized.
	Q_PROPERTY(bool minimized READ default NOTIFY minimizedChanged BINDABLE bindableMinimized);
	/// True when this window is fullscreen.
	Q_PROPERTY(bool fullscreen READ default NOTIFY fullscreenChanged BINDABLE bindableFullscreen);
	/// True when this window is sticky.
	Q_PROPERTY(bool sticky READ default NOTIFY stickyChanged BINDABLE bindableSticky);
	/// True when this window is an overlay.
	Q_PROPERTY(bool overlay READ default NOTIFY overlayChanged BINDABLE bindableOverlay);
	/// True when this window is unmanaged globally.
	Q_PROPERTY(bool unmanagedGlobal READ default NOTIFY unmanagedGlobalChanged BINDABLE bindableUnmanagedGlobal);
	/// Fullscreen output ID, or 0.
	Q_PROPERTY(quint32 fullscreenOutput READ default NOTIFY fullscreenOutputChanged BINDABLE bindableFullscreenOutput);
	/// Tiled width proportion.
	Q_PROPERTY(qreal widthProportion READ default NOTIFY widthProportionChanged BINDABLE bindableWidthProportion);
	/// Tiled height proportion.
	Q_PROPERTY(qreal heightProportion READ default NOTIFY heightProportionChanged BINDABLE bindableHeightProportion);
	/// Actual window width.
	Q_PROPERTY(qint32 actualWidth READ default NOTIFY actualWidthChanged BINDABLE bindableActualWidth);
	/// Actual window height.
	Q_PROPERTY(qint32 actualHeight READ default NOTIFY actualHeightChanged BINDABLE bindableActualHeight);
	/// Floating X coordinate.
	Q_PROPERTY(qint32 floatingX READ default NOTIFY floatingXChanged BINDABLE bindableFloatingX);
	/// Floating Y coordinate.
	Q_PROPERTY(qint32 floatingY READ default NOTIFY floatingYChanged BINDABLE bindableFloatingY);
	/// Floating width.
	Q_PROPERTY(qint32 floatingWidth READ default NOTIFY floatingWidthChanged BINDABLE bindableFloatingWidth);
	/// Floating height.
	Q_PROPERTY(qint32 floatingHeight READ default NOTIFY floatingHeightChanged BINDABLE bindableFloatingHeight);
	/// True when keyboard shortcuts are inhibited by the client.
	Q_PROPERTY(bool keyboardShortcutsInhibit READ default NOTIFY keyboardShortcutsInhibitChanged BINDABLE bindableKeyboardShortcutsInhibit);
	/// Idle inhibit mode.
	Q_PROPERTY(QString idleInhibit READ default NOTIFY idleInhibitChanged BINDABLE bindableIdleInhibit);
	/// True when Triad identifies this window as a terminal.
	Q_PROPERTY(bool terminal READ default NOTIFY terminalChanged BINDABLE bindableTerminal);
	/// True when this window may swallow another window.
	Q_PROPERTY(bool allowSwallow READ default NOTIFY allowSwallowChanged BINDABLE bindableAllowSwallow);
	/// ID of the window swallowing this window, or 0.
	Q_PROPERTY(quint32 swallowedBy READ default NOTIFY swallowedByChanged BINDABLE bindableSwallowedBy);
	/// ID of the window swallowed by this window, or 0.
	Q_PROPERTY(quint32 swallowing READ default NOTIFY swallowingChanged BINDABLE bindableSwallowing);
	/// Last JSON object received for this window, as a JavaScript object.
	Q_PROPERTY(QVariantMap lastIpcObject READ lastIpcObject NOTIFY lastIpcObjectChanged);
	// clang-format on
	QML_ELEMENT;
	QML_UNCREATABLE("TriadWindows must be retrieved from the Triad object.");

public:
	explicit TriadWindow(TriadIpc* ipc);

	void updateFromObject(const QVariantMap& object);

	/// Focus this window.
	Q_INVOKABLE void focus();
	/// Close this window.
	Q_INVOKABLE void close();

	[[nodiscard]] QBindable<quint32> bindableId() { return &this->bId; }
	[[nodiscard]] QBindable<qint32> bindablePid() { return &this->bPid; }
	[[nodiscard]] QBindable<quint32> bindableParentId() { return &this->bParentId; }
	[[nodiscard]] QBindable<QString> bindableTitle() { return &this->bTitle; }
	[[nodiscard]] QBindable<QString> bindableAppId() { return &this->bAppId; }
	[[nodiscard]] QBindable<quint32> bindableTagId() { return &this->bTagId; }
	[[nodiscard]] QBindable<qint32> bindableWorkspaceIndex() { return &this->bWorkspaceIndex; }
	[[nodiscard]] QBindable<QString> bindableOutputName() { return &this->bOutputName; }
	[[nodiscard]] QBindable<qint32> bindableColumnIndex() { return &this->bColumnIndex; }
	[[nodiscard]] QBindable<qint32> bindableWindowIndex() { return &this->bWindowIndex; }
	[[nodiscard]] QBindable<bool> bindableFocused() { return &this->bFocused; }
	[[nodiscard]] QBindable<bool> bindableFloating() { return &this->bFloating; }
	[[nodiscard]] QBindable<bool> bindableMaximized() { return &this->bMaximized; }
	[[nodiscard]] QBindable<bool> bindableMinimized() { return &this->bMinimized; }
	[[nodiscard]] QBindable<bool> bindableFullscreen() { return &this->bFullscreen; }
	[[nodiscard]] QBindable<bool> bindableSticky() { return &this->bSticky; }
	[[nodiscard]] QBindable<bool> bindableOverlay() { return &this->bOverlay; }
	[[nodiscard]] QBindable<bool> bindableUnmanagedGlobal() { return &this->bUnmanagedGlobal; }
	[[nodiscard]] QBindable<quint32> bindableFullscreenOutput() { return &this->bFullscreenOutput; }
	[[nodiscard]] QBindable<qreal> bindableWidthProportion() { return &this->bWidthProportion; }
	[[nodiscard]] QBindable<qreal> bindableHeightProportion() { return &this->bHeightProportion; }
	[[nodiscard]] QBindable<qint32> bindableActualWidth() { return &this->bActualWidth; }
	[[nodiscard]] QBindable<qint32> bindableActualHeight() { return &this->bActualHeight; }
	[[nodiscard]] QBindable<qint32> bindableFloatingX() { return &this->bFloatingX; }
	[[nodiscard]] QBindable<qint32> bindableFloatingY() { return &this->bFloatingY; }
	[[nodiscard]] QBindable<qint32> bindableFloatingWidth() { return &this->bFloatingWidth; }
	[[nodiscard]] QBindable<qint32> bindableFloatingHeight() { return &this->bFloatingHeight; }
	[[nodiscard]] QBindable<bool> bindableKeyboardShortcutsInhibit() {
		return &this->bKeyboardShortcutsInhibit;
	}
	[[nodiscard]] QBindable<QString> bindableIdleInhibit() { return &this->bIdleInhibit; }
	[[nodiscard]] QBindable<bool> bindableTerminal() { return &this->bTerminal; }
	[[nodiscard]] QBindable<bool> bindableAllowSwallow() { return &this->bAllowSwallow; }
	[[nodiscard]] QBindable<quint32> bindableSwallowedBy() { return &this->bSwallowedBy; }
	[[nodiscard]] QBindable<quint32> bindableSwallowing() { return &this->bSwallowing; }
	[[nodiscard]] QVariantMap lastIpcObject() const;

signals:
	void idChanged();
	void pidChanged();
	void parentIdChanged();
	void titleChanged();
	void appIdChanged();
	void tagIdChanged();
	void workspaceIndexChanged();
	void outputNameChanged();
	void columnIndexChanged();
	void windowIndexChanged();
	void focusedChanged();
	void floatingChanged();
	void maximizedChanged();
	void minimizedChanged();
	void fullscreenChanged();
	void stickyChanged();
	void overlayChanged();
	void unmanagedGlobalChanged();
	void fullscreenOutputChanged();
	void widthProportionChanged();
	void heightProportionChanged();
	void actualWidthChanged();
	void actualHeightChanged();
	void floatingXChanged();
	void floatingYChanged();
	void floatingWidthChanged();
	void floatingHeightChanged();
	void keyboardShortcutsInhibitChanged();
	void idleInhibitChanged();
	void terminalChanged();
	void allowSwallowChanged();
	void swallowedByChanged();
	void swallowingChanged();
	void lastIpcObjectChanged();

private:
	TriadIpc* ipc;
	QVariantMap mLastIpcObject;

	// clang-format off
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, quint32, bId, 0, &TriadWindow::idChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bPid, -1, &TriadWindow::pidChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, quint32, bParentId, 0, &TriadWindow::parentIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bTitle, &TriadWindow::titleChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bAppId, &TriadWindow::appIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, quint32, bTagId, 0, &TriadWindow::tagIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bWorkspaceIndex, -1, &TriadWindow::workspaceIndexChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bOutputName, &TriadWindow::outputNameChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bColumnIndex, -1, &TriadWindow::columnIndexChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bWindowIndex, -1, &TriadWindow::windowIndexChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bFocused, &TriadWindow::focusedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bFloating, &TriadWindow::floatingChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bMaximized, &TriadWindow::maximizedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bMinimized, &TriadWindow::minimizedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bFullscreen, &TriadWindow::fullscreenChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bSticky, &TriadWindow::stickyChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bOverlay, &TriadWindow::overlayChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bUnmanagedGlobal, &TriadWindow::unmanagedGlobalChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, quint32, bFullscreenOutput, 0, &TriadWindow::fullscreenOutputChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qreal, bWidthProportion, &TriadWindow::widthProportionChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qreal, bHeightProportion, &TriadWindow::heightProportionChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qint32, bActualWidth, &TriadWindow::actualWidthChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qint32, bActualHeight, &TriadWindow::actualHeightChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qint32, bFloatingX, &TriadWindow::floatingXChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qint32, bFloatingY, &TriadWindow::floatingYChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qint32, bFloatingWidth, &TriadWindow::floatingWidthChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, qint32, bFloatingHeight, &TriadWindow::floatingHeightChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bKeyboardShortcutsInhibit, &TriadWindow::keyboardShortcutsInhibitChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bIdleInhibit, &TriadWindow::idleInhibitChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bTerminal, &TriadWindow::terminalChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bAllowSwallow, &TriadWindow::allowSwallowChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, quint32, bSwallowedBy, 0, &TriadWindow::swallowedByChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, quint32, bSwallowing, 0, &TriadWindow::swallowingChanged);
	// clang-format on
};

} // namespace qs::triad
