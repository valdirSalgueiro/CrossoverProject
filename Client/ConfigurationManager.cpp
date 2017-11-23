#include "stdafx.h"
#include "ConfigurationManager.h"


ConfigurationManager::ConfigurationManager()
{
}


ConfigurationManager::~ConfigurationManager()
{
}


void ConfigurationManager::readConfiguration()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile("client.xml");
	auto client = doc.FirstChildElement("client");

	key = client->Attribute("key");
	host = client->Attribute("host");
	email = client->Attribute("mail");
	refreshInterval = client->IntAttribute("refreshInterval");
	alertInterval = client->IntAttribute("alertInterval");

	for (tinyxml2::XMLElement* e = client->FirstChildElement("alert"); e != NULL; e = e->NextSiblingElement("alert"))
	{
		alerts[e->Attribute("type")] = e->Attribute("limit");
	}
}
