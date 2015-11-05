(function() {
	loadOptions();
	submitHandler();
})();

function submitHandler() {
	var $submitButton = $('#submitButton');
	$submitButton.on('click', function() {
		console.log('Submit');

		var return_to = getQueryParam('return_to', 'pebblejs://close#');
		document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
	});
}

var $submitButton = $('#submitButton');

$submitButton.on('click', function() {
	console.log('Submit');

	var return_to = getQueryParam('return_to', 'pebblejs://close#');
	document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
});

function getAndStoreConfigData() {
	var $backgroundColorPicker = $('#backgroundColorPicker');

	var options = {
		backgroundColor : $backgroundColorPicker.val(),
	};

	localStorage.willdorfcirclesquaredbackgroundColor = options.backgroundColor;

	console.log('Got Options: ' + JSON.stringify(options));
	return options;
}

function loadOptions() {
	var $backgroundColorPicker = $('#backgroundColorPicker');

	if (localStorage.backgroundColor) {
		$backgroundColorPicker[0].value = localStorage.willdorfcirclesquaredbackgroundColor;
	}
}

function getQueryParam(variable, defaultValue) {
	var query = location.search.substring(1);
	var vars = query.split('&');
	for (var i = 0; i < vars.length; i++) {
		var pair = vars[i].split('=');
		if (pair[0] === variable) {
			return decodeURIComponent(pair[1]);
		}
	}
	return defaultValue || false;
}
