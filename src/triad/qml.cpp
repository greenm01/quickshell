#include "qml.hpp"

#include <qobject.h>
#include <qproperty.h>
#include <qstringlist.h>
#include <qvariant.h>

#include "../core/model.hpp"
#include "connection.hpp"

namespace qs::triad {

TriadIpcQml::TriadIpcQml() {
	auto* instance = TriadIpc::instance();

	// clang-format off
	QObject::connect(instance, &TriadIpc::connectedChanged, this, &TriadIpcQml::connectedChanged);
	QObject::connect(instance, &TriadIpc::socketPathChanged, this, &TriadIpcQml::socketPathChanged);
	QObject::connect(instance, &TriadIpc::capabilitiesChanged, this, &TriadIpcQml::capabilitiesChanged);
	QObject::connect(instance, &TriadIpc::focusedWorkspaceChanged, this, &TriadIpcQml::focusedWorkspaceChanged);
	QObject::connect(instance, &TriadIpc::focusedOutputChanged, this, &TriadIpcQml::focusedOutputChanged);
	QObject::connect(instance, &TriadIpc::focusedWindowChanged, this, &TriadIpcQml::focusedWindowChanged);
	QObject::connect(instance, &TriadIpc::overviewOpenChanged, this, &TriadIpcQml::overviewOpenChanged);
	QObject::connect(instance, &TriadIpc::overviewSelectedWindowIdChanged, this, &TriadIpcQml::overviewSelectedWindowIdChanged);
	QObject::connect(instance, &TriadIpc::activeTagChanged, this, &TriadIpcQml::activeTagChanged);
	QObject::connect(instance, &TriadIpc::activeWorkspaceIndexChanged, this, &TriadIpcQml::activeWorkspaceIndexChanged);
	QObject::connect(instance, &TriadIpc::layoutsChanged, this, &TriadIpcQml::layoutsChanged);
	QObject::connect(instance, &TriadIpc::layoutCycleChanged, this, &TriadIpcQml::layoutCycleChanged);
	QObject::connect(instance, &TriadIpc::layoutCycleEntriesChanged, this, &TriadIpcQml::layoutCycleEntriesChanged);
	QObject::connect(instance, &TriadIpc::keyboardLayoutsChanged, this, &TriadIpcQml::keyboardLayoutsChanged);
	QObject::connect(instance, &TriadIpc::currentKeyboardLayoutIndexChanged, this, &TriadIpcQml::currentKeyboardLayoutIndexChanged);
	QObject::connect(instance, &TriadIpc::commandsCatalogChanged, this, &TriadIpcQml::commandsCatalogChanged);
	QObject::connect(instance, &TriadIpc::rawEvent, this, &TriadIpcQml::rawEvent);
	QObject::connect(instance, &TriadIpc::requestFinished, this, &TriadIpcQml::requestFinished);
	// clang-format on
}

qint32 TriadIpcQml::refresh() { return TriadIpc::instance()->refresh(); }
qint32 TriadIpcQml::refreshLayout() { return TriadIpc::instance()->refreshLayout(); }
qint32 TriadIpcQml::refreshWindows() { return TriadIpc::instance()->refreshWindows(); }
qint32 TriadIpcQml::refreshCapabilities() {
	return TriadIpc::instance()->refreshCapabilities();
}
qint32 TriadIpcQml::refreshWorkspaces() { return TriadIpc::instance()->refreshWorkspaces(); }
qint32 TriadIpcQml::refreshOutputs() { return TriadIpc::instance()->refreshOutputs(); }
qint32 TriadIpcQml::refreshFocusedWindow() {
	return TriadIpc::instance()->refreshFocusedWindow();
}
qint32 TriadIpcQml::refreshOverview() { return TriadIpc::instance()->refreshOverview(); }
qint32 TriadIpcQml::refreshKeyboardLayouts() {
	return TriadIpc::instance()->refreshKeyboardLayouts();
}
qint32 TriadIpcQml::refreshCommands() { return TriadIpc::instance()->refreshCommands(); }
qint32 TriadIpcQml::sendRequest(const QString& request, const QVariantMap& payload) {
	return TriadIpc::instance()->sendRequest(request, payload);
}
qint32 TriadIpcQml::sendAction(const QString& action, const QVariantMap& payload) {
	return TriadIpc::instance()->sendAction(action, payload);
}
qint32 TriadIpcQml::dispatch(const QString& action, const QVariantMap& payload) {
	return TriadIpc::instance()->dispatch(action, payload);
}
qint32 TriadIpcQml::dispatchBinding(
    const QString& kind,
    const QString& binding,
    qint32 amount
) {
	return TriadIpc::instance()->dispatchBinding(kind, binding, amount);
}
qint32 TriadIpcQml::focusWorkspace(qint32 workspaceIndex) {
	return TriadIpc::instance()->focusWorkspace(workspaceIndex);
}
qint32 TriadIpcQml::focusTag(quint32 tagId) { return TriadIpc::instance()->focusTag(tagId); }
qint32 TriadIpcQml::focusWindow(quint32 windowId) {
	return TriadIpc::instance()->focusWindow(windowId);
}
qint32 TriadIpcQml::closeWindow(quint32 windowId) {
	return TriadIpc::instance()->closeWindow(windowId);
}
qint32 TriadIpcQml::switchLayout() { return TriadIpc::instance()->switchLayout(); }
qint32 TriadIpcQml::setLayout(const QString& layoutId, const QVariantMap& target) {
	return TriadIpc::instance()->setLayout(layoutId, target);
}
qint32 TriadIpcQml::spawn(const QStringList& argv) { return TriadIpc::instance()->spawn(argv); }
qint32 TriadIpcQml::switchKeyboardLayout(const QVariant& layout) {
	return TriadIpc::instance()->switchKeyboardLayout(layout);
}
qint32 TriadIpcQml::powerOffMonitors() { return TriadIpc::instance()->powerOffMonitors(); }
qint32 TriadIpcQml::powerOnMonitors() { return TriadIpc::instance()->powerOnMonitors(); }
qint32 TriadIpcQml::powerOffMonitor(const QString& output) {
	return TriadIpc::instance()->powerOffMonitor(output);
}
qint32 TriadIpcQml::powerOnMonitor(const QString& output) {
	return TriadIpc::instance()->powerOnMonitor(output);
}
qint32 TriadIpcQml::toggleOverview() { return TriadIpc::instance()->toggleOverview(); }
qint32 TriadIpcQml::openOverview() { return TriadIpc::instance()->openOverview(); }
qint32 TriadIpcQml::closeOverview() { return TriadIpc::instance()->closeOverview(); }
qint32 TriadIpcQml::toggleScratchpad() { return TriadIpc::instance()->toggleScratchpad(); }
qint32 TriadIpcQml::toggleNamedScratchpad(const QString& name) {
	return TriadIpc::instance()->toggleNamedScratchpad(name);
}
qint32 TriadIpcQml::moveToScratchpad() { return TriadIpc::instance()->moveToScratchpad(); }
qint32 TriadIpcQml::moveToNamedScratchpad(const QString& name) {
	return TriadIpc::instance()->moveToNamedScratchpad(name);
}
qint32 TriadIpcQml::toggleFloating() { return TriadIpc::instance()->toggleFloating(); }
qint32 TriadIpcQml::fullscreenWindow(quint32 windowId) {
	return TriadIpc::instance()->fullscreenWindow(windowId);
}
qint32 TriadIpcQml::toggleMaximized() { return TriadIpc::instance()->toggleMaximized(); }
qint32 TriadIpcQml::minimize() { return TriadIpc::instance()->minimize(); }
qint32 TriadIpcQml::moveToTag(quint32 tagId) {
	return TriadIpc::instance()->moveToTag(tagId);
}
qint32 TriadIpcQml::moveToWorkspace(qint32 workspaceIndex) {
	return TriadIpc::instance()->moveToWorkspace(workspaceIndex);
}
qint32 TriadIpcQml::moveWindowToTag(quint32 windowId, quint32 tagId, bool follow) {
	return TriadIpc::instance()->moveWindowToTag(windowId, tagId, follow);
}
qint32 TriadIpcQml::moveWindowToWorkspace(
    quint32 windowId,
    qint32 workspaceIndex,
    bool follow
) {
	return TriadIpc::instance()->moveWindowToWorkspace(windowId, workspaceIndex, follow);
}
qint32 TriadIpcQml::focusOutput(const QString& output) {
	return TriadIpc::instance()->focusOutput(output);
}
qint32 TriadIpcQml::moveWorkspaceToOutput(const QString& output) {
	return TriadIpc::instance()->moveWorkspaceToOutput(output);
}
qint32 TriadIpcQml::moveToOutput(const QString& output) {
	return TriadIpc::instance()->moveToOutput(output);
}
qint32 TriadIpcQml::newWorkspace() { return TriadIpc::instance()->newWorkspace(); }
QVariantMap TriadIpcQml::commandSpec(const QString& name) {
	return TriadIpc::instance()->commandSpec(name);
}
bool TriadIpcQml::hasCommand(const QString& name) {
	return TriadIpc::instance()->hasCommand(name);
}
bool TriadIpcQml::validateAction(const QString& action, const QVariantMap& payload) {
	return TriadIpc::instance()->validateAction(action, payload);
}
qint32 TriadIpcQml::sendValidatedAction(const QString& action, const QVariantMap& payload) {
	return TriadIpc::instance()->sendValidatedAction(action, payload);
}

QString TriadIpcQml::socketPath() { return TriadIpc::instance()->socketPath(); }
QVariantMap TriadIpcQml::capabilities() { return TriadIpc::instance()->capabilities(); }
QVariantMap TriadIpcQml::commandsCatalog() { return TriadIpc::instance()->commandsCatalog(); }
QVariantList TriadIpcQml::layouts() { return TriadIpc::instance()->layouts(); }
QStringList TriadIpcQml::layoutCycle() { return TriadIpc::instance()->layoutCycle(); }
QVariantList TriadIpcQml::layoutCycleEntries() {
	return TriadIpc::instance()->layoutCycleEntries();
}
QStringList TriadIpcQml::keyboardLayouts() { return TriadIpc::instance()->keyboardLayouts(); }
ObjectModel<TriadWorkspace>* TriadIpcQml::workspaces() {
	return TriadIpc::instance()->workspaces();
}
ObjectModel<TriadOutput>* TriadIpcQml::outputs() { return TriadIpc::instance()->outputs(); }
ObjectModel<TriadWindow>* TriadIpcQml::windows() { return TriadIpc::instance()->windows(); }
QBindable<bool> TriadIpcQml::bindableConnected() {
	return TriadIpc::instance()->bindableConnected();
}
QBindable<TriadWorkspace*> TriadIpcQml::bindableFocusedWorkspace() {
	return TriadIpc::instance()->bindableFocusedWorkspace();
}
QBindable<TriadOutput*> TriadIpcQml::bindableFocusedOutput() {
	return TriadIpc::instance()->bindableFocusedOutput();
}
QBindable<TriadWindow*> TriadIpcQml::bindableFocusedWindow() {
	return TriadIpc::instance()->bindableFocusedWindow();
}
QBindable<bool> TriadIpcQml::bindableOverviewOpen() {
	return TriadIpc::instance()->bindableOverviewOpen();
}
QBindable<quint32> TriadIpcQml::bindableOverviewSelectedWindowId() {
	return TriadIpc::instance()->bindableOverviewSelectedWindowId();
}
QBindable<quint32> TriadIpcQml::bindableActiveTag() {
	return TriadIpc::instance()->bindableActiveTag();
}
QBindable<qint32> TriadIpcQml::bindableActiveWorkspaceIndex() {
	return TriadIpc::instance()->bindableActiveWorkspaceIndex();
}
QBindable<qint32> TriadIpcQml::bindableCurrentKeyboardLayoutIndex() {
	return TriadIpc::instance()->bindableCurrentKeyboardLayoutIndex();
}

} // namespace qs::triad
