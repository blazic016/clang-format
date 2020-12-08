#ifndef _THIS_
#define _THIS_
#include <wx/wx.h>

#include "MainConf.h"
#include "MainFrame.h"

#define SOFT_VERSION "3.0.0"

class dmsMainCliApp
{
public:
	bool OnInit(int argc, char** argv);
	void mainLoop();
	int OnExit();
	bool CreateStream();
};


#endif
