#include "output.hpp"

#include <limits>
#include <qvariant.h>

#include "connection.hpp"

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

TriadOutput::TriadOutput(TriadIpc* ipc): QObject(ipc), ipc(ipc) {}

void TriadOutput::updateFromObject(const QVariantMap& object) {
	this->mLastIpcObject = object;
	auto geometry = object.value("geometry").toMap();
	this->bId = uintOrNone(object, "id");
	this->bName = stringOrEmpty(object, "name");
	this->bPrimary = object.value("is_primary").toBool();
	this->bConnected = object.value("connected", true).toBool();
	this->bX = geometry.value("x").toInt();
	this->bY = geometry.value("y").toInt();
	this->bWidth = geometry.value("width").toInt();
	this->bHeight = geometry.value("height").toInt();
	this->bScale = object.value("scale", 1).toReal();
	this->bRefreshRate = intOrInvalid(object, "refresh_rate");
	this->bPhysicalWidth = intOrInvalid(object, "physical_width");
	this->bPhysicalHeight = intOrInvalid(object, "physical_height");
	this->bTransform = stringOrEmpty(object, "transform");
	emit this->lastIpcObjectChanged();
}

void TriadOutput::setFocused(bool focused) { this->bFocused = focused; }

void TriadOutput::setActiveWorkspace(TriadWorkspace* workspace) {
	this->bActiveWorkspace = workspace;
}

QVariantMap TriadOutput::lastIpcObject() const { return this->mLastIpcObject; }

} // namespace qs::triad
