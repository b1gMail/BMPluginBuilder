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

#ifndef _BMPLUGINCREATOR_H_
#define _BMPLUGINCREATOR_H_

#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

#define BMPLUGIN_DATA_OFFSET		(sizeof(_szBMPluginMagic)-1 + sizeof(checkSum)-1)

typedef enum
{
	BMP_FILE_INVALID,
	BMP_FILE_PLUGIN,
	BMP_FILE_TEMPLATE,
	BMP_FILE_IMAGE,
	BMP_FILE_CSS,
	BMP_FILE_JS
}
bmPluginFileType;

class BMPluginCreator
{
public:
	BMPluginCreator(const char *fileName = NULL);
	~BMPluginCreator();

public:
	int Generate();
	static bmPluginFileType GetFileType(string fileName);

private:
	void WriteHeader();
	void ComputeChecksum();
	void GenerateFileArray(vector<string> &v, stringstream &sArray);
	void GenerateArray();
	string GetBaseName(string &str);

public:
	string metaName,
			metaVersion,
			metaForb1gMail,
			metaVendor,
			metaVendorURL,
			metaVendorMail;
	vector<string> metaClasses,
			pluginFiles,
			templateFiles,
			imageFiles,
			jsFiles,
			cssFiles;
	string fileName;
	char checkSum[33];

private:
	FILE *fp;
};

#endif
