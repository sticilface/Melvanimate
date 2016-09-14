var _online = navigator.onLine;

console.log( (_online)? "SITE is online" : "SITE is offline");
// Update configuration to our liking
//$( document ).on( "mobileinit", function() {


if (typeof(Storage) !== "undefined") {
  // Code for localStorage/sessionStorage.
  console.log("LOCAL STORAGE SUPPORTED");
} else {
  // Sorry! No Web Storage support..
  console.log("LOCAL STORAGE NOT SUPPORTED");
}

function RemoveTopic(topic) {
            console.log("Topic: " + topic);
            $.post( _currentdevice + "data.esp"  , "remove_topic=" + topic  , function(data) {

                  }).success(function(e) {
                      processJSON(e);
                      $("#status").empty().append("Success").css("color", "green");
                  })
                  .error(function() {

                      $("#status").empty().append("Offline").css("color", "red");

                      _connectedStatus = "offline";

                  })
                  .complete(function() {});
}

function loadpreset(preset) {

            $.post( _currentdevice + "data.esp"  , "presetcommand=load&selectedeffect=" + preset  , function(data) {

                  }).success(function(e) {
                      processJSON(e);
                      $("#status").empty().append("Success").css("color", "green");
                  })
                  .error(function() {

                      $("#status").empty().append("Offline").css("color", "red");

                      _connectedStatus = "offline";

                  })
                  .complete(function() {});

}
