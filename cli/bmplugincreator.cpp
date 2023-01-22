/*
 * BMPluginBuilder
 * Copyright (c) 2002-2022
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "bmplugincreator.h"

#include <zlib.h>

#include <sstream>

#include "md5.h"

const char _szBMPluginMagic[] = "B1GPLUGIN100!";

BMPluginCreator::BMPluginCreator(const char *fileName)
{
	if(fileName != NULL)
		this->fileName = fileName;
	memset(checkSum, 0, sizeof(this->checkSum));
}

BMPluginCreator::~BMPluginCreator()
{
}

bmPluginFileType BMPluginCreator::GetFileType(string fileName)
{
	if(fileName.length() > 4)
	{
		string extension = fileName.substr(fileName.length() - 4),
				extensionShort = fileName.substr(fileName.length() - 3);

		if(strcasecmp(extension.c_str(), ".php") == 0)
			return(BMP_FILE_PLUGIN);
		else if(strcasecmp(extension.c_str(), ".tpl") == 0)
			return(BMP_FILE_TEMPLATE);
		else if(strcasecmp(extension.c_str(), ".gif") == 0
			|| strcasecmp(extension.c_str(), ".jpg") == 0
			|| strcasecmp(extension.c_str(), ".png") == 0)
			return(BMP_FILE_IMAGE);
		else if(strcasecmp(extension.c_str(), ".css") == 0)
			return(BMP_FILE_CSS);
		else if(strcasecmp(extensionShort.c_str(), ".js") == 0)
			return(BMP_FILE_JS);
	}

	return(BMP_FILE_INVALID);
}

string BMPluginCreator::GetBaseName(string &str)
{
	size_t slashPos = str.find_last_of('/');
	if(slashPos == string::npos)
		slashPos = str.find_last_of('\\');

	if(slashPos == string::npos)
		return(str);
	else
		return(str.substr(slashPos+1));
}

void BMPluginCreator::WriteHeader()
{
	fseek(fp, 0, SEEK_SET);
	fwrite(_szBMPluginMagic, 1, sizeof(_szBMPluginMagic)-1, fp);
	fwrite(checkSum, 1, sizeof(checkSum)-1, fp);
}

void BMPluginCreator::GenerateFileArray(vector<string> &v, stringstream &sArray)
{
	for(unsigned int i=0; i<v.size(); i++)
	{
		string fileName = v.at(i),
				baseName = GetBaseName(fileName);
		size_t fileSize;
		char *fileContents;

		FILE *fileFP = fopen(fileName.c_str(), "rb");
		if(fileFP != NULL)
		{
			fseek(fileFP, 0, SEEK_END);
			fileSize = ftell(fileFP);
			fseek(fileFP, 0, SEEK_SET);
			fileContents = new char[fileSize];
			fread(fileContents, 1, fileSize, fileFP);
			fclose(fileFP);

			sArray << "s:" << baseName.size() << ":\"" << baseName << "\";s:" << fileSize << ":\"";
			sArray.write(fileContents, fileSize);
			sArray << "\";";

			delete fileContents;
		}
	}
}

void BMPluginCreator::GenerateArray()
{
	stringstream sArray;

	// meta information
	sArray << "a:2:{s:4:\"meta\";a:7:{s:4:\"name\";s:" << metaName.size() << ":\"" << metaName << "\";"
			<< "s:7:\"version\";s:" << metaVersion.size() << ":\"" << metaVersion << "\";"
			<< "s:11:\"for_b1gmail\";s:" << metaForb1gMail.size() << ":\"" << metaForb1gMail << "\";"
			<< "s:6:\"vendor\";s:" << metaVendor.size() << ":\"" << metaVendor << "\";"
			<< "s:10:\"vendor_url\";s:" << metaVendorURL.size() << ":\"" << metaVendorURL << "\";"
			<< "s:11:\"vendor_mail\";s:" << metaVendorMail.size() << ":\"" << metaVendorMail << "\";";

	// meta classes
	sArray << "s:7:\"classes\";a:" << metaClasses.size() << ":{";
	for(unsigned int i=0; i<metaClasses.size(); i++)
	{
		string metaClass = metaClasses.at(i);
		sArray << "i:" << i << ";s:" << metaClass.size() << ":\"" << metaClass << "\";";
	}

	// plugin files
	sArray << "}}s:5:\"files\";a:5:{s:7:\"plugins\";a:" << pluginFiles.size() << ":{";
	GenerateFileArray(pluginFiles, sArray);

	// template files
	sArray << "}s:9:\"templates\";a:" << templateFiles.size() << ":{";
	GenerateFileArray(templateFiles, sArray);

	// image files
	sArray << "}s:6:\"images\";a:" << imageFiles.size() << ":{";
	GenerateFileArray(imageFiles, sArray);

	// js files
	sArray << "}s:2:\"js\";a:" << jsFiles.size() << ":{";
	GenerateFileArray(jsFiles, sArray);

	// css files
	sArray << "}s:3:\"css\";a:" << cssFiles.size() << ":{";
	GenerateFileArray(cssFiles, sArray);

	// finish
	sArray << "}}}";

	// get serialized array
	string serializedArray = sArray.str();
	unsigned long compressedBuffLen = serializedArray.size() + serializedArray.size()/1000 + 15 + 1;
	char *compressedSerializedArray = new char[compressedBuffLen];

	// prepare zlib stream
	z_stream stream;
	stream.data_type = Z_ASCII;
	stream.zalloc = (alloc_func)Z_NULL;
	stream.zfree = (free_func)Z_NULL;
	stream.opaque = (voidpf)Z_NULL;
	stream.next_in = (Bytef *)serializedArray.c_str();
	stream.next_out = (Bytef *)compressedSerializedArray;
	stream.avail_in = serializedArray.size();
	stream.avail_out = compressedBuffLen;

	// deflate
	int status = -1;
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
		fwrite(compressedSerializedArray, 1, stream.total_out, fp);
	}

	delete[] compressedSerializedArray;
}

void BMPluginCreator::ComputeChecksum()
{
	// init checksum
	md5_state_t pms;
	md5_init(&pms);

	// calculate checksum
	char buffer[512];
	fseek(fp, BMPLUGIN_DATA_OFFSET, SEEK_SET);
	while(!feof(fp))
	{
		size_t readSize = fread(buffer, 1, sizeof(buffer), fp);
		md5_append(&pms, (const md5_byte_t *)buffer, readSize);
	}

	// finish checksum calculation
	md5_byte_t rawCheckSum[16];
	md5_finish(&pms, rawCheckSum);

	// generate HEX checksum
	char hexStr[3];
	memset(checkSum, 0, sizeof(checkSum));
	for(unsigned int i=0; i<sizeof(rawCheckSum); i++)
	{
		snprintf(hexStr, 3, "%02x", (int)rawCheckSum[i]);
		*(checkSum+i*2) = *hexStr;
		*(checkSum+i*2+1) = *(hexStr+1);
	}
}

int BMPluginCreator::Generate()
{
	// open file
	fp = fopen(fileName.c_str(), "wb+");
	if(fp == NULL)
		return(-1);

	// seek past header
	fseek(fp, BMPLUGIN_DATA_OFFSET, SEEK_SET);

	// generate serialized array
	GenerateArray();

	// compute checksum
	ComputeChecksum();

	// write header
	WriteHeader();

	// close
	fseek(fp, 0, SEEK_END);
	int res = (int)ftell(fp);
	fclose(fp);

	// return
	return(res);
}
