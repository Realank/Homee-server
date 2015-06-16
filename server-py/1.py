# -*- coding: cp936 -*-
import json
import string
def getWebContentFromUrl(url):
	import urllib2
	enable_proxy = True
	proxy_handler = urllib2.ProxyHandler({"http" : 'http://109.130.2.36:8080'})
	null_proxy_handler = urllib2.ProxyHandler({})
	if enable_proxy:
		opener = urllib2.build_opener(proxy_handler)
	else:
		opener = urllib2.build_opener(null_proxy_handler)
	urllib2.install_opener(opener)
	return urllib2.urlopen(url).read()

def urlOfCity(city):   
	url1 = u'http://api.map.baidu.com/telematics/v3/weather?location='
	url2 = u'&output=json&ak=FD4bc79c3f082216d5ccd573756fd65a'
	cityUrl = repr(city.encode('utf8'))
	cityUrl = cityUrl[1:-1].replace('\\x','%')
	return url1+cityUrl+url2

def getWeatherInfo(day):
	city = u'天津'
	weatherJson = getWebContentFromUrl(urlOfCity(city))
	weatherDict = json.loads(weatherJson)
	if weatherDict.has_key('results'):
		if len(weatherDict['results']) > 0:
			if weatherDict['results'][0].has_key('weather_data'):
				weatherData = weatherDict['results'][0]["weather_data"]
				if day < len(weatherData):
					return (weatherData[day]["date"],weatherData[day]["weather"],weatherData[day]["wind"],weatherData[day]["temperature"])

class WeatherInfo:
        def __init__(self,city = u'天津'):
                self.city = city
                url1 = u'http://api.map.baidu.com/telematics/v3/weather?location='
                url2 = u'&output=json&ak=FD4bc79c3f082216d5ccd573756fd65a'
                cityUrl = repr(city.encode('utf8'))
                cityUrl = cityUrl[1:-1].replace('\\x','%')
                self.url = url1+cityUrl+url2
                self.__getWeatherInfo()
        def __getWebContentFromUrl(self,url):
                import urllib2
                enable_proxy = True
                proxy_handler = urllib2.ProxyHandler({"http" : 'http://109.130.2.36:8080'})
                null_proxy_handler = urllib2.ProxyHandler({})
                if enable_proxy:
                        opener = urllib2.build_opener(proxy_handler)
                else:
                        opener = urllib2.build_opener(null_proxy_handler)
                urllib2.install_opener(opener)
                return urllib2.urlopen(url).read()
        def __getWeatherInfo(self):
                city = u'天津'
                weatherJson = self.__getWebContentFromUrl(self.url)
                weatherDict = json.loads(weatherJson)
                if weatherDict.has_key('results'):
                        if len(weatherDict['results']) > 0:
                                if weatherDict['results'][0].has_key('weather_data'):
                                        self.weatherData = weatherDict['results'][0]["weather_data"]
        def getWeather(self):
                return self.weatherData
        
                
        
wtif = WeatherInfo()
weatherInfo = wtif.getWeather()
print len(weatherInfo)
for i in range(4):
        print '===================='
        print weatherInfo[i]['date']
        print weatherInfo[i]['weather']
        print weatherInfo[i]['wind']
        print weatherInfo[i]['temperature']
        print '===================='
