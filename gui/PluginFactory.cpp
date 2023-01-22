#include "PluginFactory.h"
#include <QTextCodec>
#include <QFileInfo>
#include <zlib.h>
#include "md5.h"

#define BMPLUGIN_DATA_OFFSET		(sizeof(_szBMPluginMagic)-1 + sizeof(checkSum) - 1)

const char _szBMPluginMagic[] = "B1GPLUGIN100!";

PluginFactory::PluginFactory(QObject *parent) : QObject(parent)
{
	memset(checkSum, 0, sizeof(checkSum));
}

int PluginFactory::Build(QString fileName)
{
	fp = new QFile(fileName);
	if(!fp->open(QIODevice::ReadWrite))
		return(-1);
	fp->resize(0);

	// generate serialized array
	generateArray();

	// compute checksum
	computeChecksum();

	// write header
	writeHeader();

	// close
	int res = fp->size();
	fp->close();
	delete fp;

	return(res);
}

void PluginFactory::generateArray()
{
	QString workStr;
	QTextStream sArray(&workStr);
	sArray.setCodec(QTextCodec::codecForName("ISO-8859-15"));

	// meta info
	sArray << "a:2:{s:4:\"meta\";a:8:{s:4:\"name\";s:" << name.toLatin1().size() << ":\"" << name.toLatin1() << "\";"
			<< "s:7:\"version\";s:" << version.toLatin1().size() << ":\"" << version.toLatin1() << "\";"
			<< "s:11:\"for_b1gmail\";s:" << forb1gMail.toLatin1().size() << ":\"" << forb1gMail.toLatin1() << "\";"
			<< "s:6:\"vendor\";s:" << vendor.toLatin1().size() << ":\"" << vendor.toLatin1() << "\";"
			<< "s:10:\"vendor_url\";s:" << vendorURL.toLatin1().size() << ":\"" << vendorURL.toLatin1() << "\";"
			<< "s:11:\"vendor_mail\";s:" << vendorEMail.toLatin1().size() << ":\"" << vendorEMail.toLatin1() << "\";"
			<< "s:3:\"tag\";s:" << tag.toLatin1().size() << ":\"" << tag.toLatin1() << "\";";

	// meta classes
	sArray << "s:7:\"classes\";a:" << classNames.count() << ":{";
	for(int i=0; i<classNames.count(); i++)
	{
		QString className = classNames.at(i);
		sArray << "i:" << i << ";s:" << className.size() << ":\"" << className << "\";";
	}

	// plugin files
	sArray << "}}s:5:\"files\";a:5:{s:7:\"plugins\";a:" << pluginFiles.count() << ":{";
	generateFileArray(pluginFiles, sArray);

	// template files
	sArray << "}s:9:\"templates\";a:" << templateFiles.count() << ":{";
	generateFileArray(templateFiles, sArray);

	// image files
	sArray << "}s:6:\"images\";a:" << imageFiles.count() << ":{";
	generateFileArray(imageFiles, sArray);

	// css files
	sArray << "}s:3:\"css\";a:" << cssFiles.count() << ":{";
	generateFileArray(cssFiles, sArray);

	// js files
	sArray << "}s:2:\"js\";a:" << jsFiles.count() << ":{";
	generateFileArray(jsFiles, sArray);

	// finish
	sArray << "}}}";
	sArray.flush();

	// get serialized array
	QByteArray data = sArray.string()->toAscii();

	unsigned long compressedBuffLen = data.size() + data.size()/1000 + 15 + 1;
	char *compressedSerializedArray = new char[compressedBuffLen];

	// prepare zlib stream
	z_stream stream;
	stream.data_type = Z_ASCII;
	stream.zalloc = (alloc_func)Z_NULL;
	stream.zfree = (free_func)Z_NULL;
	stream.opaque = (voidpf)Z_NULL;
	stream.next_in = (Bytef *)data.constData();
	stream.next_out = (Bytef *)compressedSerializedArray;
	stream.avail_in = data.size();
	stream.avail_out = compressedBuffLen;

	// deflate
	int status = 0;
	if(deflateInit2(&stream, 9, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, 0) == Z_OK)
	{
		status = deflate(&stream, Z_FINISH);
		if(status != Z_STREAM_END)
		{
			deflateEnd(&stream);
			if(status == Z_OK)
				status = Z_BUF_ERROR;
		}
		else
		{
			status = deflateEnd(&stream);
		}
	}

	// ok?
	if(status == Z_OK)
	{
		fp->seek(BMPLUGIN_DATA_OFFSET);
		fp->write(compressedSerializedArray, stream.total_out);
	}

	delete[] compressedSerializedArray;
}

void PluginFactory::generateFileArray(QStringList &v, QTextStream &sArray)
{
	for(int i=0; i<v.size(); i++)
	{
		QFileInfo theFile(v.at(i));
		if(!theFile.exists())
			continue;

		QString fileName = theFile.fileName();

		QFile fileFP(v.at(i));
		if(fileFP.open(QIODevice::ReadOnly))
		{
			sArray << "s:" << fileName.size() << ":\"" << fileName << "\";s:" << fileFP.size() << ":\"";
			sArray << fileFP.readAll();
			sArray << "\";";

			fileFP.close();
		}
	}
}

void PluginFactory::writeHeader()
{
	fp->seek(0);
	fp->write(_szBMPluginMagic, sizeof(_szBMPluginMagic)-1);
	fp->write((const char *)this->checkSum, sizeof(this->checkSum)-1);
}

void PluginFactory::computeChecksum()
{
	// init checksum
	md5_state_t pms;
	md5_init(&pms);

	// calculate checksum
	char buffer[512];
	fp->seek(BMPLUGIN_DATA_OFFSET);
	while(fp->bytesAvailable())
	{
		size_t readSize = fp->read(buffer, sizeof(buffer));
		md5_append(&pms, (const md5_byte_t *)buffer, readSize);
	}

	// finish checksum calculation
	md5_byte_t rawCheckSum[16];
	md5_finish(&pms, rawCheckSum);

	// generate HEX checksum
	memset(checkSum, 0, sizeof(checkSum));
	char hexStr[3];
	for(unsigned int i=0; i<sizeof(rawCheckSum); i++)
	{
		snprintf(hexStr, 3, "%02x", (int)rawCheckSum[i]);
		*(checkSum+i*2) = *hexStr;
		*(checkSum+i*2+1) = *(hexStr+1);
	}
}
