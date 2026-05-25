#include "window.hpp"

#include <qvariant.h>

#include "connection.hpp"

namespace qs::triad {

namespace {
qint32 intOrInvalid(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	return value.isValid() && !value.isNull() ? value.toInt() : -1;
}

QString stringOrEmpty(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	return value.isValid() && !value.isNull() ? value.toString() : QString();
}
} // namespace

TriadWindow::TriadWindow(TriadIpc* ipc): QObject(ipc), ipc(ipc) {}

void TriadWindow::updateFromObject(const QVariantMap& object) {
	this->mLastIpcObject = object;
	this->bId = intOrInvalid(object, "id");
	this->bPid = intOrInvalid(object, "pid");
	this->bTitle = stringOrEmpty(object, "title");
	this->bAppId = stringOrEmpty(object, "app_id");
	this->bTagId = intOrInvalid(object, "tag_id");
	this->bWorkspaceIndex = intOrInvalid(object, "workspace_idx");
	this->bOutputName = stringOrEmpty(object, "output");
	this->bFocused = object.value("is_focused").toBool();
	this->bFloating = object.value("is_floating").toBool();
	this->bMaximized = object.value("is_maximized").toBool();
	this->bMinimized = object.value("is_minimized").toBool();
	this->bFullscreen = object.value("is_fullscreen").toBool();
	this->bTerminal = object.value("is_terminal").toBool();
	emit this->lastIpcObjectChanged();
}

void TriadWindow::focus() { this->ipc->focusWindow(this->bId.value()); }

void TriadWindow::close() { this->ipc->closeWindow(this->bId.value()); }

QVariantMap TriadWindow::lastIpcObject() const { return this->mLastIpcObject; }

} // namespace qs::triad
