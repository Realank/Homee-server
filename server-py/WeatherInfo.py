# -*- coding: cp936 -*-
import json
import string
import time
class WeatherInfo:
	def __init__(self,city = u'天津'):
		self.city = city
		url1 = u'http://api.map.baidu.com/telematics/v3/weather?location='
		url2 = u'&output=json&ak=FD4bc79c3f082216d5ccd573756fd65a'
		cityUrl = repr(city.encode('utf8'))
		cityUrl = cityUrl[1:-1].replace('\\x','%')
		self.url = url1+cityUrl+url2
		self.lastUpdateTimeSlot = 0
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
				if weatherDict['results'][0].has_key('pm25'):
					self.weatherData[0]['pm25'] = weatherDict['results'][0]["pm25"]
	def getWeather(self):
		if (self.lastUpdateTimeSlot== 0) or (time.time()-self.lastUpdateTimeSlot>5):
			print 'update the json'
			self.lastUpdateTimeSlot = time.time()
			self.__getWeatherInfo()
		return self.weatherData

if __name__ == '__main__':
	wtif = WeatherInfo()
	weatherInfo = wtif.getWeather()
	print len(weatherInfo)
	for i in range(4):
		print '===================='
		print weatherInfo[i]['date']
		print weatherInfo[i]['weather']
		print weatherInfo[i]['wind']
		print weatherInfo[i]['temperature']
		if i == 0:
			print 'PM2.5:' + weatherInfo[i]['pm25']
		print '===================='

