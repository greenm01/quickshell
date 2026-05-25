#include <algorithm>

#include <qbytearray.h>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qlocalsocket.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qqml.h>
#include <qqmlcomponent.h>
#include <qqmlengine.h>
#include <qqmlextensionplugin.h>
#include <qsignalspy.h>
#include <qtenvironmentvariables.h>
#include <qtest.h>
#include <qtestcase.h>
#include <qtmetamacros.h>
#include <qtypes.h>
#include <qurl.h>
#include <qvariant.h>

#include "../connection.hpp"
#include "../output.hpp"
#include "../window.hpp"
#include "../workspace.hpp"

Q_IMPORT_QML_PLUGIN(QuickshellPlugin);
Q_IMPORT_QML_PLUGIN(Quickshell_TriadPlugin);

using namespace qs::triad;

namespace {
constexpr auto TRIAD_IPC_VERSION = 1;
constexpr auto LIVE_REQUEST_TIMEOUT_MS = 1000;

QString liveSocketPath() {
	auto socket = qEnvironmentVariable("TRIAD_SOCKET");
	if (!socket.isEmpty()) return socket;

	auto runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
	if (runtimeDir.isEmpty()) return {};

	socket = QDir(runtimeDir).filePath("triad.sock");
	if (!QFileInfo::exists(socket)) return {};
	return socket;
}

QByteArray liveRequestPayload(const QString& request) {
	auto payload = QJsonObject({{"version", TRIAD_IPC_VERSION}, {"request", request}});
	return QJsonDocument(QJsonObject({{"triad", payload}})).toJson(QJsonDocument::Compact) + '\n';
}

bool readLiveRequest(const QString& socketPath, const QString& request, QJsonObject* triad) {
	auto socket = QLocalSocket();
	socket.connectToServer(socketPath, QLocalSocket::ReadWrite);
	if (!socket.waitForConnected(LIVE_REQUEST_TIMEOUT_MS)) return false;

	socket.write(liveRequestPayload(request));
	if (!socket.waitForBytesWritten(LIVE_REQUEST_TIMEOUT_MS)) return false;

	auto data = QByteArray();
	while (!data.contains('\n')) {
		if (!socket.waitForReadyRead(LIVE_REQUEST_TIMEOUT_MS)) return false;
		data.append(socket.readAll());
	}

	auto error = QJsonParseError();
	auto root = QJsonDocument::fromJson(data.left(data.indexOf('\n')), &error).object();
	if (error.error != QJsonParseError::NoError || !root.value("ok").toBool()) return false;

	*triad = root.value("triad").toObject();
	return !triad->isEmpty();
}

bool hasCommandEntry(const QVariantMap& catalog, const QString& commandName) {
	auto commands = catalog.value("commands").toList();
	return std::ranges::any_of(commands, [&commandName](const QVariant& item) {
		auto command = item.toMap();
		return command.value("name").toString() == commandName;
	});
}

QList<QVariant> requestResult(const QSignalSpy& spy, qint32 requestId) {
	for (const auto& item: spy) {
		if (item.at(0).toInt() == requestId) return item;
	}
	return {};
}

bool hasRequestResult(const QSignalSpy& spy, qint32 requestId) {
	return !requestResult(spy, requestId).isEmpty();
}
} // namespace

// Qt test macros are intentionally instance-oriented and confuse generic clang-tidy checks.
// NOLINTBEGIN(misc-include-cleaner, misc-use-internal-linkage, readability-convert-member-functions-to-static)
class TestTriadLive: public QObject {
	Q_OBJECT;

private slots:
	void initTestCase() {
		this->socketPath = liveSocketPath();
		if (this->socketPath.isEmpty()) QSKIP("No live Triad IPC socket path detected.");

		auto capabilities = QJsonObject();
		if (!readLiveRequest(this->socketPath, "capabilities", &capabilities)) {
			QSKIP("No live Triad IPC session responded to a capabilities request.");
		}

		qputenv("TRIAD_SOCKET", this->socketPath.toUtf8());
	}

	void readsLiveIpcThroughBackend() {
		auto* ipc = TriadIpc::instance();
		QTRY_VERIFY_WITH_TIMEOUT(ipc->bindableConnected().value(), 3000);

		auto spy = QSignalSpy(ipc, &TriadIpc::requestFinished);

		auto capabilitiesId = ipc->refreshCapabilities();
		QTRY_VERIFY_WITH_TIMEOUT(hasRequestResult(spy, capabilitiesId), 3000);
		auto capabilitiesResult = requestResult(spy, capabilitiesId);
		QVERIFY(capabilitiesResult.at(1).toBool());
		QVERIFY(!ipc->capabilities().isEmpty());

		auto stateId = ipc->refresh();
		QTRY_VERIFY_WITH_TIMEOUT(hasRequestResult(spy, stateId), 3000);
		auto stateResult = requestResult(spy, stateId);
		QVERIFY(stateResult.at(1).toBool());

		auto commandsId = ipc->refreshCommands();
		QTRY_VERIFY_WITH_TIMEOUT(hasRequestResult(spy, commandsId), 3000);
		auto commandsResult = requestResult(spy, commandsId);
		QVERIFY(commandsResult.at(1).toBool());

		QVERIFY(!ipc->workspaces()->valueList().empty());
		QVERIFY(!ipc->outputs()->valueList().empty());
		QVERIFY(!ipc->commandsCatalog().isEmpty());
		QVERIFY(hasCommandEntry(ipc->commandsCatalog(), QStringLiteral("focus-window")));
		QVERIFY(ipc->hasCommand(QStringLiteral("focus-window")));
		QCOMPARE(
		    ipc->commandSpec(QStringLiteral("focus-window")).value("name").toString(),
		    QStringLiteral("focus-window")
		);

		auto* focusedWindow = ipc->bindableFocusedWindow().value();
		if (focusedWindow != nullptr) {
			QVERIFY(!ipc->windows()->valueList().empty());
			QVERIFY(ipc->validateAction(
			    QStringLiteral("focus-window"),
			    {{"id", focusedWindow->bindableId().value()}}
			));
		}
	}

	void exposesLiveStateToQml() {
		auto engine = QQmlEngine();
		engine.addImportPath(QStringLiteral(QS_TEST_QML_IMPORT_PATH));

		auto component = QQmlComponent(&engine);
		component.setData(
		    R"(
import QtQml
import Quickshell.Triad

QtObject {
	property bool triadConnected: Triad.connected
	property int workspaceCount: Triad.workspaces.values.length
	property int outputCount: Triad.outputs.values.length
	property int commandCount: Triad.commandsCatalog.commands ? Triad.commandsCatalog.commands.length : 0
	property bool focusValidationOk: !Triad.focusedWindow || Triad.validateAction("focus-window", {"id": Triad.focusedWindow.id})

	Component.onCompleted: {
		Triad.refreshCapabilities()
		Triad.refresh()
		Triad.refreshCommands()
	}
}
)",
		    QUrl(QStringLiteral("qrc:/triad-live-test.qml"))
		);

		QVERIFY2(component.isReady(), qPrintable(component.errorString()));

		auto* object = component.create();
		QVERIFY2(object != nullptr, qPrintable(component.errorString()));

		QTRY_VERIFY_WITH_TIMEOUT(object->property("triadConnected").toBool(), 3000);
		QTRY_VERIFY_WITH_TIMEOUT(object->property("workspaceCount").toInt() > 0, 3000);
		QTRY_VERIFY_WITH_TIMEOUT(object->property("outputCount").toInt() > 0, 3000);
		QTRY_VERIFY_WITH_TIMEOUT(object->property("commandCount").toInt() > 0, 3000);
		QVERIFY(object->property("focusValidationOk").toBool());

		delete object;
	}

private:
	QString socketPath;
};
// NOLINTEND(misc-include-cleaner, misc-use-internal-linkage, readability-convert-member-functions-to-static)

QTEST_MAIN(TestTriadLive);
#include "triad_live.moc"
