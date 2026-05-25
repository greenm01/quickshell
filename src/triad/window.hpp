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
	Q_PROPERTY(qint32 id READ default NOTIFY idChanged BINDABLE bindableId);
	/// Client process ID, or -1.
	Q_PROPERTY(qint32 pid READ default NOTIFY pidChanged BINDABLE bindablePid);
	/// Window title.
	Q_PROPERTY(QString title READ default NOTIFY titleChanged BINDABLE bindableTitle);
	/// Application ID.
	Q_PROPERTY(QString appId READ default NOTIFY appIdChanged BINDABLE bindableAppId);
	/// Workspace tag ID, or -1.
	Q_PROPERTY(qint32 tagId READ default NOTIFY tagIdChanged BINDABLE bindableTagId);
	/// Compact workspace index, or -1.
	Q_PROPERTY(qint32 workspaceIndex READ default NOTIFY workspaceIndexChanged BINDABLE bindableWorkspaceIndex);
	/// Output name.
	Q_PROPERTY(QString outputName READ default NOTIFY outputNameChanged BINDABLE bindableOutputName);
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
	/// True when Triad identifies this window as a terminal.
	Q_PROPERTY(bool terminal READ default NOTIFY terminalChanged BINDABLE bindableTerminal);
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

	[[nodiscard]] QBindable<qint32> bindableId() { return &this->bId; }
	[[nodiscard]] QBindable<qint32> bindablePid() { return &this->bPid; }
	[[nodiscard]] QBindable<QString> bindableTitle() { return &this->bTitle; }
	[[nodiscard]] QBindable<QString> bindableAppId() { return &this->bAppId; }
	[[nodiscard]] QBindable<qint32> bindableTagId() { return &this->bTagId; }
	[[nodiscard]] QBindable<qint32> bindableWorkspaceIndex() { return &this->bWorkspaceIndex; }
	[[nodiscard]] QBindable<QString> bindableOutputName() { return &this->bOutputName; }
	[[nodiscard]] QBindable<bool> bindableFocused() { return &this->bFocused; }
	[[nodiscard]] QBindable<bool> bindableFloating() { return &this->bFloating; }
	[[nodiscard]] QBindable<bool> bindableMaximized() { return &this->bMaximized; }
	[[nodiscard]] QBindable<bool> bindableMinimized() { return &this->bMinimized; }
	[[nodiscard]] QBindable<bool> bindableFullscreen() { return &this->bFullscreen; }
	[[nodiscard]] QBindable<bool> bindableTerminal() { return &this->bTerminal; }
	[[nodiscard]] QVariantMap lastIpcObject() const;

signals:
	void idChanged();
	void pidChanged();
	void titleChanged();
	void appIdChanged();
	void tagIdChanged();
	void workspaceIndexChanged();
	void outputNameChanged();
	void focusedChanged();
	void floatingChanged();
	void maximizedChanged();
	void minimizedChanged();
	void fullscreenChanged();
	void terminalChanged();
	void lastIpcObjectChanged();

private:
	TriadIpc* ipc;
	QVariantMap mLastIpcObject;

	// clang-format off
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bId, -1, &TriadWindow::idChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bPid, -1, &TriadWindow::pidChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bTitle, &TriadWindow::titleChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bAppId, &TriadWindow::appIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bTagId, -1, &TriadWindow::tagIdChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadWindow, qint32, bWorkspaceIndex, -1, &TriadWindow::workspaceIndexChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, QString, bOutputName, &TriadWindow::outputNameChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bFocused, &TriadWindow::focusedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bFloating, &TriadWindow::floatingChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bMaximized, &TriadWindow::maximizedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bMinimized, &TriadWindow::minimizedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bFullscreen, &TriadWindow::fullscreenChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadWindow, bool, bTerminal, &TriadWindow::terminalChanged);
	// clang-format on
};

} // namespace qs::triad
