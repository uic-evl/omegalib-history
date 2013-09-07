/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The omegalib entry point (main), initialization and shutdown code, plus a
 *	set of system utility functions.
 ******************************************************************************/
#include "omega/osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"
#include "omega/Engine.h"
#include "omicron/StringUtils.h"
#include "omega/MissionControl.h"

#include <iostream>

#ifndef WIN32
	#include <unistd.h>
	#include<sys/wait.h>
#endif

#ifdef OMEGA_OS_WIN
    #include <direct.h>
    #define GetCurrentDir _getcwd
	// Needed for GetModuleFileName
	#include <Windows.h>
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

namespace omega
{
	///////////////////////////////////////////////////////////////////////////
	libconfig::ArgumentHelper sArgs;
	GLEWContext* sGlewContext;

	///////////////////////////////////////////////////////////////////////////
	GLEWContext* glewGetContext()
	{
		return sGlewContext;
	}

	///////////////////////////////////////////////////////////////////////////
	void glewSetContext(const GLEWContext* context)
	{
		sGlewContext = (GLEWContext*)context;
	}

	///////////////////////////////////////////////////////////////////////////
	OMEGA_API libconfig::ArgumentHelper& oargs()
	{
		return sArgs;
	}

	///////////////////////////////////////////////////////////////////////////
	extern "C" void abortHandler(int signal_number)
	{
		// Just exit.
		exit(-1);
	}

	///////////////////////////////////////////////////////////////////////////
	extern "C" void sigproc(int signal_number)
	{ 		 
		DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();

		// Explicitly kill sound server
		SoundEnvironment* se = Engine::instance()->getSoundEnvironment();
		if(se != NULL)
		{
			se->getSoundManager()->stopAllSounds();
			se->getSoundManager()->cleanupAllSounds();
		}

		ds->killCluster();
		
		osleep(2000);
	}

	///////////////////////////////////////////////////////////////////////////
	void setupMultiInstance(SystemManager* sys, const String& multiAppString)
	{
		Vector<String> args = StringUtils::split(multiAppString, ",");
		if(args.size() < 4)
		{
			ofwarn("Invalid number of arguments for -I option '%1%'. 4-5 expected: <tilex>,<tiley>,<tilewidth>,<tileHeight>[,portPool = 100]", %multiAppString);
		}
		else
		{
			MultiInstanceConfig& mic = sys->getMultiInstanceConfig();

			mic.enabled = true;
			mic.tilex = boost::lexical_cast<int>(args[0]);
			mic.tiley = boost::lexical_cast<int>(args[1]);
			mic.tilew = boost::lexical_cast<int>(args[2]);
			mic.tileh = boost::lexical_cast<int>(args[3]);

			if(args.size() == 5) mic.portPool = boost::lexical_cast<int>(args[4]);
			else mic.portPool = 100;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	int omain(omega::ApplicationBase& app, int argc, char** argv)
	{
		// register the abort handler.
		signal(SIGABRT, &abortHandler);

#ifdef OMEGA_ENABLE_VLD
		// Mark everything before this point as already reported to avoid 
		// reporting static global objects as leaks. This makes the report less
		// precise but gets rid of a lot of noise.
		VLDMarkAllLeaksAsReported();
#endif
		{
			bool remote = false;
			String masterHostname;
			String configFilename = ostr("%1%.cfg", %app.getName());
			String multiAppString = "";
			String mcmode = "default";
#ifdef OMEGA_APPROOT_DIRECTORY
			String dataPath = OMEGA_APPROOT_DIRECTORY;
#else
			String dataPath = "";
#endif
			String logFilename = ostr("%1%.log", %app.getName());

			bool kill = false;
			bool help = false;
			bool disableSIGINTHandler = false;

			bool logRemoteNodes = false;

			sArgs.newNamedString(
				'c',
				"config", 
				ostr("configuration file to use with this application (default: %1% or default.cfg if the previous is not found)", %configFilename).c_str(), "",
				configFilename);

			sArgs.newFlag(
				'K',
				"kill",
				"Don't run the application, only run the nodeKiller command on all nodes in a clustered configuration",
				kill);

			sArgs.newFlag(
				'?',
				"help",
				"Prints this application help screen",
				help);

			sArgs.newFlag(
				'r',
				"log-remote",
				"generate log for remote nodes",
				logRemoteNodes);
				
			sArgs.newNamedString(
				'D',
				"data",
				"Data path for this application", "",
				dataPath);
		
			sArgs.newNamedString(
				'L',
				"log",
				ostr("log file to use with this application (default: %1%)", %logFilename).c_str(), "",
				logFilename);

			sArgs.newNamedString(
				'I',
				"instance",
				"Enable multi-instance mode and set global viewport and port pool as a string <tilex>,<tiley>,<tilewidth>,<tileHeight>,<portPool>", "",
				multiAppString);

			sArgs.newNamedString(
				'm',
				"mc",
				"Sets mission control mode. (default, server, disable) ", "In default mode, the application opens a mission control server if enabled in the configuration file. ",
				mcmode);

			sArgs.newFlag(
				'd',
				"disable-sigint",
				"disables the Control+C handler (useful when debugging with gdb)",
				disableSIGINTHandler);
				
			sArgs.setName("omegalib");
			sArgs.setAuthor("The Electronic Visualization Lab, UIC");
			String appName;
			String extName;
			String pathName;
			StringUtils::splitFullFilename(app.getName(), appName, extName, pathName);
			sArgs.setDescription(appName.c_str());
			sArgs.setName(appName.c_str());
			sArgs.setVersion(OMEGA_VERSION);
			
			// If argument processing fails, exit immediately.
			if(!sArgs.process(argc, argv))
			{
				return -1;
			}
			
			if(!disableSIGINTHandler)
			{
				//omsg("Registering Control-C SIGINT handler");
				signal(SIGINT, sigproc);
			}

			if(help)
			{
				sArgs.writeUsage(std::cout);
				return 0;
			}

			std::vector<std::string> args = StringUtils::split(configFilename, "@");
			configFilename = args[0];
			if(args.size() == 2)
			{
				remote = true;
				masterHostname = args[1];
			
				// If logging on remote nodes is enabled, set up an output file using the app + node name.
				// otherwise, disable logging.
				if(logRemoteNodes)
				{
					omsg("Remote node logging enabled");
					String hostLogFilename = masterHostname + "-" + logFilename;
					ologopen(hostLogFilename.c_str());
				}
				else
				{
					ologdisable();
				}
			}
			else
			{
				ologopen(logFilename.c_str());
			}
		
			SystemManager* sys = SystemManager::instance();
			DataManager* dm = sys->getDataManager();
			
			omsg("omegalib data search paths:");
			String cwd = ogetcwd();
			ofmsg("::: %1%", %cwd);

			// Add some default filesystem search paths: 
			// - an empty search path for absolute paths
			// - the current directory
			// - the omegalib applications root path (if exists)
			// - the default omegalib data path
			dm->addSource(new FilesystemDataSource("./"));
			dm->addSource(new FilesystemDataSource(""));

#ifdef OMEGA_HARDCODE_DATA_PATHS
			dm->addSource(new FilesystemDataSource(dataPath));
			ofmsg("::: %1%", %dataPath);
			dm->addSource(new FilesystemDataSource(OMEGA_DATA_PATH));
			ofmsg("::: %1%", %OMEGA_DATA_PATH);
			dm->addSource(new FilesystemDataSource(OMEGA_BINARY_PATH));
			ofmsg("::: %1%", %OMEGA_DATA_PATH);
#endif
			omsg("omegalib application config lookup:");
			String curCfgFilename = ostr("%1%/%2%", %app.getName() %configFilename);
			ofmsg("::: trying %1%", %curCfgFilename);
			String path;
			if(!DataManager::findFile(curCfgFilename, path))
			{
				curCfgFilename = configFilename;
				ofmsg("::: not found, trying %1%", %curCfgFilename);

				if(!DataManager::findFile(curCfgFilename, path))
				{
					curCfgFilename = "default.cfg";
					ofmsg("::: not found, trying %1%", %curCfgFilename);
					if(!DataManager::findFile(curCfgFilename, path))
					{
						oerror("FATAL: Could not load default.cfg. Aplication will exit now.");
						return -1;
					}
				}
			}

			ofmsg("::: found config: %1%", %curCfgFilename);

			Config* cfg = new Config(curCfgFilename);
			
			// If multiApp string is set, setup multi-application mode.
			// In multi-app mode, this instance will output to a subset of the available tiles, and will choose a
			// communication port using a port interval starting at the configuration base port plus and dependent on a port pool.
			if(multiAppString != "") setupMultiInstance(sys, multiAppString);

			if(kill)
			{
				sys->setApplication(&app);
				sys->setupConfig(cfg);
				sys->setupDisplaySystem();
				DisplaySystem* ds = sys->getDisplaySystem();
				ds->killCluster();
			}
			else
			{
				omsg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> OMEGALIB BOOT");
				sys->setApplication(&app);
				if(remote)
				{
					sys->setupRemote(cfg, masterHostname);
				}
				else
				{
					sys->setup(cfg);
					sys->setupMissionControl(mcmode);
				}

				sys->initialize();
				omsg("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OMEGALIB BOOT\n\n");

				sys->run();

				omsg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> OMEGALIB SHUTDOWN");
				sys->cleanup();
				omsg("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OMEGALIB SHUTDOWN\n\n");

				omsg("===================== ReferenceType object leaks follow:");
				ReferenceType::printObjCounts();
			}

			ologclose();
		}

#ifdef OMEGA_ENABLE_VLD
		_cexit();
		VLDReportLeaks();
#endif

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////
	bool olaunch(const String& command)
	{
		if( command.empty( )) return false;

#ifdef OMEGA_OS_WIN
		STARTUPINFO         startupInfo;
		ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

		PROCESS_INFORMATION procInfo;
		ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

		const char*         cmdLine     = command.c_str();

		startupInfo.cb = sizeof( STARTUPINFO );
		const bool success = 
			CreateProcess( 0, LPSTR( cmdLine ), // program, command line
						   0, 0,                // process, thread attributes
						   FALSE,               // inherit handles
						   0,                   // creation flags
						   0,                   // environment
						   0,                   // current directory
						   &startupInfo,
						   &procInfo );

		//WaitForInputIdle( procInfo.hProcess, 1000 );
		CloseHandle( procInfo.hProcess );
		CloseHandle( procInfo.hThread );

		return true;
#else
		std::vector<std::string> commandLine = StringUtils::split(command, " ");

		// NOTE 28Jul13: This has been changed to SIG_IGN instead of a custom 
		// handler to avoid a weird deadlock with glXCreateContext on SUSE 12.3
		// glXCreateContext invoked (thorugh the nvidia driver) a waitpid that apparently
		// waits for the same child process generated here. Note that the child 
		// handler just waits for the process to exit cleanly to avoid 
		// creating zombies, which can be done just by specifying SIG_IGN as
		// the handler.
		signal( SIGCHLD, SIG_IGN );
		const int result = fork();
		switch( result )
		{
			case 0: // child
				break;

			case -1: // error
				ofwarn("Launching command %1% failed.", %command);
				return false;
			default: // parent
				return true;
		}

		// child
		const size_t  argc         = commandLine.size();
		char*         argv[argc+1];
		std::ostringstream stringStream;

		for( size_t i=0; i<argc; i++ )
		{
			argv[i] = (char*)commandLine[i].c_str();
			stringStream << commandLine[i] << " ";
		}

		argv[argc] = 0;

		//ofmsg("Executing: %1%", %command);
		int nTries = 10;
		while( nTries-- )
		{
			execvp( argv[0], argv );
			ofwarn("Error executing %1%", %command);
			// EQWARN << "Error executing '" << argv[0] << "': " << sysError
				   // << std::endl;
			if( errno != ETXTBSY )
				break;
		}

		// Launch failed. Crash and burn.
		exit(-1);
		return false;
#endif
	}

	///////////////////////////////////////////////////////////////////////////
	String ogetcwd()
	{
		char cCurrentPath[FILENAME_MAX];
		GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));
		return cCurrentPath;
	}

	///////////////////////////////////////////////////////////////////////////
	String ogetexecpath()
	{
		char path[2048];
		path[0] = '\0';
#ifdef OMEGA_OS_LINUX
		readlink("/proc/self/exe", path, 2048);
#elif defined OMEGA_OS_WIN
		GetModuleFileName(NULL, path, 2048);
#else
		owarn("OSX NOT IMPLEMENTED: (osystem.cpp) ogetexecpath");
		owarn("Imlement using _NSGetExecutablePath()");
#endif	
		return path;
	}
	
	String _dataPrefix;

	///////////////////////////////////////////////////////////////////////////
	void osetdataprefix(const String& data)
	{
		_dataPrefix = data;
	}

	///////////////////////////////////////////////////////////////////////////
	String ogetdataprefix()
	{
		return _dataPrefix;
	}
}
