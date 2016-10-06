function loadcss(url, callback) {
 var link = document.createElement("link");
 link.setAttribute("rel", "stylesheet");
 link.setAttribute("href", url);
 link.addEventListener('load', function () {
  var script = document.createElement("script");
  script.textContent = "(" + callback.toString() + ")();";
  document.body.appendChild(script);
 }, false);
 document.body.appendChild(link);
}

function loadjs(url, callback) {
 var script = document.createElement("script");
 script.setAttribute("src", url);
 script.addEventListener('load', function () {
  var script = document.createElement("script");
  script.textContent = "(" + callback.toString() + ")();";
  document.body.appendChild(script);
 }, false);
 document.body.appendChild(script);
}

// loadcss("/jquery/jqm1.4.5.css", function() {
//     loadjs("/jquery/jq1.11.1.js", function() {
//         //window.jQ=jQuery.noConflict(true);
//         loadjs("/jquery/jqm1.4.5.js", function() {
//             //window.jQ=jQuery.noConflict(true);
//             loadjs("jqColorPicker.min.js", function() {

$(function () {

 $("[data-role='header']").toolbar();
 $("[data-role='footer']").toolbar();
 $("body>[data-role='panel']").panel();

});
RUNDOCUMENT();
GetData("homepage");
$("body").css("visibility", "visible");

//             });
//         });
//     });
// });

function getBaseUrl() {
 var re = new RegExp(/^.*\//);
 return re.exec(window.location.href);
}

var _currentdevice = getBaseUrl();
var _currentdeviceName = "";
var _originalname = "";
console.log(_currentdevice);
var _connectedStatus = "";

var _offlineDeviceList = {};

if (localStorage.devicelist) {
 console.log("deviceList @ init = ");
 console.log(localStorage["devicelist"]);
 try {
  _offlineDeviceList = JSON.parse(localStorage["devicelist"]);
 } catch (err) {
  console.log("PARSE ERROR");
 }
} else {
 console.log("deviceList is not defined ");
}

var _deviceName = "";

var currentmode;
var globaldata = {};
var settings = {};
var presetspagedata = {};

var plot1;
var graphvalues = [''];
var disable_update = false;
var isconnected = false;
var graphdata;
var xaxisvar = 60;
var booldynamic;
var graphtimeframe = 60;

var NEO_MATRIX_TOP = 0x00 // Pixel 0 is at top of matrix
var NEO_MATRIX_BOTTOM = 0x01 // Pixel 0 is at bottom of matrix
var NEO_MATRIX_LEFT = 0x00 // Pixel 0 is at left of matrix
var NEO_MATRIX_RIGHT = 0x02 // Pixel 0 is at right of matrix

var NEO_MATRIX_CORNER = 0x03 // Bitmask for pixel 0 matrix corner
var NEO_MATRIX_ROWS = 0x00 // Matrix is row major (horizontal)
var NEO_MATRIX_COLUMNS = 0x04 // Matrix is column major (vertical)
var NEO_MATRIX_AXIS = 0x04 // Bitmask for row/column layout
var NEO_MATRIX_PROGRESSIVE = 0x00 // Same pixel order across each line
var NEO_MATRIX_ZIGZAG = 0x08 // Pixel order reverses between lines
var NEO_MATRIX_SEQUENCE = 0x08 // Bitmask for pixel line order

// These apply only to tiled displays (multiple matrices):

var NEO_TILE_TOP = 0x00 // First tile is at top of matrix
var NEO_TILE_BOTTOM = 0x10 // First tile is at bottom of matrix
var NEO_TILE_LEFT = 0x00 // First tile is at left of matrix
var NEO_TILE_RIGHT = 0x20 // First tile is at right of matrix
var NEO_TILE_CORNER = 0x30 // Bitmask for first tile corner
var NEO_TILE_ROWS = 0x00 // Tiles ordered in rows
var NEO_TILE_COLUMNS = 0x40 // Tiles ordered in columns
var NEO_TILE_AXIS = 0x40 // Bitmask for tile H/V orientation
var NEO_TILE_PROGRESSIVE = 0x00 // Same tile order across each line
var NEO_TILE_ZIGZAG = 0x80 // Tile order reverses between lines
var NEO_TILE_SEQUENCE = 0x80 // Bitmask for tile line order

// $(document).on('offline online', function (event) {
//     alert('You are ' + event.type + '!');
//     _connectedStatus = event.type;
// });

// $(document).ready(function() {
//   $(window).bind("offline online", function(event) { alert('You are ' + event.type + '!') ; });
// });

/****************************************************
 *                    Appcache
 *
 ****************************************************/

function handleAppCache() {
 if (applicationCache === undefined) {
  return;
 }

 if (applicationCache.status == applicationCache.UPDATEREADY) {
  popUpMessage("Refreshing AppCache");
  applicationCache.swapCache();
  location.reload();
  return;
 }

 applicationCache.addEventListener('updateready', handleAppCache, false);
}

/****************************************************
 *                    Events
 *
 ****************************************************/
var _waiting_reboot = false;
var _upgrade_failed;

var source;

var _messageCount = 0;

function ge(s) {
 return document.getElementById(s);
}

function ce(s) {
 return document.createElement(s);
}

function addMessage(m) {
 if (_messageCount > 14) {
  //$("#wsmessages").find('div').first().remove();
  _messageCount = _messageCount - 1;
 }
 _messageCount = _messageCount + 1;
 var msg = ce("div");
 msg.innerText = m;
 //$("#wsmessages").append(msg);

}

function popUpMessage(m) {
 addMessage(m);
 var msg = ce("div");
 msg.innerText = m;
 $("#wsalert").append(msg);
 $("#wsalert").popup("open");

 setTimeout(function () {
  $("#wsalert").popup("close");

 }, 6000);
}

function startEvents() {
 if (!!window.EventSource) {
  source = new EventSource('espman/events');

  // source.addEventListener('open', function(e) {
  //   console.log("Events Opened", e);
  //
  //   if (_waiting_reboot) {
  //     $("#upgrade_message").empty().append( "<h4>Done</h4>");
  //     clearTimeout(_upgrade_failed);
  //     _waiting_reboot = false;
  //     setTimeout(function() {
  //       $("#upgradepopup").popup("close");
  //     }, 2000);
  //
  //   }
  //
  // }, false);

  source.onopen = function (e) {
   console.log("Events Opened", e);
   addMessage("Events Opened");
   if (_waiting_reboot) {
    $("#upgrade_message").empty().append("<h4>Done</h4>");
    clearTimeout(_upgrade_failed);
    _waiting_reboot = false;
    setTimeout(function () {
     $("#upgradepopup").popup("close");
    }, 2000);

   }
  };

  source.onerror = function (e) {
   console.log("Events ERROR", e);
   if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Closed");
    addMessage("Events Closed");
   }
  };

  source.addEventListener('message', function (e) {
   console.log("message", e.data);
   popUpMessage(e.data);
  }, false);

  source.addEventListener('console', function (e) {
   //console.log("message", e.data);
   addMessage(e.data);
  }, false);

  // source.addEventListener('myevent', function(e) {
  //   console.log("myevent", e.data);
  // }, false);
  //  upgrade is via JSON package
  source.addEventListener('upgrade', function (e) {
   //console.log("upgrade event", e.data);
   if (e.data == "begin") {
    $("#upgrade_message").empty();
    $("#updatebanner").empty().append("Upgrade Started");
    $("#upgrade-slider").val(0);
    $("#upgradepopup").popup("open");
    addMessage("Upgrade Started");
   } else if (e.data == "end") {
    addMessage("Upgrade Finished");
    setTimeout(function () {
     $("#upgradepopup").popup("close");
    }, 1000);
   } else if ($.isNumeric(e.data) && e.data >= 0 && e.data <= 100) {
    //console.log(e.data);
    $("#upgrade-slider").val(e.data);
    $("#upgradepopup").popup("open");
    $("#upgradepopup").enhanceWithin().popup();
   } else if (e.data == "firmware") {
    $("#upgrade_message").empty().append("<h4>Updating Firmware...</h4>");
    _upgrade_failed = setTimeout(function () {
     $("#upgrade_message").empty().append("<h4>REBOOT failed: Device Not online</h4>");
     setTimeout(function () {
      $("#upgradepopup").popup("close");
      location.reload();
     }, 1000);
    }, 90000);
   } else if (e.data == "firmware-end") {
    $("#upgrade_message").empty().append("<h4>Rebooting...</h4>");
    _waiting_reboot = true;
    clearTimeout(_upgrade_failed);
    _upgrade_failed = setTimeout(function () {
     $("#upgrade_message").empty().append("<h4>REBOOT failed: Device Not online</h4>");
     setTimeout(function () {
      $("#upgradepopup").popup("close");
      location.reload();
     }, 1000);
    }, 60000);

   } else {
    $("#upgrade_message").empty().append("<h4>" + e.data + "</h4>");
    $("#upgradepopup").enhanceWithin().popup();
    addMessage(e.data);
   }
  });

  // OTA update
  source.addEventListener('update', function (e) {

   if (e.data == "begin") {
    $("#upgrade_message").empty();
    $("#updatebanner").empty().append("OTA Update Started");
    $("#upgrade-slider").val(0);
    $("#upgradepopup").popup("open");
    addMessage("OTA upgrade Started");
   } else if (e.data == "end") {
    addMessage("OTA Finished");
    $("#upgrade_message").empty().append("<h4>Rebooting...</h4>");
    _waiting_reboot = true;
    _upgrade_failed = setTimeout(function () {
     $("#upgrade_message").empty().append("<h4>REBOOT failed: Device Not online</h4>");
     setTimeout(function () {
      $("#upgradepopup").popup("close");
     }, 2000);
    }, 60000);
   } else if ($.isNumeric(e.data) && e.data >= 0 && e.data <= 100) {
    $("#upgrade-slider").val(e.data);
    $("#upgradepopup").enhanceWithin().popup();
   } else {
    $("#upgrade_message").empty().append("<h4>" + e.data + "</h4>");
    $("#upgradepopup").enhanceWithin().popup();
    addMessage(e.data);
   }

  }, false);

 }
}

//    $('.noEnterSubmit').bind('keypress', false);
function RUNDOCUMENT() {

 $('.color').colorPicker({
  preventFocus: true,
  renderCallback: function ($elm, toggled) {
   if (toggled === true) { // simple, lightweight check
    // ... like an open callback
   } else if (toggled === false) {

    // ... like a close callback
    //var nameof = $(this).attr("name");
    //var $form = $(this).closest('input');

    //console.log($form.attr('name'));
    var colors = this.color.colors,
     rgb = colors.RND.rgb;
    //console.log(rgb.r + ', ' + rgb.g + ', ' + rgb.b + ', ' + colors.alpha);

    var $form = $elm.closest('form');
    var inputName = $elm.attr('name');
    //var inputValue = $elm.attr('value');

    // var data = {
    //     color: {
    //         name: inputName,
    //         R: rgb.r,
    //         G: rgb.g,
    //         B: rgb.b,
    //     }
    // };
    // console.log(JSON.stringify(data));

    $("#status").empty().append("Waiting ").css("color", "blue");;

    $.post(_currentdevice + "data.esp", inputName + "=" + rgb.r + "," + rgb.g + "," + rgb.b
      // function(data) {
      //     console.log(data);
      // }
     ).success(function (e) {
      if (e) {
       processJSON(e);
      }
      $("#status").empty().append("Success").css("color", "green");;
     })
     .error(function () {
      $("#status").empty().append("Error").css("color", "red");;
     })
     .complete(function () {});

    $elm.css('color', "rgb(" + colors.RND.rgb.r + "," + colors.RND.rgb.g + "," + colors.RND.rgb.b + ")");
    $elm.css('text-shadow', 'none');
   }
  },
  buildCallback: function ($elm) {
   var colorInstance = this.color,
    colorPicker = this,
    random = function (n) {
     return Math.round(Math.random() * (n || 255));
    };
   $elm.append('<div class="cp-memory">' +
    '<div></div><div></div><div></div><div></div>' +
    '<div></div><div></div><div></div><div class="cp-store">S</div>').
   on('click', '.cp-memory div', function (e) {
    console.log("Colour picker click");
    var $this = $(this);

    if (this.className) {
     $this.parent().prepend($this.prev()).children().eq(0).
     css('background-color', '#' + colorInstance.colors.HEX);
    } else {
     colorInstance.setColor($this.css('background-color'));
     colorPicker.render();
    }
   }).find('.cp-memory div').each(function () {
    !this.className && $(this).css({
     background: 'rgb(' + random() + ', ' + random() + ', ' + random() + ')'
    });
   });
  },
  opacity: false,
  cssAddon: // could also be in a css file instead
   '.cp-memory {margin-bottom:6px; clear:both;}' +
   '.cp-xy-slider:active {cursor:none;}' +
   '.cp-memory div {float:left; width:17px; height:17px; margin-right:2px;' +
   'background:rgba(0,0,0,1); text-align:center; line-height:17px;}' +
   '.cp-memory .cp-store {width:21px; margin:0; background:none; font-weight:bold;' +
   'box-sizing:border-box; border: 1px solid; border-color: #666 #222 #222 #666;}' +
   '.cp-color-picker{z-index:16777271}'
 });

 $('#effectoptions').children('div').each(function (index) {
  $(this).hide();
 });

 $('.noEnterSubmit').keypress(function (e) {
  //if ( e.which == 13 ) return false;
  //or...
  if (e.which == 13) e.preventDefault();
 });

 $(document).on("pagechange", function (event) {
  GetData($.mobile.activePage.attr('id'));
 });

 $(document).ready(function () {
  if ($(window).width() > 400) {
   console.log("Width > 400");
   $("#footer").empty().append("<h4 style=\"text-align:right\">[Power: <var id=\"power\">0</var> mA] [Heap: <var id=\"heap\">-</var> ] [Status: <var id=\"status\"></var> ]</h>");
  } else {
   console.log("Width < 400");
   $("#footer").empty().append("<h4 style=\"text-align:right\">[<var id=\"power\">0</var> mA] [<var id=\"heap\"></var> k] [<var id=\"status\"></var>]</h>");
  }
  $("[data-role='footer']").toolbar("refresh");

 });

 $(document).on("pagecreate", "#layout", function () {

  $('#select-matrix-type').change(function () {
   sortmatrix($(this));
  });

  $('#firstpixelidentify').click(function () {
   var data = $.param({
    flashfirst: "yes"
   });
   $.post(_currentdevice + "data.esp", data, function () {});
  });

  $('#revealorder').click(function () {
   var data = $.param({
    revealorder: "yes"
   });
   $.post(_currentdevice + "data.esp", data, function () {});
  })

 });

 $("#footer").on("toolbarcreate", function (event, ui) {

 });

 // $(document).on("pagebeforeshow", "#layout", function() {

 // //   GetData();
 //     console.log("layoutpage data:" + globaldata)

 //     if (globaldata) {

 //         if (globaldata.hasOwnProperty('pixels')) {
 //             $("#nopixels").val(globaldata.pixels);
 //         }
 //         if (globaldata.hasOwnProperty('grid_x')) {
 //             $("#gridx").val(globaldata.grid_x);
 //         }
 //         if (globaldata.hasOwnProperty('grid_y')) {
 //             $("#gridy").val(globaldata.grid_y);
 //         }
 //         if (globaldata.hasOwnProperty('matrixmode')) {
 //             $("#select-matrix-type").val(globaldata.matrixmode).selectmenu("refresh");
 //         }

 //         if (globaldata.hasOwnProperty('firstpixel')) {
 //             $("#select-singlematrix").val(globaldata.firstpixel).selectmenu("refresh");
 //         }
 //         if (globaldata.hasOwnProperty('axis')) {
 //             $("#select-singlematrix-axis").val(globaldata.axis).selectmenu("refresh");
 //         }
 //         if (globaldata.hasOwnProperty('sequence')) {
 //             $("#select-singlematrix-seq").val(globaldata.sequence).selectmenu("refresh");
 //         }

 //         if (globaldata.hasOwnProperty('multimatrixtile')) {
 //             $("#select-multimatrix").val(globaldata.multimatrixtile).selectmenu("refresh");
 //         }
 //         if (globaldata.hasOwnProperty('multimatrixaxis')) {
 //             $("#select-multimatrix-axis").val(globaldata.multimatrixaxis).selectmenu("refresh");
 //         }
 //         if (globaldata.hasOwnProperty('multimatrixseq')) {
 //             $("#select-multimatrix-seq").val(globaldata.multimatrixseq).selectmenu("refresh");
 //         }
 //     }

 //     sortmatrix('#select-matrix-type');

 // });

 $(document).on("pagecreate", "#homepage", function () {

  // if (localStorage.founddevices > 0) {

  // } else {

  // }
  $.mobile.resetActivePageHeight();

  $('#modeslist').change(function () {

   //  gives effect parameters time to load.
   setTimeout(function () {
    $.post(_currentdevice + "data.esp", "data=homepage", function () {

    }).success(function (e) {
     processJSON(e);
    });

   }, 100);

  });

 });

 //$(document).on("pagecreate", "#eqpage", function() {
 $("#eqpage").on("pagebeforeshow", function () {
  // $(this).children(':input[value=""]').attr("disabled", "disabled");

  $("#flip-enable-EQ").on("change", function () {

   $('#eqvars').hide();
   $('#beatsipinputs').hide();

   if ($(this).val() == 1) {
    $('#eqvars').show();

   } else if ($(this).val() == 2) {

    $('#beatsipinputs').show();
   }

  });
 });

 //
 $(document).on('touchstart click', '.myheader', function (e) {
  e.preventDefault();
  GetData($.mobile.activePage.attr('id'));
  //console.log("#myheader click");
 });

 // $(document).on("mobileinit", function() {
 //     GetData("all");
 // });

 $(document).on('click', '#device_list_button', function () {

  $("#status").empty().append("Waiting ").css("color", "blue");

  if (_offlineDeviceList) {
   console.log("Using Offine List");

   populatedevicelist(_offlineDeviceList);

  }

  $.post(_currentdevice + "data.esp", "data=devicelist", function (data) {

   }).success(function (e) {

    if (e.hasOwnProperty('devices')) {

     populatedevicelist(e.devices);

     //localStorage.setItem("founddevices", );
     localStorage["devicelist"] = JSON.stringify(e.devices);
     console.log("Stored found devices: ");
     _offlineDeviceList = JSON.parse(localStorage["devicelist"]);

     console.log(_offlineDeviceList);
     //gameState = JSON.parse(localStorage["currentGame"])

    }

    $("#status").empty().append("Success").css("color", "green");
   })
   .error(function () {

    $("#status").empty().append("Offline").css("color", "red");

    _connectedStatus = "offline";

   })
   .complete(function () {});

 });

 $(document).on('change', '.deviceselectclass', function (e) {

  // if ($(this).val() != 0 ) {
  _currentdevice = "http://" + $(this).val() + "/";
  _currentdeviceName = $(this).parent().text();
  $("#ESPmanagerlink").attr("href", "http://" + $(this).val() + "/espman/");
  // } else {
  //  _currentdevice = getBaseUrl();
  //  _currentdeviceName = _originalname;
  //}

  GetData($.mobile.activePage.attr('id'));

 });

 $(document).on('click', '.mysubmit', function () {
  $("#status").empty().append("Waiting ").css("color", "blue");
  $.post(_currentdevice + "data.esp", $(this).closest("form").find('input,select').filter(':visible').serialize(), function (data) {
    //console.log("Data Sent");
   }).success(function (e) {
    processJSON(e);
    $("#status").empty().append("Success").css("color", "green");;
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {});
 });

 $(document).on('change', '.mysubmitthis', function (e) {
  $("#status").empty().append("Waiting ").css("color", "blue");;
  $.post(_currentdevice + "data.esp", $(this).serialize(), function (data) {
    //console.log("Data Sent");
   }).success(function (e) {
    processJSON(e);
    $("#status").empty().append("Success").css("color", "green");;
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {
    //GetData();
   });
 });

 $(document).on('click', '#button_add_topic', function (e) {
  $.post(_currentdevice + "data.esp", "add_topic=" + encodeURIComponent($("#input_add_topic").val()), function (data) {}).success(function (e) {
    $("#status").empty().append("Success").css("color", "green");;
    $("#input_add_topic").val("");
    processJSON(e);
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {});
 });

 $(document).on('change', '.mysubmittext', function (e) {
  $("#status").empty().append("Waiting ").css("color", "blue");;
  $.post(_currentdevice + "data.esp", $(this).find('input'), function (data) {
    //console.log("Data Sent");
   }).success(function (e) {
    $("#status").empty().append("Success").css("color", "green");;
    processJSON(e);
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {});
 });

 $(document).on('change', '.mysubmitdropdown', function (e) {
  $("#status").empty().append("Waiting ").css("color", "blue");;
  $.post(_currentdevice + "data.esp", $(this).serialize(), function (data) {
    //console.log("Data Sent");
   }).success(function (e) {
    processJSON(e);
    $("#status").empty().append("Success").css("color", "green");;
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {});
 });

 $(document).on('slidestop', '.mysubmitslider', function (e) {
  $("#status").empty().append("Waiting ").css("color", "blue");;
  $.post(_currentdevice + "data.esp", $(this).serialize(), function (data) {
    //console.log("Data Sent");
   }).success(function (e) {
    processJSON(e);
    $("#status").empty().append("Success").css("color", "green");;
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {});
 });

 $(document).on('swipeleft', '.ui-page', function (event) {
  if (event.handled !== true) // This will prevent event triggering more then once
  {
   var nextpage = $.mobile.activePage.next('[data-role="page"]');
   // swipe using id of next page if exists
   if (nextpage.length > 0) {
    $(":mobile-pagecontainer").pagecontainer("change", "#allpresetspage", {
     transition: "slide"
    });
    //$.mobile.changePage(nextpage, {transition: "slide", reverse: false}, true, true);
   }
   event.handled = true;
  }
  return false;
 });

 // better way to get data for new page...
 // $(document).on("pagebeforechange", function(e, data) {
 //     var toPage = data.toPage[0].id;
 //     GetData(toPage);
 // });

 $("#configpage").on("pagebeforeshow", function () {

  $("#flip-enable-MQTT").on("change", function () {
   //$('#flip-enable-MQTT').change( function() {

   console.log("MQQT flip = " + $(this).val());
   // input_add_topic
   if ($(this).val() == "on") {
    $('#mqtt_ip').closest('div.ui-field-contain').show();
    $('#mqtt_port').closest('div.ui-field-contain').show();
    $('#input_add_topic').closest('div.ui-field-contain').show();
    $('#topics_text').show();
    $("#button_add_topic").show();
    $('#topic_list').removeClass('ui-screen-hidden').trigger('change');

   } else if ($(this).val() == "off") {
    $('#mqtt_ip').closest('div.ui-field-contain').hide();
    $('#mqtt_port').closest('div.ui-field-contain').hide();
    $('#input_add_topic').closest('div.ui-field-contain').hide();
    $('#topics_text').hide();
    $("#button_add_topic").hide();
    $('#topic_list').addClass('ui-screen-hidden').trigger('change');
   }

  });

 });

 $("#timer").on("pagebeforeshow", function () {

  // hide all options first
  $('#timeroptionsdiv').children('div').each(function (index) {
   $(this).closest("div").addClass("ui-state-disabled");
   $(this).hide();
  });

  if (globaldata.hasOwnProperty('modes')) {
   temp = globaldata.modes;
   $("#select-timer-option").find('option').remove();
   $.each(temp, function (key, value) {
    $("#select-timer-option").append('<option value=' + value + '>' + value + '</option>');
   });

   $('#select-timer-option').selectmenu('refresh');
  }

  //  GetData();

  $("#flip-enable-timer").change(function () {
   $("#timer-time-left").empty();
   console.log("Changeed to " + $(this).val());
   if ($(this).val() == "on") {
    $('#maintimerdiv').children('div').each(function (index) {
     $(this).show();
    });
   } else {
    $('#maintimerdiv').children('div').each(function (index) {
     $(this).hide();
    });
   }
  });

  $('#select-timer-command').change(function () {

   var option = $(this).val();

   $('#timeroptionsdiv').children('div').each(function (index) {
    $(this).closest("div").addClass("ui-state-disabled");
    $(this).hide();
   });

   $('#timeroptionsdiv').children('div').each(function (index) {
    if ($(this).hasClass(option)) {
     $(this).show();
     $(this).closest("div").removeClass("ui-state-disabled");

    }
   });

  });

 });

 $("#presetspage").on("pagebeforeshow", function () {

  // #savepage_effects_list_select
  // #select-saveasnew
  // #input_savename

  //               <option value="new" selected="selected">New</option>
  //               <option value="overwrite">Overwrite Existing</option>
  //               <option value="delete">Delete Existing</option>

  $('#select-saveasnew').change(function () {
   var command = $(this).val();

   if (command == "new") {
    $('#input_savename').closest('div.ui-field-contain').show();
    $('#savepage_effects_list_select').closest('div.ui-field-contain').hide();

   }

   if (command == "overwrite") {
    $('#input_savename').closest('div.ui-field-contain').show();
    $('#savepage_effects_list_select').closest('div.ui-field-contain').show();
    //$('#input_savename').val( $('#savepage_effects_list_select').val() );

    if (presetspagedata.hasOwnProperty('Presets')) {

     $.each(presetspagedata.Presets, function (key, value) {
      //console.log("key = " + key + " value = " , value);
      if ($('#savepage_effects_list_select').val() == value.ID) {
       $('#input_savename').val(value.name);
      }

     });
    }

   }

   if (command == "delete" || command == "load") {
    $('#input_savename').closest('div.ui-field-contain').hide();
    $('#savepage_effects_list_select').closest('div.ui-field-contain').show();
   }

   if (command == "deleteall") {
    $('#input_savename').closest('div.ui-field-contain').hide();
    $('#savepage_effects_list_select').closest('div.ui-field-contain').hide();
   }

  });

  $('#savepage_effects_list_select').change(function () {

   selected = $(this).val();
   console.log("selected = " + selected);

   if (presetspagedata.hasOwnProperty('Presets')) {

    $.each(presetspagedata.Presets, function (key, value) {
     console.log("key = " + key + " value = ", value);
     if (selected == value.ID) {
      $('#input_savename').val(value.name);
     }

    });
   }

   //$('#input_savename').val($(this).val());
  });

  //               presetspagedata = result;
  // $("#savepage_effects_list_select").find('option').remove();
  // if (result.hasOwnProperty('Presets')) {
  //       $.each(result.Presets, function(key, value) {
  //           var isconnected = " ";
  //           if (result.hasOwnProperty('settings')) {
  //           if (result.settings.currentpreset == value.name) isconnected = "selected=\"selected\"";
  //           }
  //             console.log("KEY = " + key + ", VALUE = " + value);
  //           $("#savepage_effects_list_select").append('<option value=' + key + " " + isconnected + '>' + value.name +"  (" + value.effect + ")" + '</option>');
  //       });
  //   $('#savepage_effects_list_select').selectmenu('refresh');

  // }

 });

 $("#homepage").on("pagebeforeshow", function () {

  // if (navigator.onLine) {
  //   console.log("Online");
  // } else {
  //   console.log("Offline");
  // }

  $("#flip-enable").on("change", flipChanged);

  if (localStorage.founddevices > 1) {

   $("#device_list_button").show();

  } else {
   $("#device_list_button").hide();
  }

  var mode = $('#modeslist').val();

  $('#effectoptions').children('div').each(function (index) {
   $(this).hide();
  });

  $('#effectoptions').children('div').each(function (index) {
   if ($(this).hasClass(mode)) $(this).show();
  });

  //   $('#input_udp_usemulticast').change( function() {

  // if ( $(this).val() == "true" ) {
  //     $('#udp_multicast_ip_addr').closest('.ui-field-contain').show();
  //     } else {
  //     $('#udp_multicast_ip_addr').closest('.ui-field-contain').hide();
  //     }

  //   });

  //   $('#input_dmx_usemulticast').change( function() {

  // if ( $(this).val() == "true" ) {
  //     $('#dmx_multicast_ip_addr').closest('.ui-field-contain').show();
  //     } else {
  //     $('#dmx_multicast_ip_addr').closest('.ui-field-contain').hide();
  //     }
  //   });

  // $("#brightness").on('slidestop', function() {
  //     $("#status").empty().append("Waiting...").css("color", "blue");;
  //     $.post("data.esp", $(this).serialize())
  //         .success(function(e) {
  //             processJSON(e);
  //             $("#status").empty().append("Success").css("color", "green");;
  //         })
  //         .error(function() {
  //             $("#status").empty().append("Error").css("color", "red");;
  //         })
  //         .complete(function() {});
  // });

  // $("#speed").on('slidestop', function() {
  //     $("#status").empty().append("Waiting...").css("color", "blue");;
  //     $.post("data.esp", $(this).serialize())
  //         .success(function(e) {
  //             processJSON(e);
  //             $("#status").empty().append("Success").css("color", "green");;
  //         })
  //         .error(function() {
  //             $("#status").empty().append("Error").css("color", "red");;
  //         })
  //         .complete(function() {});
  // });
 });

 function SendColor() {

  $.post(_currentdevice + "data.esp", $(this).serialize()).success(function (e) {
    processJSON(e)
    $("#status").empty().append("Success").css("color", "green");;
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");;
   })
   .complete(function () {});
 }

 function GetDynamicData() {

 }

 // function GetDataOnly() {
 //   $.getJSON("melvide.esp?plain=data", function(result) {

 //         }).success(function() { $("#status").empty().append("Connected").css("color", "green");;  })
 //     .error(function() {   $("#status").empty().append("Not Connected").css("color", "red");;  })
 //     .complete(function() {  });
 // }
 //$('#homepage').live('pageshow', function() {
 //});
 function toHHMMSS(seconds) {
  var h, m, s, result = '';
  // HOURs
  h = Math.floor(seconds / 3600);
  seconds -= h * 3600;
  if (h) {
   result = h + "h ";
  }
  // MINUTEs
  m = Math.floor(seconds / 60);
  seconds -= m * 60;
  if (m) {
   result += m < 10 ? '0' + m + ' min' : m + ' min';
  }
  // SECONDs
  //  s=seconds%60;
  //  result += s<10 ? '0'+s : s;
  // var min = h * 60 + m;

  //return min;
  return "Last " + result;

 }

} // end of rundocumentt

function flipChanged(e) {

 $("#status").empty().append("Waiting ").css("color", "blue");;
 $.post(_currentdevice + "data.esp", $(this).serialize(), function (data) {
   //console.log("Data Sent");
  }).success(function (e) {
   $("#status").empty().append("Success").css("color", "green");;
   //GetData();
   processJSON(e);
  })
  .error(function () {
   $("#status").empty().append("Error").css("color", "red");;
  })
  .complete(function () {});
}

function processJSON(result) {

 if (!result) {
  return;
 }

 // console.log("              DUMP         ");
 // console.log(result);
 // console.log("              DUMP END");
 // trying to get the json for settings in before
 if (result.hasOwnProperty('settings')) {
  settings = result.settings;
  //console.log("Settings Allocated");
 }

 if (result.hasOwnProperty('devices')) {
  localStorage["devicelist"] = JSON.stringify(result.devices);
  _offlineDeviceList = result.devices;
 }

 // else {
 //   localStorage["devicelist"] = {};
 //   _offlineDeviceList = {};
 // }

 if (result.hasOwnProperty('founddevices')) {
  localStorage.founddevices = result.founddevices;
  if (result.founddevices > 1) {
   $("#device_list_button").show();
   //populatedevicelist(result.devices);
  } else {
   $("#device_list_button").hide();
   //localStorage.founddevices = 0;
  }

 }

 if (!pagevar) {
  pagevar = $.mobile.activePage.attr('id');
 }

 //console.log("RESULT:")
 //console.log(result);
 // if (pagevar === "all") {
 //      globaldata = result;
 //console.log("GLOBALDATA:");
 //console.log(globaldata);
 //  }

 if ($.mobile.activePage.attr('id') == 'timer') {

  $("#timer-time-left").empty();

  if (result.hasOwnProperty("timer")) {
   values = result.timer;
   if (values.hasOwnProperty('running')) {

    $("#flip-enable-timer").val((values.running) ? "on" : "off").flipswitch('refresh').trigger("change");
   }

   // add time remaining in here
   if (values.hasOwnProperty('remaining')) {
    $("#timer-time-left").empty().append(" " + values.remaining[0] + "min " + values.remaining[1] + "sec remaining");

   }
  }

  $("#select-timer-preset").find('option').remove();
  if (result.hasOwnProperty('Presets')) {

   $.each(result.Presets, function (key, value) {
    var isconnected = " ";
    if (result.hasOwnProperty('settings')) {
     if (result.settings.currentpreset == value.ID) isconnected = "selected=\"selected\"";
    }
    //console.log("KEY = " + key + ", VALUE = " + value);
    $("#select-timer-preset").append('<option value=' + value.ID + " " + isconnected + '>' + value.name + "  (" + value.effect + ")" + '</option>');
   });
   $('#select-timer-preset').selectmenu('refresh');
  }
 }

 // Homepage

 if (result.hasOwnProperty('device')) {

  $('.devicename').empty().append(result.device);
  _deviceName = result.device;
  _currentdeviceName = result.device;

  if (_originalname == "") {
   _originalname = _deviceName;
  }
 }

 if ($.mobile.activePage.attr('id') == 'homepage') {

  if (result.hasOwnProperty('modes') && result.hasOwnProperty('settings')) {
   temp = result.modes;
   $("#modeslist").find('option').remove();
   //console.log("currentmode = " + result.currentmode);
   $.each(temp, function (key, value) {
    var isconnected = " ";
    if (result.hasOwnProperty('settings')) {
     if (result.settings.effect == value) isconnected = "selected=\"selected\"";
    }

    $("#modeslist").append('<option value=' + value + " " + isconnected + '>' + value + '</option>');
   });

   $('#modeslist').selectmenu('refresh');

   if (result.settings.effect != "Off") {
    $("#flip-enable").off("change").val("on").flipswitch('refresh').on("change", flipChanged); //.trigger("change");

   } else {
    $("#flip-enable").off("change").val("off").flipswitch('refresh').on("change", flipChanged); //.trigger("change");
   }

  }

  // add time remaining in here
  if (result.hasOwnProperty("timer") && result.timer.hasOwnProperty('remaining')) {
   $('#div_inserted_timer').remove();
   $("#content_homepage").prepend("<div id='div_inserted_timer' style='text-align: center'>  Timer " + result.timer.remaining[0] + "min " + result.timer.remaining[1] + "sec remaining </div>");
  } else {
   $('#div_inserted_timer').remove();
  }

  if (result.hasOwnProperty('settings')) {

   if (result.settings.hasOwnProperty("Palette")) {
    $("#palettebutton").show();
   } else {
    $("#palettebutton").hide();
   }
   if (result.settings.hasOwnProperty("Matrix")) {
    //if (result.settings.Matrix.enabled) {
    $("#matrixbutton").show();
   } else {
    $("#matrixbutton").hide();
    //}
   }

   if (result.settings.hasOwnProperty('EQ')) {
    $("#eqbutton").show();
   } else {
    $("#eqbutton").hide();
   }

   $('#effectoptions').children('div').each(function (index) {
    $(this).hide();
   });

   $('#effectoptions').children('div').each(function (index) {
    //if ($(this).hasClass(mode)) $(this).show();
    currentdiv = $(this);
    $.each(result.settings, function (key, value) {
     //console.log("[" + key + "] " + value);
     if (currentdiv.hasClass(key)) {
      currentdiv.show();
      //console.log("SHOW: " + key);
     };
    });

   });

   $.each(settings, function (key, value) {

    $('#effectoptions > .input_textbox').find("#input_" + key).closest('input').val(value);
    $('#effectoptions > .input_textbox_IP').find("#input_" + key).closest('input').val(value[0] + "." + value[1] + "." + value[2] + "." + value[3]);
    $('#effectoptions > .input_slider').find("#input_" + key).val(value).closest('input').slider("refresh");
    $('#effectoptions > .input_textbox_rgb').find("#input_" + key).val("rgb(" + value[0] + "," + value[1] + "," + value[2] + ")").colorPicker();
    $('#effectoptions > .input_selectmenu').find("#input_" + key).closest('select').val(value.toString()).selectmenu('refresh');
    $('#effectoptions > .input_selectmenu_palette').find("#input_" + key).closest('select').val(value.mode).selectmenu('refresh');
   });

  }

  if (result.hasOwnProperty('currentpresets') && result.currentpresets.length) {
   //console.log("Currentpreset length " + result.currentpresets.length);

   $("#currentpresetslist").find('option').remove();
   //console.log("currentmode = " + result.currentmode);
   $("#currentpresetslist").append('<option value=255>  none </option>');
   $.each(result.currentpresets, function (key, value) {
    var isconnected = " ";
    if (result.hasOwnProperty('settings')) {
     if (result.settings.currentpreset == value.ID) isconnected = "selected=\"selected\"";
    }

    $("#currentpresetslist").append('<option value=' + value.ID + " " + isconnected + '>' + value.name + '</option>');
   });
   $('#currentpresetslist').selectmenu('refresh');

   $(".presetlistclass").show();

  } else {

   $("#currentpresetslist").find('option').remove();
   $('.presetlistclass').hide();

  }
  //

  if (result.hasOwnProperty('heap')) {
   $("#heap").empty().append(result.heap).css("color", "grey");
  }
  if (result.hasOwnProperty('power')) {
   $("#power").empty().append(result.power).css("color", "orange");
  }

 }

 //presetspage

 if ($.mobile.activePage.attr('id') == 'presetspage') {
  presetspagedata = result;
  $("#savepage_effects_list_select").find('option').remove();
  if (result.hasOwnProperty('Presets')) {
   $.each(result.Presets, function (key, value) {
    var isconnected = " ";
    if (result.hasOwnProperty('settings')) {
     if (result.settings.currentpreset == value.ID) isconnected = "selected=\"selected\"";
    }
    //console.log("KEY = " + key + ", VALUE = " + value);
    $("#savepage_effects_list_select").append('<option value=' + value.ID + " " + isconnected + '>' + value.name + "  (" + value.effect + ")" + '</option>');
   });
   $('#savepage_effects_list_select').selectmenu('refresh');

  }
  if (result.hasOwnProperty('settings') && settings.hasOwnProperty('currentpresetname')) {
   $('#presetpage_currentEffect').empty().append("Current Effect: " + result.settings.currentpresetname); // .css("color", "red");;
  }
 }

 // allpresetspage

 if ($.mobile.activePage.attr('id') == 'allpresetspage') {

  $("#allpresets_list").empty();

  if (result.hasOwnProperty('Presets')) {
   $.each(result.Presets, function (key, value) {
    $("#allpresets_list").append('<li><a href="#" data-rel="back" onclick=loadpreset(' + value.ID + ')>' + value.ID + '. ' + value.name + ' [' + value.effect + '] </a></li>');
    //console.log("KEY = " + key + ", VALUE = " + value);
    //$("#savepage_effects_list_select").append('<option value=' + value.ID + " " + isconnected + '>' + value.name +"  (" + value.effect + ")" + '</option>');
   });

  }

  $("#allpresets_list").listview("refresh");

 }

 if ($.mobile.activePage.attr('id') == 'configpage') {
  $("#input_add_topic").val("");
  console.log("DATA fetched for configpage");
  console.log(result);
  if (result.hasOwnProperty('pixels')) {
   $("#nopixels").val(result.pixels);
  }

  //<label for="mqtt_ip">Server IP:</label>flip-enable-MQTT
  // "MQTT":{"enable":true,"ip":[192,168,1,24],"port":1883}}
  if (result.hasOwnProperty('MQTT')) {

   var mqtt = result.MQTT;

   if (mqtt.hasOwnProperty('enabled')) {
    $("#flip-enable-MQTT").val((mqtt.enabled) ? "on" : "off").flipswitch('refresh').trigger("change");
   }

   if (mqtt.hasOwnProperty('ip')) {
    $("#mqtt_ip").val(mqtt.ip[0] + "." + mqtt.ip[1] + "." + mqtt.ip[2] + "." + mqtt.ip[3]);
   }

   if (mqtt.hasOwnProperty('port')) {
    $("#mqtt_port").val(mqtt.port);
   }

   if (mqtt.hasOwnProperty("topics")) {
    $("#topic_list").empty();

    $.each(mqtt.topics, function (key, value) {
     console.log(key + ":" + value);
     $("#topic_list").append("<li><a><h2>" + value + "</h2></a><a onClick=\"RemoveTopic('" + encodeURIComponent(value) + "'); return false;\" ></a></li>");
    });

    //<li><a><h2>Broken Bells</h2></a><a onClick="RemoveTopic('lalal/bc/abc'); return false;" ></a></li>

    $("#topic_list").listview("refresh");

   }
  }

 } // end of config page

 // Layout Page
 if ($.mobile.activePage.attr('id') == 'matrixpage') {

  if (result.hasOwnProperty('settings')) {

   if (result.settings.hasOwnProperty('Matrix')) {

    console.log("updating matrix page");

    var matrixresult = result.settings.Matrix;

    if (matrixresult.hasOwnProperty('multiple')) {
     if (matrixresult.multiple === true) {

      $("#select-matrix-type").val('multiplematrix').selectmenu("refresh");

      console.log("TRUE multiple = " + matrixresult.multiple);

     } else {
      $("#select-matrix-type").val('singlematrix').selectmenu("refresh");

      console.log("FALSE multiple = " + matrixresult.multiple);

     }

    }

    if (matrixresult.hasOwnProperty('x')) {
     $("#gridx").val(matrixresult.x);
    }
    if (matrixresult.hasOwnProperty('y')) {
     $("#gridy").val(matrixresult.y);
    }
    // if (matrixresult.hasOwnProperty('config')) {
    //     $("#select-matrix-type").val(matrixresult.config).selectmenu("refresh");
    // }

    if (matrixresult.hasOwnProperty('firstpixel')) {
     $("#select-singlematrix").val(matrixresult.firstpixel).selectmenu("refresh");
    }
    if (matrixresult.hasOwnProperty('axis')) {
     $("#select-singlematrix-axis").val(matrixresult.axis).selectmenu("refresh");
    }
    if (matrixresult.hasOwnProperty('sequence')) {
     $("#select-singlematrix-seq").val(matrixresult.sequence).selectmenu("refresh");
    }

    if (matrixresult.hasOwnProperty('multimatrixtile')) {
     $("#select-multimatrix").val(matrixresult.multimatrixtile).selectmenu("refresh");
    }
    if (matrixresult.hasOwnProperty('multimatrixaxis')) {
     $("#select-multimatrix-axis").val(matrixresult.multimatrixaxis).selectmenu("refresh");
    }
    if (matrixresult.hasOwnProperty('multimatrixseq')) {
     $("#select-multimatrix-seq").val(matrixresult.multimatrixseq).selectmenu("refresh");
    }

    sortmatrix('#select-matrix-type');
   }
  }
 }
 //palette
 if ($.mobile.activePage.attr('id') == 'palette') {
  console.log("Active page = palette");
  console.log(result);
  if (result.hasOwnProperty('settings') && result.settings.hasOwnProperty('Palette')) {

   palettevars = result.settings.Palette;

   if (palettevars.hasOwnProperty('mode')) {
    $("#select-palettes2").val(palettevars.mode).selectmenu("refresh");
   } else {
    $("#select-palettes2").val("off").selectmenu("refresh");
   }

   if (palettevars.hasOwnProperty('randmodeString')) {
    $("#select-palettes-randomness").val(palettevars.randmodeString).selectmenu("refresh");
    //console.log("randmodeString = " + palettevars.randmodeString);
   } else {
    //console.log("randmodeString not found in palettevars");
    $("#select-palettes-randomness").val("off").selectmenu("refresh");

   }
   if (palettevars.hasOwnProperty('range')) {
    $("#input-palette-spread").val(palettevars.range);
   }

   if (palettevars.hasOwnProperty('delay')) {
    $("#input-palette-delay").val(palettevars.delay);
   }

  } else {
   //console.log("NO PALETTE");
   $("#select-palettes2").val("off").selectmenu("refresh");
   $("#select-palettes-randomness").val("off").selectmenu("refresh");

  }

 }
 //eqpage
 if ($.mobile.activePage.attr('id') == 'eqpage') {

  if (result.hasOwnProperty('settings') && result.settings.hasOwnProperty('EQ')) {

   EQ = result.settings.EQ;

   if (EQ.hasOwnProperty('resetpin')) {
    $("#field-resetpin").empty().append(EQ.resetpin + " ").css("color", "red");
   }

   if (EQ.hasOwnProperty('strobepin')) {
    $("#field-strobepin").empty().append(EQ.strobepin + " ").css("color", "red");
   }

   if (EQ.hasOwnProperty('eqmode')) {
    //$("#flip-enable-EQ").val((EQ.enablebeats) ? "on" : "off").flipswitch('refresh').trigger("change");
    $("#flip-enable-EQ").val(EQ.eqmode).selectmenu("refresh").trigger("change");
   }

   if (EQ.hasOwnProperty('peakfactor')) {
    $("#text-input-peakfactor").val(EQ.peakfactor);
   }

   if (EQ.hasOwnProperty('beatskiptime')) {
    $("#text-input-beatskiptime").val(EQ.beatskiptime);
   }

   if (EQ.hasOwnProperty('samples')) {
    $("#text-input-samples").val(EQ.samples);
   }

   if (EQ.hasOwnProperty('sampletime')) {
    $("#text-input-sampletime").val(EQ.sampletime);
   }

   textstring = "";

   if (EQ.hasOwnProperty("eqmode")) {

    if (EQ.eqmode == 2) {
     textstring = "RECEIVE ";
    } else if (EQ.hasOwnProperty('eq_send_udp') && EQ.eq_send_udp) {
     textstring = "SEND ";
    } else {
     textstring = "[Disabled";
    }

   }

   if (textstring != "[Disabled") {
    textstring += "[" + EQ.eq_addr[0] + "." + EQ.eq_addr[1] + "." + EQ.eq_addr[2] + "." + EQ.eq_addr[3] + ": " + EQ.eq_port;

   }

   $("#field-udpsend").empty().append(textstring);

  }

 }
 //  udp send page

 if ($.mobile.activePage.attr('id') == 'equdpsendpage') {

  if (result.hasOwnProperty('settings') && result.settings.hasOwnProperty('EQ')) {

   EQ = result.settings.EQ;

   if (EQ.hasOwnProperty('eq_send_udp')) {
    $("#flip-enable-eq_send_udp").val((EQ.eq_send_udp) ? "on" : "off").flipswitch('refresh').trigger("change");
   }

   if (EQ.hasOwnProperty('eq_port')) {
    $("#text-input-eqport2").val(EQ.eq_port);
   }

   if (EQ.hasOwnProperty('eq_addr')) {
    $("#text-input-eq_addr2").val(EQ.eq_addr[0] + "." + EQ.eq_addr[1] + "." + EQ.eq_addr[2] + "." + EQ.eq_addr[3]);
   }
  }
 }
}

var pagevar;
var _waiting_data = false;

function GetData(pagevar) {

 //            console.log("pagevar = " + pagevar);
 // if (!pagevar) {
 //       pagevar = $.mobile.activePage.attr('id');
 //       //console.log("Pagevar empty");
 //   } else {
 //       //console.log("Pagevar NOT empty");
 //     // return;
 //   }
 _waiting_data = true;

 $("#status").empty().append("Waiting ").css("color", "blue");;

 if (_currentdevice) {

  $.post(_currentdevice + "data.esp", "data=" + pagevar, function (result) {

    processJSON(result);

   }).success(function () {
    $("#status").empty().append("Success").css("color", "green");
    setTimeout(function () {
     _waiting_data = false;
    }, 10000);
   })
   .error(function () {
    $("#status").empty().append("Error").css("color", "red");
    _waiting_data = false;
   })
   .complete(function () {});

 }

};

function sortmatrix(e) {

 var mode = $(e).val();
 //console.log("Matrix mode changed:" + mode);

 if (mode === "multiplematrix") {
  $('#singlematrix-config').fadeIn();
  $('#multimatrix-config').fadeIn();
 }

 if (mode === "singlematrix") {
  $('#singlematrix-config').fadeIn();
  $('#multimatrix-config').fadeOut();
 }

}

function populatedevicelist(data) {

 var isconnected = "";

 $('#device_list_fieldset').empty().append("<legend>Choose Device:</legend>").css("color", "black");

 $.each(data, function (key, value) {
  name = value.name.trim();

  if (name == _currentdeviceName) {
   isconnected = "checked=\"checked\"";
   console.log("matched: name =" + name + ", _currentdeviceName=" + _currentdeviceName);
  } else {
   isconnected = "";
  }

  ip = value.IP[0] + "." + value.IP[1] + "." + value.IP[2] + "." + value.IP[3];
  $("#device_list_fieldset").append("<input type=\"radio\" data-mini=\"true\" name=\"device\" class=\"deviceselectclass\" id=\"radio-choice-v-" + key + "a\" value=\"" +
   ip + "\"" + isconnected + "><label for=\"radio-choice-v-" + key + "a\">" + name + "</label>");

 });

 $("#device_list_fieldset").enhanceWithin();

}
