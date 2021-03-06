/*
 *    Copyright (C)2015 by YOUR NAME HERE
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */


/** \mainpage RoboComp::JoystickPublishFalcon
 *
 * \section intro_sec Introduction
 *
 * The JoystickPublishFalcon component...
 *
 * \section interface_sec Interface
 *
 * interface...
 *
 * \section install_sec Installation
 *
 * \subsection install1_ssec Software depencences
 * ...
 *
 * \subsection install2_ssec Compile and install
 * cd JoystickPublishFalcon
 * <br>
 * cmake . && make
 * <br>
 * To install:
 * <br>
 * sudo make install
 *
 * \section guide_sec User guide
 *
 * \subsection config_ssec Configuration file
 *
 * <p>
 * The configuration file etc/config...
 * </p>
 *
 * \subsection execution_ssec Execution
 *
 * Just: "${PATH_TO_BINARY}/JoystickPublishFalcon --Ice.Config=${PATH_TO_CONFIG_FILE}"
 *
 * \subsection running_ssec Once running
 *
 * ...
 *
 */
// QT includes
#include <QtCore>
#include <QtGui>

// ICE includes
#include <Ice/Ice.h>
#include <IceStorm/IceStorm.h>
#include <Ice/Application.h>

#include <rapplication/rapplication.h>
#include <qlog/qlog.h>

#include "config.h"
#include "genericmonitor.h"
#include "genericworker.h"
#include "specificworker.h"
#include "specificmonitor.h"
#include "commonbehaviorI.h"


#include <JoystickAdapter.h>


// User includes here

// Namespaces
using namespace std;
using namespace RoboCompCommonBehavior;

using namespace RoboCompJoystickAdapter;



class JoystickPublishFalcon : public RoboComp::Application
{
private:
	void initialize();
	std::string prefix;
	MapPrx mprx;

public:
	virtual int run(int, char*[]);
};

void JoystickPublishFalcon::initialize()
{
	// Config file properties read example
	// configGetString( PROPERTY_NAME_1, property1_holder, PROPERTY_1_DEFAULT_VALUE );
	// configGetInt( PROPERTY_NAME_2, property1_holder, PROPERTY_2_DEFAULT_VALUE );
}

int JoystickPublishFalcon::run(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);  // NON-GUI application
	int status=EXIT_SUCCESS;

	JoystickAdapterPrx joystickadapter_proxy;

	string proxy, tmp;
	initialize();


	IceStorm::TopicManagerPrx topicManager = IceStorm::TopicManagerPrx::checkedCast(communicator()->propertyToProxy("TopicManager.Proxy"));



	IceStorm::TopicPrx joystickadapter_topic;
	while (!joystickadapter_topic)
	{
		try
		{
			joystickadapter_topic = topicManager->retrieve("JoystickAdapter");
		}
		catch (const IceStorm::NoSuchTopic&)
		{
			try
			{
				joystickadapter_topic = topicManager->create("JoystickAdapter");
			}
			catch (const IceStorm::TopicExists&){
				// Another client created the topic.
			}
		}
	}
	Ice::ObjectPrx joystickadapter_pub = joystickadapter_topic->getPublisher()->ice_oneway();
	JoystickAdapterPrx joystickadapter = JoystickAdapterPrx::uncheckedCast(joystickadapter_pub);
	mprx["JoystickAdapterPub"] = (::IceProxy::Ice::Object*)(&joystickadapter);



	GenericWorker *worker = new SpecificWorker(mprx);
	//Monitor thread
	GenericMonitor *monitor = new SpecificMonitor(worker,communicator());
	QObject::connect(monitor,SIGNAL(kill()),&a,SLOT(quit()));
	QObject::connect(worker,SIGNAL(kill()),&a,SLOT(quit()));
	monitor->start();

	if ( !monitor->isRunning() )
		return status;
	try
	{
		// Server adapter creation and publication
		Ice::ObjectAdapterPtr adapterCommonBehavior = communicator()->createObjectAdapter("CommonBehavior");
		CommonBehaviorI *commonbehaviorI = new CommonBehaviorI(monitor );
		adapterCommonBehavior->add(commonbehaviorI, communicator()->stringToIdentity("commonbehavior"));
		adapterCommonBehavior->activate();





		// Server adapter creation and publication
		cout << SERVER_FULL_NAME " started" << endl;

		// User defined QtGui elements ( main window, dialogs, etc )

#ifdef USE_QTGUI
		//ignoreInterrupt(); // Uncomment if you want the component to ignore console SIGINT signal (ctrl+c).
		a.setQuitOnLastWindowClosed( true );
#endif
		// Run QT Application Event Loop
		a.exec();
		status = EXIT_SUCCESS;
	}
	catch(const Ice::Exception& ex)
	{
		status = EXIT_FAILURE;

		cout << "[" << PROGRAM_NAME << "]: Exception raised on main thread: " << endl;
		cout << ex;

#ifdef USE_QTGUI
		a.quit();
#endif
		monitor->exit(0);
}

	return status;
}

int main(int argc, char* argv[])
{
	bool hasConfig = false;
	string arg;
 JoystickPublishFalcon app;

	// Search in argument list for --Ice.Config= argument and prefix (if exist)
	for (int i = 1; i < argc; ++i)
	{
		arg = argv[i];
		if (arg.find ( "--Ice.Config=", 0 ) != string::npos )
			hasConfig = true;
		if (arg.find ( "--prefix=", 0 ) != string::npos )
		{
			hasConfig = true;
		}
	}

// 	app.prefix = 
	if (hasConfig)
	{
		return app.main(argc, argv);
	}
	else
	{
		return app.main(argc, argv, "etc/config"); // "config" is the default config file name
	}
}

