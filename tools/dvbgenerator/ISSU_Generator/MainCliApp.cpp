

#include <wx/wx.h>
#include <wx/config.h>

#include <Tools/Tools.h>
#include <ToolsGui/Window.h>

#include "MainCliApp.h"

#include <regex>
#include <string>
#include <iostream>


dmsMainCli* g_poMainCli;
bool dmsMainCliApp::OnInit(int argc, char **argv)
{

    g_poLogManager = new dmsLogManager();


    if((argc == 3 || argc == 4) && !strcmp(argv[1], "-f"))
    {
    	g_poMainConf = new dmsCliConf(std::string(argv[2]));
    }
    else
    {
    	g_poMainConf = new dmsCliConf;
    }

    if(!static_cast<dmsCliConf*>(g_poMainConf)->isFileOpen())
    {
    	return false;
    }

	g_poMainConf->Load();

	g_poMainCli = new dmsMainCli("LALA");
    if (g_poMainCli->m_poManager->Find("ROOT"))
    {
        dmsvEnum* map = (dmsvEnum*) g_poMainCli->m_poManager->m_poValidatorManager->Find("Profile");

        g_poMainConf->GetGroupList(map->m_oValueNames);
        
        map->AssumeValid(g_poMainConf->m_iProfile);
        
        
        map->SetIndex(g_poMainConf->m_iProfile);
        
        g_poMainConf->Load(map->Get(), *g_poMainCli->m_poManager->m_poValidatorManager);

        if(argc == 4)
        {
			static_cast<dmsCliConf*>(g_poMainConf)->openConfig(std::string(argv[3]));
			g_poMainConf->Load(map->Get(), *g_poMainCli->m_poManager->m_poValidatorManager);
			g_poMainConf->Save();
		}

        g_poMainCli->m_poManager->m_poValidatorManager->InitReset();
        
        g_poMainCli->m_poManager->Get();

    }

    g_poMainCli->m_poManager->m_poValidatorManager->ClearErrors();

    return true;
}


int dmsMainCliApp::OnExit()
{
    if(g_poMainConf)
    	delete g_poMainConf;
    return 0;
}

void dmsMainCliApp::mainLoop() 
{
    std::string input;
   	std::regex regex_set("set ([a-zA-Z0-9]+) ([./a-zA-Z0-9]+)");
   	std::regex regex_get("get ([a-zA-Z0-9]+)");
   	std::smatch match;
	while(true) {
		std::cout << ">";
		std::getline(std::cin, input);

		if(!std::cin)break;

		if(input == std::string("CreateStream")) {

			g_poMainCli->CreateStream();
		}
		else if(std::regex_match(input, regex_set)) {

			std::regex_search(input, match, regex_set);
			auto* res = g_poMainCli->m_poManager->m_poValidatorManager->Find(std::string(match[1]).c_str());

			if(res)
				res->Set(std::string(match[2]).c_str());
			else
				std::cout << std::string(match[1]) << " Not found " << std::endl;
		}
		else if(std::regex_match(input, regex_get)) {
			std::regex_search(input, match, regex_get);
			auto* res = g_poMainCli->m_poManager->m_poValidatorManager->Find(std::string(match[1]).c_str());
			if(res)
				std::cout << res->Get() << std::endl;
			else
				std::cout << std::string(match[1]) << " Not found " << std::endl;
		}

	}
}


bool dmsMainCliApp::CreateStream()
{
	return g_poMainCli->CreateStream();
}


int main(int argc, char** argv)
{
    dmsMainCliApp app;
    if(app.OnInit(argc, argv))
    	app.CreateStream();
    return app.OnExit();
}
