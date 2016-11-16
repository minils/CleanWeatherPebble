var apikey = '228825d4e332b6be51039f5f3397dddd';

var xhrRequest = function(url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function() {
	callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
}

function locationSuccess(pos)
{
    var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
	pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + apikey + '&units=metric';
    
    xhrRequest(url, 'GET', function(responseText) {
	var json = JSON.parse(responseText);

	var temperature = Math.round(json.main.temp);
	console.log('Temperature is ' + temperature);
	var icon = json.weather[0].icon;
	console.log('Icon is ' + icon);

	var dictionary = {
	    'TEMPERATURE': temperature,
	    'ICON': icon
	};

	Pebble.sendAppMessage(dictionary, function(e) {
	    console.log('Weather info sent to Pebble successfully');
	}, function(e) {
	    console.log('Error sending weather info to Pebble');
	});
    });
	
}

function locationError(err)
{
    console.log('Error requesting the location!');
}

function getWeather()
{
    navigator.geolocation.getCurrentPosition(
	locationSuccess,
	locationError,
	{timeout: 15000, maximumAge: 60000}
    );
}

Pebble.addEventListener('ready', function(e) {
    console.log('PebbleKit JS ready!');
    getWeather();
});

Pebble.addEventListener('appmessage', function(e) {
    console.log('AppMessage received');
    getWeather();
});
