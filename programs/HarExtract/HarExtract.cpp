#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QByteArray>
#include <QUrl>

#include <iostream>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	if (argc < 2) {
		std::cerr << "Usage: HarExtract <file>\n\n";
		return -1;
	}

	QFile fileHar(QString::fromUtf8(argv[1]));
	if (!fileHar.open(QIODevice::ReadOnly)) {
		std::cerr << "Failed to open: \"" << fileHar.fileName().toUtf8().data() << "\" for reading\n\n";
		return -2;
	}
	QByteArray baHarFile(fileHar.readAll());
	fileHar.close();

	QJsonParseError jsonError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(baHarFile, &jsonError);
	if (jsonError.error != QJsonParseError::NoError) {
		std::cerr << "JSON Parse Error: " << jsonError.errorString().toUtf8().data() << "\n\n";
		return -3;
	}

	QJsonValue log = jsonDoc["log"];
	QJsonValue entries = log["entries"];
	if (!entries.isArray()) {
		std::cerr << "Couldn't find 'entries' in file\n";
		return -4;
	}

	QFile fileMD5SUMS("md5sums.txt");
	if (!fileMD5SUMS.open(QIODevice::WriteOnly | QIODevice::Append)) {
		std::cerr << "Failed to open md5sums.txt file\n";
		return -5;
	}

	QJsonArray arrEntries = entries.toArray();
	for (auto const &itr : arrEntries) {
		QJsonObject req = itr.toObject()["request"].toObject();
		QJsonObject resp = itr.toObject()["response"].toObject();
		QUrl url(req["url"].toString());

		QString strFilename = url.fileName();
		std::cout << strFilename.toUtf8().data() << "\n";

		if (QFileInfo(strFilename).suffix() != "mp3") continue;

		QString etag;
		QString contentLength;
		QJsonArray headers = resp["headers"].toArray();
		for (auto const &header : headers) {
			if (header.toObject()["name"].toString() == "Content-Length") {
				contentLength = header.toObject()["value"].toString();
			} else if (header.toObject()["name"].toString() == "ETag") {
				etag = header.toObject()["value"].toString();
			}
		}
		etag.remove('\"');

//		std::cout << "    len: " << contentLength.toUtf8().data() << "  etag: " << etag.toUtf8().data() << "\n";

		QJsonObject content = resp["content"].toObject();
		int nSize = content["size"].toInt();
		if (nSize != contentLength.toInt()) {
			std::cout << "    *** Content size: " << nSize << "  Header size: " << contentLength.toUtf8().data() << "\n";
		}
		QByteArray baContent = QByteArray::fromBase64(content["text"].toString().toUtf8().data());
		if (baContent.size() != nSize) {
			std::cout << "    *** Content size: " << nSize << "  baContent: " << baContent.size() << "\n";
		}

		QFile fileMP3(strFilename);
		if (fileMP3.open(QIODevice::WriteOnly)) {
			fileMP3.write(baContent);
			fileMP3.close();
		} else {
			std::cout << "    *** Failed to write: \"" << strFilename.toUtf8().data() << "\"\n";
		}

		fileMD5SUMS.write(QString("%1\t%2\n").arg(etag).arg(strFilename).toUtf8().data());
	}

	fileMD5SUMS.close();

	return 0;
}

