var keys = require('message_keys');
var cachedUserToken = undefined;

const BASE_LAUNCH_URL = "https://ll.thespacedevs.com/2.2.0/launch/"

function formatLaunchQuery(limit, offset) {
    if(limit == undefined) limit = 10;
    if(offset == undefined) limit = 0;

    var nowDate = new Date(Date.now());
    var settledDate = new Date(nowDate.getFullYear(), nowDate.getMonth(), nowDate.getDate());
    return BASE_LAUNCH_URL + "?limit=" + limit + "&offset=" + offset + "&net__gte=" + settledDate.toISOString();
}

function formatDisplayDate(date) {
    const month = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"];
    const hours = date.getHours();
    const hours_12h = (hours > 12) ? hours - 12 : ((hours == 0) ? 12 : hours);
    const suffix_12h = (hours >= 12) ? "PM" : "AM";
    const hours_string = (hours < 10) ? "0" + hours_12h : String(hours_12h);
    const minutes = date.getMinutes();
    const minutes_string = (minutes < 10) ? "0" + minutes : String(minutes);

    const formatted_result = month[date.getMonth()] + " " + date.getDate() + " @ " + hours_string + ":" + minutes_string + " " + suffix_12h;
    return formatted_result;
}

function handleAllLaunchData(launchResponses) {
    // We want to get:
    // - ID
    // - Name
    // - Net
    var payloadDict = {};
    for (var i = 0; i < launchResponses.length; i++) {
        var launch = launchResponses[i];
        payloadDict[keys.s_return_launches_ids + i] = launch["id"];
        payloadDict[keys.s_return_launches_names + i] = launch["name"].substring(0, 36); // Trim for data purposes
        payloadDict[keys.s_return_launches_times + i] = formatDisplayDate(new Date(launch["net"]));
    }

    // Send it to the pebble
    Pebble.sendAppMessage(payloadDict, function () {
        console.log('Message sent to pebble successfully');
    }, function (e) { 
        console.log('Message failed to send: ' + JSON.stringify(e));
    });
}

var resultsCache = undefined;
function getAllLaunchData(offset) {
    
    // Cache our results, in case we need to ask again this runtime
    if(resultsCache) {
        return handleAllLaunchData(resultsCache);
    }

    const method = 'GET';
    const url = formatLaunchQuery(10, offset);
    const request = new XMLHttpRequest;


    request.onreadystatechange = function () {
        if(this.readyState == 4 && this.status == 200) {
            try {
                var response = JSON.parse(this.responseText);
                console.log("Got a launches response with " + response["results"].length + " entries");
                resultsCache = response["results"];
                handleAllLaunchData(response["results"]);
            } catch(e) {
                console.log("Error parsing launch data response: " + e.name + " > " + e.message);
            }
        } else if (this.readyState == 4 && this.status == 429) {
            // The API we use is free for 15 requests an hour. Nice enough for regular
            // use, but only if we ask it once a runtime. If we run out of calls, we
            // get an error message with an estimated return time. We can
            // steal that and display that on the watch for the user.
            var response = JSON.parse(this.responseText);

            // The message is formatted "Request was throttled. Expected available in <time> seconds."
            // Getting the second to last word gets us the time.
            const splitDetails = response["detail"].split(" ");
            splitDetails.pop();
            const delayTime = parseInt(splitDetails.pop());

            // Log it as an error
            console.warn("API has been saturated. Current timeout of " + delayTime);

            // Send it to the watch
            const payloadDict = {}
            payloadDict[keys.i_return_launch_error] = delayTime;
            Pebble.sendAppMessage(payloadDict, function () {}, function () {});
        }
    }

    request.open(method, url);
    request.send();
}

function getLoneLaunchData(index) {
    /*
    // get data from our results cache
    if(resultsCache == undefined) return;
    const launch = resultsCache[index];

    // Prepare data to be sent to the watch
    const payloadDict = {};
    const parts = [];
   payloadDict[keys.s_return_launch_bundle] = launch["name"] + "\n" + launch["status"]["name"];
    // Send it to the pebble
    Pebble.sendAppMessage(payloadDict, function () {
        console.log('Message sent to pebble successfully');
    }, function (e) { 
        console.log('Message failed to send: ' + JSON.stringify(e));
    });*/
}

function pushPin(index) {
    // Get the data from the cache, if it is full
    if(!resultsCache) return;
    const ourData = resultsCache[index];

    // Build the pin
    const pin = {
        "id": ourData["id"],
        "time": ourData["net"],
        "createNotification": {
        "layout": {
            "type": "genericNotification",
            "title": "New Item",
            "tinyIcon": "system://images/NOTIFICATION_FLAG",
            "body": "A new appointment has been added to your calendar at 4pm."
            }
        },
        "layout": {
            "type": "genericPin",
            "title": ourData["name"],
            "tinyIcon": "system://images/NOTIFICATION_FLAG"
        }
    };

    // Send the request
    const method = "PUT";
    const url = "https://timeline-sync.rebble.io/v1/user/pins/" + pin["id"];
    const request = XMLHttpRequest();

    request.setRequestHeader('Content-Type', 'application/json');
    request.setRequestHeader('X-User-Token', cachedUserToken);
    

    request.onreadystatechange = function () {
        if(this.readyState == 4 && this.status == 200) {
            try {
                var response = JSON.parse(this.responseText);
                console.log(response);
            } catch(e) {
                console.log("Error parsing launch data response: " + e.name + " > " + e.message);
            }
        }
    }

    request.open(method, url);
    request.send(pin);
}

// Function from Kat, after we found a weird bug where message keys were inconsistent
function fixMissingNumbers(dict) {
    for (var k in keys) {
        if (Object.prototype.hasOwnProperty.call(keys, k)) {
            if (Object.prototype.hasOwnProperty.call(dict, k)) {
                dict[keys[k]] = dict[k]
            }
        }
    }
}

Pebble.addEventListener("ready", function(e) {
    // We're ready to roll!
    console.log("PebbleKit JS ready!");

    // Let the app know we're ready to process messages
    var payload = {};
    payload[keys._ready] = 1;

    Pebble.sendAppMessage(payload, function () { });

    /* -- Fix once I figure out how to use RWS on emulators -- //
    Pebble.getTimelineToken(function(token) {
        cachedUserToken = token;
    }, function(error) {
        // Error getting token
        console.error("Error getting token");
        console.error(error);
    })*/
});

Pebble.addEventListener('appmessage', function (e) {
    // Get the dictionary from the message
    var data = e.payload;
    fixMissingNumbers(data);

    if (data[keys.i_request_all_launches] !== undefined) {
        // It is! Perform a request
        console.log("Got a request for all launches");
        getAllLaunchData(data[keys.i_request_all_launches]);
    }

    if (data[keys.i_request_specific_launch_data] !== undefined) {
        // We want to get specific launch data
        console.log("Got a request for one lunch");
        getLoneLaunchData(data[keys.i_request_specific_launch_data]);
    }

    if (data[keys.i_push_pin] !== undefined) {
        // If we have a push pin request, grab our old data,
        // create a pin, and push it to our user's timeline
        pushPin(data[keys.i_push_pin]);
    }
})