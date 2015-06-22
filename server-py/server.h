//
//  service.h
//  
//
//  Created by Realank on 14/8/2.
//
//
#ifndef SERVER_H
#define SERVER_H

#define BUFFER_SIZE 100
#define socketCMDnum 10
#define CMDperLenth 4
#define UPDATE_XML_INTERVAL 1000 //sec

#define HOSTPORT 3000			// the port users will be connecting to
#define SLAVEPORT 3001			// the port users will be connecting to
#define SERVERIP   "182.92.183.168"
#define HOST_CLIENT_NUM 1			// how many pending connections queue will hold
#define SLAVE_CLIENT_NUM 100			// how many pending connections queue will hold
#define TIMEOUT_SEC 5

//command string define

#define STATUSCMDLENTH 4 //"STUS:CHEK:SEVR:ISOK"
#define STATUS "STUS"
#define ISOK "ISOK"
#define SERVERSIDE "SEVR"
#define HOSTSIDE "HOST"
#define STATUSGOOD "GOOD"
#define NOHOST "NHST"
#define PULSE "PLSE"


#define LIGHTCMDLENTH  4 //"LITE:SETE:LVRM:0002
#define LIGHT "LITE"
#define LIVINGROOM "LVRM"
#define BEDROOM01 "BDR1"
#define BEDROOM02 "BDR2"
#define BEDROOM03 "BDR3"
#define BATHROOM01 "BTH1"
#define BATHROOM02 "BTH2"
#define BALCONY01 "BLC1"
#define BALCONY02 "BLC2"
#define DINNINGROOM "DNRM"

#define TVCMDLENTH 3 //"TLVS:SETE:CHPL"
#define TV "TLVS"
/*
#define TVCHANNELPLUS "CHPL"
#define TVCHANNELMINUS "CHMI"
#define TVVOLUMEUP "VOUP"
#define TVVOLUMEDOWN "VODN"
#define TVPOWER "TVPW"
#define TVFUNCTION1 "TVF1"
#define TVFUNCTION2 "TVF2"
#define TVFUNCTION3 "TVF3"*/

#define AIRCONDITIONORCMDLENTH 3 //"ARCD:SETE:TCHI"
#define AIRCONDITIONOR "ARCD"
/*
#define TEMPERATURECOLDHIGH "TCHI"
#define TEMPERATURECOLDMID "TCMD"
#define TEMPERATURECOLDLOW "TCLW"
#define TEMPERATUREWARMHIGH "TWHI"
#define TEMPERATUREWARMMID "TWMD"
#define TEMPERATUREWARMLOW "TWLW"*/

#define DELAYSWICHCMDLENTH 3 //"FLWR:SETE:0005"
#define FLOWERWATER "FLWR"
#define PETFOOD "PTFD"


#define WEATHERCMDLENTH 4 //"WTHR:CHEK:WTHR:0000"
#define WEEKNUM "WEEK"
#define WEATHER "WTHR"
#define WIND "WIND"
#define TEMP "TEMP"
#define PM25 "PM25"

#define CHECK "CHEK"
#define SET "SETE"
#define BACK "BACK"
#define START "STRT"
#define END "ENDD"
#define ERRORSTATUS "ERRR"
#define SETERROR "STER"

#endif
