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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <string>
#include "bmplugincreator.h"

using namespace std;

void show_help(char *name)
{
	printf("Usage: %s [options] file...\n",
		name);
	printf("Options:\n");
	printf("  --help                    Display this information\n");
	printf("  --output=<file>           Save plugin to <file>\n");
	printf("  --name=<name>             Set plugin name\n");
	printf("  --version=<version>       Set plugin version\n");
	printf("  --for-b1gmail=<version>   Set required b1gMail version\n");
	printf("  --vendor=<name>           Set plugin vendor\n");
	printf("  --vendor-url=<url>        Set plugin vendor URL\n");
	printf("  --vendor-mail=<url>       Set plugin vendor mail\n");
	printf("  --class=<name>            Set plugin class name\n");
	printf("\n");
	printf("All options except --help and at least one file must be specified.\n");
}

int main (int argc, char * const argv[])
{
	BMPluginCreator creator;
	int c, help_flag = 0;

	printf("b1gMail Plugin Builder\n");
	printf("(c) 2002-2010 B1G Software\n\n");

	// show help if no arguments are specified
	if(argc == 1)
	{
		show_help(argv[0]);
		return(0);
	}

	//
	// parse args
	//
	while(true)
	{
		static struct option long_options[] =
		{
			{ "help",			no_argument,		&help_flag,			1	},
			{ "output",			required_argument,	NULL,				'o'	},
			{ "name",			required_argument,	NULL,				'n'	},
			{ "version",		required_argument,	NULL,				'v'	},
			{ "for-b1gmail",	required_argument,	NULL,				'f'	},
			{ "vendor",			required_argument,	NULL,				'h' },
			{ "vendor-url",		required_argument,	NULL,				'u' },
			{ "vendor-mail",	required_argument,	NULL,				'm' },
			{ "class",			required_argument,	NULL,				'c' },
			{ 0,				0,					0,					0	}
		};

		int option_index = 0;
		if((c = getopt_long(argc, argv, "o:n:v:h:u:m:c:", long_options, &option_index)) == -1)
			break;

		switch(c)
		{
		case 0:
			break;

		case 'o':
			creator.fileName = optarg;
			break;

		case 'n':
			creator.metaName = optarg;
			break;

		case 'v':
			creator.metaVersion = optarg;
			break;

		case 'f':
			creator.metaForb1gMail = optarg;
			break;

		case 'h':
			creator.metaVendor = optarg;
			break;

		case 'u':
			creator.metaVendorURL = optarg;
			break;

		case 'm':
			creator.metaVendorMail = optarg;
			break;

		case 'c':
			creator.metaClasses.push_back(optarg);
			break;

		default:
            return(1);
		}
	}

	//
	// help?
	//
	if(help_flag == 1)
	{
		show_help(argv[0]);
		return(0);
	}

	//
	// parse files
	//
	if(optind < argc)
	{
		while(optind < argc)
		{
			char *fileName = argv[optind++];

			// check if file exists
			static struct stat st;
			if(stat(fileName, &st) != 0)
			{
				printf("%s: file not found: `%s'\n",
					argv[0],
					fileName);
				return(1);
			}

			// check file type
			bmPluginFileType fileType = BMPluginCreator::GetFileType(fileName);
			if(fileType == BMP_FILE_INVALID)
			{
				printf("%s: unrecognized file type `%s'\n",
					argv[0],
					fileName);
				return(1);
			}
			else if(fileType == BMP_FILE_PLUGIN)
				creator.pluginFiles.push_back(fileName);
			else if(fileType == BMP_FILE_TEMPLATE)
				creator.templateFiles.push_back(fileName);
			else if(fileType == BMP_FILE_IMAGE)
				creator.imageFiles.push_back(fileName);
			else if(fileType == BMP_FILE_JS)
				creator.jsFiles.push_back(fileName);
			else if(fileType == BMP_FILE_CSS)
				creator.cssFiles.push_back(fileName);
		}
	}

	//
	// check required parameters
	//
	if(creator.pluginFiles.size() == 0)
	{
		printf("%s: no plugin files specified\n",
			argv[0]);
		return(1);
	}
	else if(creator.metaClasses.size() == 0)
	{
		printf("%s: no plugin classes specified\n",
			argv[0]);
		return(1);
	}
	else if(creator.fileName.size() == 0)
	{
		printf("%s: no output file name specified\n",
			argv[0]);
		return(1);
	}
	else if(creator.metaName.size() == 0
		|| creator.metaVersion.size() == 0
		|| creator.metaForb1gMail.size() == 0
		|| creator.metaVendor.size() == 0
		|| creator.metaVendorURL.size() == 0
		|| creator.metaVendorMail.size() == 0)
	{
		printf("%s: one or more missing meta parameters\n",
			argv[0]);
		return(1);
	}

	//
	// generate plugin
	//
	int pluginSize = creator.Generate();
	if(pluginSize <= 0)
	{
		printf("%s: plugin file `%s' cannot be built\n",
			argv[0],
			creator.fileName.c_str());
		return(1);
	}
	else
	{
		printf("Plugin `%s' successfuly built (size=%d, md5=%s)\n",
			creator.fileName.c_str(),
			pluginSize,
			creator.checkSum);
		return(0);
	}
}
