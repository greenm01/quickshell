#include "output.hpp"

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

TriadOutput::TriadOutput(TriadIpc* ipc): QObject(ipc), ipc(ipc) {}

void TriadOutput::updateFromObject(const QVariantMap& object) {
	this->mLastIpcObject = object;
	auto geometry = object.value("geometry").toMap();
	this->bId = intOrInvalid(object, "id");
	this->bName = stringOrEmpty(object, "name");
	this->bPrimary = object.value("is_primary").toBool();
	this->bConnected = object.value("connected", true).toBool();
	this->bX = geometry.value("x").toInt();
	this->bY = geometry.value("y").toInt();
	this->bWidth = geometry.value("width").toInt();
	this->bHeight = geometry.value("height").toInt();
	this->bScale = object.value("scale", 1).toReal();
	this->bRefreshRate = intOrInvalid(object, "refresh_rate");
	this->bTransform = stringOrEmpty(object, "transform");
	emit this->lastIpcObjectChanged();
}

void TriadOutput::setFocused(bool focused) { this->bFocused = focused; }

void TriadOutput::setActiveWorkspace(TriadWorkspace* workspace) {
	this->bActiveWorkspace = workspace;
}

QVariantMap TriadOutput::lastIpcObject() const { return this->mLastIpcObject; }

} // namespace qs::triad
