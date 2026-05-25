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
qint32 TriadIpcQml::focusTag(qint32 tagId) { return TriadIpc::instance()->focusTag(tagId); }
qint32 TriadIpcQml::focusWindow(qint32 windowId) {
	return TriadIpc::instance()->focusWindow(windowId);
}
qint32 TriadIpcQml::closeWindow(qint32 windowId) {
	return TriadIpc::instance()->closeWindow(windowId);
}
qint32 TriadIpcQml::switchLayout() { return TriadIpc::instance()->switchLayout(); }
qint32 TriadIpcQml::setLayout(const QString& layoutId, const QVariantMap& target) {
	return TriadIpc::instance()->setLayout(layoutId, target);
}
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
QBindable<qint32> TriadIpcQml::bindableOverviewSelectedWindowId() {
	return TriadIpc::instance()->bindableOverviewSelectedWindowId();
}
QBindable<qint32> TriadIpcQml::bindableActiveTag() {
	return TriadIpc::instance()->bindableActiveTag();
}
QBindable<qint32> TriadIpcQml::bindableActiveWorkspaceIndex() {
	return TriadIpc::instance()->bindableActiveWorkspaceIndex();
}
QBindable<qint32> TriadIpcQml::bindableCurrentKeyboardLayoutIndex() {
	return TriadIpc::instance()->bindableCurrentKeyboardLayoutIndex();
}

} // namespace qs::triad
