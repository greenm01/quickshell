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
	this->bParentId = intOrInvalid(object, "parent_id");
	this->bTitle = stringOrEmpty(object, "title");
	this->bAppId = stringOrEmpty(object, "app_id");
	this->bTagId = intOrInvalid(object, "tag_id");
	this->bWorkspaceIndex = intOrInvalid(object, "workspace_idx");
	this->bOutputName = stringOrEmpty(object, "output");
	auto position = object.value("position").toMap();
	this->bColumnIndex = intOrInvalid(position, "column_idx");
	this->bWindowIndex = intOrInvalid(position, "window_idx");
	this->bFocused = object.value("is_focused").toBool();
	this->bFloating = object.value("is_floating").toBool();
	this->bMaximized = object.value("is_maximized").toBool();
	this->bMinimized = object.value("is_minimized").toBool();
	this->bFullscreen = object.value("is_fullscreen").toBool();
	this->bSticky = object.value("is_sticky").toBool();
	this->bOverlay = object.value("is_overlay").toBool();
	this->bUnmanagedGlobal = object.value("is_unmanaged_global").toBool();
	this->bFullscreenOutput = intOrInvalid(object, "fullscreen_output");
	this->bWidthProportion = object.value("width_proportion").toReal();
	this->bHeightProportion = object.value("height_proportion").toReal();
	auto actualSize = object.value("actual_size").toMap();
	this->bActualWidth = actualSize.value("width").toInt();
	this->bActualHeight = actualSize.value("height").toInt();
	auto floatingGeometry = object.value("floating_geometry").toMap();
	this->bFloatingX = floatingGeometry.value("x").toInt();
	this->bFloatingY = floatingGeometry.value("y").toInt();
	this->bFloatingWidth = floatingGeometry.value("width").toInt();
	this->bFloatingHeight = floatingGeometry.value("height").toInt();
	this->bKeyboardShortcutsInhibit = object.value("keyboard_shortcuts_inhibit").toBool();
	this->bIdleInhibit = stringOrEmpty(object, "idle_inhibit");
	this->bTerminal = object.value("is_terminal").toBool();
	this->bAllowSwallow = object.value("allow_swallow").toBool();
	this->bSwallowedBy = intOrInvalid(object, "swallowed_by");
	this->bSwallowing = intOrInvalid(object, "swallowing");
	emit this->lastIpcObjectChanged();
}

void TriadWindow::focus() { this->ipc->focusWindow(this->bId.value()); }

void TriadWindow::close() { this->ipc->closeWindow(this->bId.value()); }

QVariantMap TriadWindow::lastIpcObject() const { return this->mLastIpcObject; }

} // namespace qs::triad
