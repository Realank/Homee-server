
#include <stdio.h>
#include <stdlib.h>
#include"tinyxml2.h"
#include "server.h"
#include <time.h>

using namespace std;
using namespace tinyxml2;

XMLDocument doc;
XMLElement* weakDate[4];
XMLElement* dayWeather[4];
XMLElement* dayWind[4];
XMLElement* dayTemp[4];
XMLElement* pm25;



int readWeather()
{
	
	const char* content;
	doc.LoadFile("./weather.xml");
	XMLElement *CityWeatherResponse=doc.RootElement();
	XMLElement *error=CityWeatherResponse->FirstChildElement("error");
    if (strncmp(error->GetText(),"0",1)) {
        //error
        return -1;
    }
	XMLElement *status=error->NextSiblingElement();
	XMLElement *date=status->NextSiblingElement();
	XMLElement *results=date->NextSiblingElement("results");
	
	if(results)
	{
	
		XMLElement *currentCity=results->FirstChildElement("currentCity");
		XMLElement *weather_data=currentCity->NextSiblingElement();
		if(weather_data)
		{
            //time_t now = time(NULL);
            //struct tm *localTimeNow = localtime(&now);
            
			weakDate[0]=weather_data->FirstChildElement("date");
			XMLElement *d1_dayPictureUrl=weakDate[0]->NextSiblingElement();
			XMLElement *d1_nightPictureUrl=d1_dayPictureUrl->NextSiblingElement();
			dayWeather[0]=d1_nightPictureUrl->NextSiblingElement();
			dayWind[0]=dayWeather[0]->NextSiblingElement();
			dayTemp[0]=dayWind[0]->NextSiblingElement();
                       
			
			weakDate[1]=weakDate[0]->NextSiblingElement("date");
			XMLElement *d2_dayPictureUrl=weakDate[1]->NextSiblingElement();
			XMLElement *d2_nightPictureUrl=d2_dayPictureUrl->NextSiblingElement();
			dayWeather[1]=d2_nightPictureUrl->NextSiblingElement();
			dayWind[1]=dayWeather[1]->NextSiblingElement();
			dayTemp[1]=dayWind[1]->NextSiblingElement();
			
			weakDate[2]=weakDate[1]->NextSiblingElement("date");
			XMLElement *d3_dayPictureUrl=weakDate[2]->NextSiblingElement();
			XMLElement *d3_nightPictureUrl=d3_dayPictureUrl->NextSiblingElement();
			dayWeather[2]=d3_nightPictureUrl->NextSiblingElement();
			dayWind[2]=dayWeather[2]->NextSiblingElement();
			dayTemp[2]=dayWind[2]->NextSiblingElement();
			
			weakDate[3]=weakDate[2]->NextSiblingElement("date");
			XMLElement *d4_dayPictureUrl=weakDate[3]->NextSiblingElement();
			XMLElement *d4_nightPictureUrl=d4_dayPictureUrl->NextSiblingElement();
			dayWeather[3]=d4_nightPictureUrl->NextSiblingElement();
			dayWind[3]=dayWeather[3]->NextSiblingElement();
			dayTemp[3]=dayWind[3]->NextSiblingElement();
			
			/*printf("%s\n%s\n%s\n%s %s %s\n",date4->GetText(),d4_dayPictureUrl->GetText(),d4_nightPictureUrl->GetText(),d4_weather->GetText(),d4_wind->GetText(),d4_temperature->GetText());*/
			
			
		
		}
		pm25=results->FirstChildElement("pm25");
	}
	
    return 0;
}
void updateXMLfile()
{
    static time_t lastUpdate=time(NULL);
    int updateFlag = 0;
    time_t now = time(NULL);
    int diffTime = (int)difftime(now,lastUpdate);
    static int firstInvoke = 0;
    firstInvoke ++;
    if (diffTime > UPDATE_XML_INTERVAL )
    {
        updateFlag = 1;
        
        printf("Need Update XML timediff : %d\n",diffTime);
    }
    else
    {
        struct tm *localTimeLastUp = localtime(&lastUpdate);
        struct tm *localTimeNow = localtime(&now);
        if(localTimeLastUp->tm_mday!=localTimeNow->tm_mday)
        {
            updateFlag = 1;
        }
        
    }
    
    if (updateFlag || firstInvoke==1) 
	{
        
        system("wget -nv \"http://api.map.baidu.com/telematics/v3/weather?location=天津&ak=FD4bc79c3f082216d5ccd573756fd65a\" -O weather.xml ");
        lastUpdate = now;
    }
    
    
}


extern "C" void weatherCheck(char cmd[socketCMDnum][CMDperLenth+1],char back[BUFFER_SIZE])
{
    int i;
    updateXMLfile();
    
    if(readWeather()==-1)
    {
        sprintf(back,"XML data error\0");
        return;
    }
    
    if (!strncmp(cmd[1],CHECK,CMDperLenth)) 
	{
        
        int i=atoi(cmd[3]);
        if (i<0||i>3) 
		{
            sprintf(back,"Day num error\0");
            return;
        }
        if (!strncmp(cmd[2],WEEKNUM,CMDperLenth))
        {
            if (!(weakDate[i]->GetText()))
            {
                sprintf(back,"Can't find this item\0");
            }
            sprintf(back,"%s\0",weakDate[i]->GetText());
            
        }
        else if (!strncmp(cmd[2],WEATHER,CMDperLenth))
		{
            if (!(dayWeather[i]->GetText()))
            {
                sprintf(back,"Can't find this item\0");
            }
            sprintf(back,"%s\0",dayWeather[i]->GetText());
            
        }
        else if (!strncmp(cmd[2],WIND,CMDperLenth)) 
		{
            if (!(dayWind[i]->GetText()))
            {
                sprintf(back,"Can't find this item\0");
            }
            sprintf(back,"%s\0",dayWind[i]->GetText());
            
        }
        else if (!strncmp(cmd[2],TEMP,CMDperLenth)) 
		{
            if (!(dayTemp[i]->GetText()))
            {
                sprintf(back,"Can't find this item\0");
            }
            sprintf(back,"%s",dayTemp[i]->GetText());
            
        }
        else if (!strncmp(cmd[2],PM25,CMDperLenth)) 
		{
            
            if (i!=0 )
			{
                
                sprintf(back,"Day num error\0");
                return;
            }
            if (!(pm25->GetText()))
            {
                sprintf(back,"Can't find PM25\0");
            }
            else
            {
            //sprintf(back,"%d:%d\0",pm25,pm25->GetText());
            sprintf(back,"%s\0",pm25->GetText());
            }
            
        }
        else
		{
            
            sprintf(back,"%s %s %s\0",dayWeather[i]->GetText(),dayWind[i]->GetText(),dayTemp[i]->GetText());
        }
    }
    
    /*
    for (i=0; i<4; i++) {
        printf("%s %s %s %s\n",weakDate[i]->GetText(),dayWeather[i]->GetText(),dayWind[i]->GetText(),dayTemp[i]->GetText());
        
    }
    printf("PM25:%s\n",pm25->GetText());*/
    
}
