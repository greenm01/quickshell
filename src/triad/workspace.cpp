#include "workspace.hpp"

#include <limits>
#include <qvariant.h>

#include "connection.hpp"
#include "window.hpp"

namespace qs::triad {

namespace {
qint32 intOrInvalid(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	return value.isValid() && !value.isNull() ? value.toInt() : -1;
}

quint32 uintOrNone(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	if (!value.isValid() || value.isNull()) return 0;

	auto ok = false;
	auto parsed = value.toULongLong(&ok);
	if (!ok || parsed > std::numeric_limits<quint32>::max()) return 0;
	return static_cast<quint32>(parsed);
}

QString stringOrEmpty(const QVariantMap& object, const QString& key) {
	auto value = object.value(key);
	return value.isValid() && !value.isNull() ? value.toString() : QString();
}
} // namespace

TriadWorkspace::TriadWorkspace(TriadIpc* ipc): QObject(ipc), ipc(ipc) {}

void TriadWorkspace::updateFromObject(const QVariantMap& object) {
	this->mLastIpcObject = object;
	this->bTagId = uintOrNone(object, "tag_id");
	this->bWorkspaceIndex = intOrInvalid(object, "workspace_idx");
	this->bName = stringOrEmpty(object, "name");
	this->bOutputName = stringOrEmpty(object, "output");
	this->bLayout = stringOrEmpty(object, "layout");
	this->bLayoutKind = stringOrEmpty(object, "layout_kind");
	this->bRuntimeKind = stringOrEmpty(object, "runtime_kind");
	this->bLayoutSource = stringOrEmpty(object, "layout_source");
	this->bFallbackLayout = stringOrEmpty(object, "fallback_layout");
	this->bConfigured = object.value("is_configured").toBool();
	this->bActive = object.value("is_active").toBool();
	this->bOutputVisible = object.value("is_output_visible").toBool();
	this->bOccupied = object.value("occupied").toBool();
	this->bUrgent = object.value("is_urgent").toBool();
	this->bFocusedWindowId = uintOrNone(object, "focused_window_id");
	this->bMasterCount = intOrInvalid(object, "master_count");
	this->bMasterSplitRatio = object.value("master_split_ratio").toReal();
	this->mColumns = object.value("columns").toList();
	emit this->columnsChanged();
	this->mFrames = object.value("frames").toList();
	emit this->framesChanged();
	this->mBspNodes = object.value("bsp_nodes").toList();
	emit this->bspNodesChanged();
	this->mSplitNodes = object.value("split_nodes").toList();
	emit this->splitNodesChanged();
	this->mViewport = object.value("viewport").toMap();
	emit this->viewportChanged();
	emit this->lastIpcObjectChanged();
}

void TriadWorkspace::setFocusedWindow(TriadWindow* window) { this->bFocusedWindow = window; }

void TriadWorkspace::activate() { this->ipc->focusWorkspace(this->bWorkspaceIndex.value()); }

void TriadWorkspace::setLayout(const QString& layoutId) {
	this->ipc->setLayout(layoutId, {{"tag", this->bTagId.value()}});
}

QVariantList TriadWorkspace::columns() const { return this->mColumns; }
QVariantList TriadWorkspace::frames() const { return this->mFrames; }
QVariantList TriadWorkspace::bspNodes() const { return this->mBspNodes; }
QVariantList TriadWorkspace::splitNodes() const { return this->mSplitNodes; }
QVariantMap TriadWorkspace::viewport() const { return this->mViewport; }
QVariantMap TriadWorkspace::lastIpcObject() const { return this->mLastIpcObject; }

} // namespace qs::triad
