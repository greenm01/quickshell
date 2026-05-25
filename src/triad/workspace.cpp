#include "workspace.hpp"

#include <qvariant.h>

#include "connection.hpp"
#include "window.hpp"

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

TriadWorkspace::TriadWorkspace(TriadIpc* ipc): QObject(ipc), ipc(ipc) {}

void TriadWorkspace::updateFromObject(const QVariantMap& object) {
	this->mLastIpcObject = object;
	this->bTagId = intOrInvalid(object, "tag_id");
	this->bWorkspaceIndex = intOrInvalid(object, "workspace_idx");
	this->bName = stringOrEmpty(object, "name");
	this->bOutputName = stringOrEmpty(object, "output");
	this->bLayout = stringOrEmpty(object, "layout");
	this->bLayoutKind = stringOrEmpty(object, "layout_kind");
	this->bRuntimeKind = stringOrEmpty(object, "runtime_kind");
	this->bActive = object.value("is_active").toBool();
	this->bOutputVisible = object.value("is_output_visible").toBool();
	this->bOccupied = object.value("occupied").toBool();
	this->bUrgent = object.value("is_urgent").toBool();
	this->bFocusedWindowId = intOrInvalid(object, "focused_window_id");
	this->bMasterCount = intOrInvalid(object, "master_count");
	this->bMasterSplitRatio = object.value("master_split_ratio").toReal();
	emit this->lastIpcObjectChanged();
}

void TriadWorkspace::setFocusedWindow(TriadWindow* window) { this->bFocusedWindow = window; }

void TriadWorkspace::activate() { this->ipc->focusWorkspace(this->bWorkspaceIndex.value()); }

void TriadWorkspace::setLayout(const QString& layoutId) {
	this->ipc->setLayout(layoutId, {{"tag", this->bTagId.value()}});
}

QVariantMap TriadWorkspace::lastIpcObject() const { return this->mLastIpcObject; }

} // namespace qs::triad
