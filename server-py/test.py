from WeatherInfo import WeatherInfo

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
                        print 'PM25:' + weatherInfo[i]['pm25']
                print '===================='
