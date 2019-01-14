// store global variables somewhere else?
var menuPhase = ["Genres", "Series", "Title"];
var cur = 0;
var curMenu = 'Genres';
var lastMenu = 'Genres';

// onclick event
// if there are more results to display on selection, post to the server 
//and request the data, then append it to the page
//else we are only displaying titles, then play the selected choice
function getFrom(elemt, queryTerm){
	if(cur < 2){
		cur++;
		$('#back').show();
		$.post('/ajax', {
	        query: queryTerm,
	        menu: menuPhase[cur]
	    }).done(function(data) {
	    	lastMenu = curMenu;
	    	curMenu = elemt.innerHTML.replace("'", "");
	    	$('#menu').text(curMenu);
	    	$('#back').text('« Back');// + lastMenu);
	    	appendLi(data);
	    });  
	}
	else{
		adjPlaylist(elemt.value, elemt.innerHTML, curMenu);
	}
}

// play next show and adjust playlist display accordingly
function playNext(){
	$('#nowPlayingList li:first-child').remove();
	if(($('#upNextList li').length) != 0){
		var nextProgram =  $('#upNextList li').first();
		$('#upNextList li:first-child').remove();
		$('#nowPlayingList').append(nextProgram);
		initAudio(nextProgram.children().val(), "Test", "Test");
	}
}

// init audio to play current mp3
function initAudio(url, text, series){
	var player = $('#player')[0];
	$("#src").attr("src", url);
	player.addEventListener('ended', playNext);
	player.load();
	player.play();
	$("#nowPlaying").text("Now Playing: " + text);
	$("#nowPlaying2").text(series);
}

// adjust playlist to add selected program
function adjPlaylist(url, text, series){
	if(($('#nowPlayingList li').length) == 0){
		$('#nowPlayingList').append("<li class='link' value=" + url +">" + text + "</li>");
		initAudio(url, text, series);
	}else{
		//var clearButton = "<button class ='link'>Clear</button>"
		$('#upNextList').append("<li>" + 
								//clearButton + 
								"<button class='link' value=" + url + ">" + text +  "</button>" +
								"</li>");
	}
}

// can remove items now yay
function clearItem(id){
	$("#" + id).remove();
}

// function to append to the page the choices in ul
function appendLi(data){
	$('#list').empty();

	if(menuPhase[cur] != "Genres" ){//|| menuPhase != "Series"){
		var backButton = $('<li><button class="link" onclick="back()">« Back</button></li>');
		$('#list').append(backButton);
	}

	for (i in data.results) {
	    //var li = $('<li><button onclick="getFrom(this, ' + "'" + data.results[i][0] + "'" + ')">' + data.results[i][0] + '</button></li>');
	    var li = $('<li>' + 
	    			'<div class="link">' + 
	    			'<button onclick="getFrom(this, ' + "'" + addSlashes(data.results[i][0]) + "'" + ')">' + 
	    			"'" + data.results[i][0] + "'" +
	    			'</button>' + 
	    			'</div>' +
	    			'</li>');
    	if(cur == 2){
    		li.children().children().attr('value', data.results[i][1]);
    		li.children().children().before("<div class='year'> First aired on the date " + data.results[i][2] +"</div>")
    	}

    	$('#list').append(li);
    	$('#list').append(li);
    }
}

// go back to previous page, post to server tp get results
function back(){
	if(cur > 0){
		cur--;
		if(cur == 0){
			$('#back').hide();
		}
		// consisder turning ajax post into seperate function
		$.post('/ajax', {
	        query: lastMenu,
	        menu: menuPhase[cur]
	    }).done(function(data) {
	    	curMenu = lastMenu;
	    	lastMenu = 'Genres';
	    	$('#menu').text(curMenu);
	    	$('#back').text('« Back');
	    	appendLi(data);
	    });
	}
}

// change button images for audio controls
function control(){
	var player = $('#player')[0];
	if(player.readyState == 4){
		if(player.paused == false){
			player.pause();
			$('#play').attr('src', 'static/play3.png');
		}else{
			player.play();
			$('#play').attr('src', 'static/pause3.png');
		}
	}
}

// control audio and change images based on selection
function controlA(){
	var player = $('#player')[0];
	if(player.readyState == 4){
		if(player.volume == 1){
			player.volume = 0;
			$('#sound').attr('src', 'static/mute1.png');
		}else if(player.volume == 0){
			player.volume = 0.2;
			$('#sound').attr('src', 'static/volume22.png');
		}else if(player.volume == 0.2){
			player.volume = 0.6;
			$('#sound').attr('src', 'static/volume11.png');
		}
		else if(player.volume == 0.6){
			player.volume = 1;
			$('#sound').attr('src', 'static/sound1.png');
		}
	}
}

// helper function for readable apostrophes
function addSlashes(str) {
    return (str + '').replace(/[\\"']/g, '\\$&').replace(/\u0000/g, '\\0');
}

// get current weather via api
// TODO: note, as of now only returns weather for edmonton, possibly add dynamic location
function getWeather(){
	$.get("http://api.openweathermap.org/data/2.5/weather?" +
		"q=edmonton&units=metric&appid=72b47c04853a69a287da385d631e6932", 
		function(data){
			$(".weather").text(data.weather[0].description + ";" +
				" Temp: " + data.main.temp + "°C;" +
				" Wind: " + data.wind.speed + "km/h;" +
				" Humidity: " + data.main.humidity + "%;");
		}, "json");
}

function randomize(){
	$.post('/getRandom', {
	    }).done(function(data) {
	    	for(i in data.results){
	    		adjPlaylist(data.results[i][4], data.results[i][0], data.results[i][1]);
	    	}
	    });  
}