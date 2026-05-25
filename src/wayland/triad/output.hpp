#pragma once

#include <qobject.h>
#include <qproperty.h>
#include <qqmlintegration.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qvariant.h>

namespace qs::triad {

class TriadIpc;
class TriadWorkspace;

///! Triad output state
class TriadOutput: public QObject {
	Q_OBJECT;
	// clang-format off
	/// Stable Triad output ID.
	Q_PROPERTY(quint32 id READ default NOTIFY idChanged BINDABLE bindableId);
	/// Output name.
	Q_PROPERTY(QString name READ default NOTIFY nameChanged BINDABLE bindableName);
	/// True when this is the primary output.
	Q_PROPERTY(bool primary READ default NOTIFY primaryChanged BINDABLE bindablePrimary);
	/// True when this output is connected.
	Q_PROPERTY(bool connected READ default NOTIFY connectedChanged BINDABLE bindableConnected);
	/// Output X coordinate.
	Q_PROPERTY(qint32 x READ default NOTIFY xChanged BINDABLE bindableX);
	/// Output Y coordinate.
	Q_PROPERTY(qint32 y READ default NOTIFY yChanged BINDABLE bindableY);
	/// Output width in logical pixels.
	Q_PROPERTY(qint32 width READ default NOTIFY widthChanged BINDABLE bindableWidth);
	/// Output height in logical pixels.
	Q_PROPERTY(qint32 height READ default NOTIFY heightChanged BINDABLE bindableHeight);
	/// Output scale.
	Q_PROPERTY(qreal scale READ default NOTIFY scaleChanged BINDABLE bindableScale);
	/// Refresh rate in mHz.
	Q_PROPERTY(qint32 refreshRate READ default NOTIFY refreshRateChanged BINDABLE bindableRefreshRate);
	/// Physical output width in millimeters.
	Q_PROPERTY(qint32 physicalWidth READ default NOTIFY physicalWidthChanged BINDABLE bindablePhysicalWidth);
	/// Physical output height in millimeters.
	Q_PROPERTY(qint32 physicalHeight READ default NOTIFY physicalHeightChanged BINDABLE bindablePhysicalHeight);
	/// Output transform name.
	Q_PROPERTY(QString transform READ default NOTIFY transformChanged BINDABLE bindableTransform);
	/// True when this output contains the focused workspace.
	Q_PROPERTY(bool focused READ default NOTIFY focusedChanged BINDABLE bindableFocused);
	/// Workspace currently visible on this output, or null.
	Q_PROPERTY(qs::triad::TriadWorkspace* activeWorkspace READ default NOTIFY activeWorkspaceChanged BINDABLE bindableActiveWorkspace);
	/// Last JSON object received for this output, as a JavaScript object.
	Q_PROPERTY(QVariantMap lastIpcObject READ lastIpcObject NOTIFY lastIpcObjectChanged);
	// clang-format on
	QML_ELEMENT;
	QML_UNCREATABLE("TriadOutputs must be retrieved from the Triad object.");

public:
	explicit TriadOutput(TriadIpc* ipc);

	void updateFromObject(const QVariantMap& object);
	void setFocused(bool focused);
	void setActiveWorkspace(TriadWorkspace* workspace);

	[[nodiscard]] QBindable<quint32> bindableId() { return &this->bId; }
	[[nodiscard]] QBindable<QString> bindableName() { return &this->bName; }
	[[nodiscard]] QBindable<bool> bindablePrimary() { return &this->bPrimary; }
	[[nodiscard]] QBindable<bool> bindableConnected() { return &this->bConnected; }
	[[nodiscard]] QBindable<qint32> bindableX() { return &this->bX; }
	[[nodiscard]] QBindable<qint32> bindableY() { return &this->bY; }
	[[nodiscard]] QBindable<qint32> bindableWidth() { return &this->bWidth; }
	[[nodiscard]] QBindable<qint32> bindableHeight() { return &this->bHeight; }
	[[nodiscard]] QBindable<qreal> bindableScale() { return &this->bScale; }
	[[nodiscard]] QBindable<qint32> bindableRefreshRate() { return &this->bRefreshRate; }
	[[nodiscard]] QBindable<qint32> bindablePhysicalWidth() { return &this->bPhysicalWidth; }
	[[nodiscard]] QBindable<qint32> bindablePhysicalHeight() { return &this->bPhysicalHeight; }
	[[nodiscard]] QBindable<QString> bindableTransform() { return &this->bTransform; }
	[[nodiscard]] QBindable<bool> bindableFocused() { return &this->bFocused; }
	[[nodiscard]] QBindable<TriadWorkspace*> bindableActiveWorkspace() {
		return &this->bActiveWorkspace;
	}
	[[nodiscard]] QVariantMap lastIpcObject() const;

signals:
	void idChanged();
	void nameChanged();
	void primaryChanged();
	void connectedChanged();
	void xChanged();
	void yChanged();
	void widthChanged();
	void heightChanged();
	void scaleChanged();
	void refreshRateChanged();
	void physicalWidthChanged();
	void physicalHeightChanged();
	void transformChanged();
	void focusedChanged();
	void activeWorkspaceChanged();
	void lastIpcObjectChanged();

private:
	TriadIpc* ipc;
	QVariantMap mLastIpcObject;

	// clang-format off
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadOutput, quint32, bId, 0, &TriadOutput::idChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, QString, bName, &TriadOutput::nameChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, bool, bPrimary, &TriadOutput::primaryChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadOutput, bool, bConnected, true, &TriadOutput::connectedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bX, &TriadOutput::xChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bY, &TriadOutput::yChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bWidth, &TriadOutput::widthChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bHeight, &TriadOutput::heightChanged);
	Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(TriadOutput, qreal, bScale, 1, &TriadOutput::scaleChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bRefreshRate, &TriadOutput::refreshRateChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bPhysicalWidth, &TriadOutput::physicalWidthChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, qint32, bPhysicalHeight, &TriadOutput::physicalHeightChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, QString, bTransform, &TriadOutput::transformChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, bool, bFocused, &TriadOutput::focusedChanged);
	Q_OBJECT_BINDABLE_PROPERTY(TriadOutput, TriadWorkspace*, bActiveWorkspace, &TriadOutput::activeWorkspaceChanged);
	// clang-format on
};

} // namespace qs::triad
