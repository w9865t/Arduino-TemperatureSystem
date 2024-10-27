import requests
import json
import random

def getWeather():
    city = "Nagano"
    apikey = "自分のOpenWeather APIキーを入力"
    lang = "ja"
    units = 'metric'
    api = f"https://api.openweathermap.org/data/2.5/weather?q={city}&appid={apikey}&lang={lang}&units={units}"

    data = requests.get(api)
    data = json.loads(data.text)
    
    print(data)
    return data["main"]["temp"]