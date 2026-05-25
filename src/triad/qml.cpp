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

void TriadIpcQml::refresh() { TriadIpc::instance()->refresh(); }
void TriadIpcQml::refreshLayout() { TriadIpc::instance()->refreshLayout(); }
void TriadIpcQml::refreshWindows() { TriadIpc::instance()->refreshWindows(); }
qint32 TriadIpcQml::sendRequest(const QString& request, const QVariantMap& payload) {
	return TriadIpc::instance()->sendRequest(request, payload);
}
qint32 TriadIpcQml::sendAction(const QString& action, const QVariantMap& payload) {
	return TriadIpc::instance()->sendAction(action, payload);
}
void TriadIpcQml::dispatch(const QString& action, const QVariantMap& payload) {
	TriadIpc::instance()->dispatch(action, payload);
}
void TriadIpcQml::focusWorkspace(qint32 workspaceIndex) {
	TriadIpc::instance()->focusWorkspace(workspaceIndex);
}
void TriadIpcQml::focusTag(qint32 tagId) { TriadIpc::instance()->focusTag(tagId); }
void TriadIpcQml::focusWindow(qint32 windowId) { TriadIpc::instance()->focusWindow(windowId); }
void TriadIpcQml::closeWindow(qint32 windowId) { TriadIpc::instance()->closeWindow(windowId); }
void TriadIpcQml::switchLayout() { TriadIpc::instance()->switchLayout(); }
void TriadIpcQml::setLayout(const QString& layoutId, const QVariantMap& target) {
	TriadIpc::instance()->setLayout(layoutId, target);
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
