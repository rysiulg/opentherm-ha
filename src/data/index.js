/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

// Complete project details: https://randomnerdtutorials.com/esp8266-nodemcu-web-server-websocket-sliders/

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var programaticallychange = false;
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getValues(){
    websocket.send("getValues");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValues();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateSliderPWM(element) {
    var topic, message;
    topic = element.id;
    message = element.value;
    if ((element.id).indexOf('slider') >= 0 )
    {
        var sliderNumber = element.id.charAt(element.id.length-1);
        var sliderValue = document.getElementById(element.id).value;
        document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
        document.getElementById(element.id).setAttribute('value',parseFloat(sliderValue).toFixed(1));
        topic = "sliderValue"+sliderNumber;
        message = sliderValue.toString();
    }
    //for checkbox change messaqge from value to chcecked -in other case you have always have on ;(
    if ( element?.tagName?.toLowerCase() === 'input' && element?.getAttribute('type') === 'checkbox')
    {
        message = element.checked?"on":"off";
    }
    //console.log("topic: "+topic+" message: "+message+" "+element.val+" "+element.checked);
    if (topic != null && message != null) websocket.send(topic+":"+message);
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        //var str=document.getElementById(key); str = str.replace(/^\s*|\s*$/g,""); if (str == "") { alert("I'm so brilliant"); }
        if (document.getElementById(key)!=null) {
            document.getElementById(key).innerHTML= myObj[key];
             if (key.trim() == ("sliderValue"+ key.charAt(key.length-1)).trim()) {
                 document.getElementById("slider"+ key.charAt(key.length-1)).value = myObj[key];
                 document.getElementById("slider"+ key.charAt(key.length-1)).setAttribute('value', myObj[key]);
             } else
             if (key.trim() === "boilerhwwww") { //CWU water
                console.log(key+" Stan HW: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById(key).checked = true;
                    document.getElementById("HotWater_title").style.background = "#f7e08c";
                } else {
                    document.getElementById(key).checked = false;
                    document.getElementById("HotWater_title").style.background = null;
                }
          //      document.getElementById(key).focus();
          //      document.getElementById(key).click();
             } else
             if (key.trim() === "boilermodewww") {
                console.log(key+" Stan mode: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById(key).checked = true;
                } else {
                    document.getElementById(key).checked = false;
                }
            } else
            if (key.trim() === "boilerwww") {
                console.log(key+" Stan mode: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById(key).checked = true;
                    document.getElementById("BoilerCO_title").style.background = "#8bf6cd";
                } else {
                    document.getElementById(key).checked = false;
                    document.getElementById("BoilerCO_title").style.background = null;
                }
            } else
            if (key.trim() === "statusWaterActive") {
                console.log(key+" Stan mode: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById("HotWater_card").style.background = "#f7e08c";
                } else {
                    document.getElementById("HotWater_card").style.background = null;
                }
            } else
            if (key.trim() === "statusCHActive") {
                console.log(key+" Stan mode: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById("BoilerCO_card").style.background = "#8bf6cd";
                } else {
                    document.getElementById("BoilerCO_card").style.background = null;
                }
            } else

            if (key.trim() === "statusFlameOn") {
                console.log(key+" Stan mode: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById("status_Flame").style.background = "#ff0000";
                } else {
                    document.getElementById("status_Flame").style.background = null;
                }
            } else

            if (key.trim() === "statusFault") {
                console.log(key+" Stan mode: "+myObj[key]);
                if (myObj[key].toLowerCase() === "on" || myObj[key] === "1") {
                    document.getElementById("status_Error").style.background = "#ff0000";
                } else {
                    document.getElementById("status_Error").style.background = null;
                }
            } else {
                document.getElementById(key).innerHTML= myObj[key];
            }
        }
    }
}


function setOneNumberDecimal(event) {
    this.value = parseFloat(this.value).toFixed(1);
}
const allRanges = document.querySelectorAll(".rangewrap");
//console.log(allRanges);
allRanges.forEach(wrap => {
    const bubble = wrap.querySelector(".rangevalue");
    const range = wrap.querySelector(".range");
  range.addEventListener("input", () => {
    setBubble(range, bubble);
  });
  setBubble(range, bubble);
});

function setBubble(range, bubble) {
    const val = parseFloat(range.value).toFixed(1);
    const min = range.min ? range.min : 0;
    const max = range.max ? range.max : 100;
    const newVal = Number(((val - min) * 100) / (max - min));

   /* if (bubble != null) {*/
        bubble.innerHTML = `<span>${parseFloat(val).toFixed(1)}</span>`;

    // Sorta magic numbers based on size of the native UI thumb
    bubble.style.left = `calc(${newVal}% + (${15 - newVal * 0.30}px))`;
//    console.log(bubble.style.left);
//    console.log("SetBubble "+document.getElementById("sliderValue"+ range.id.charAt(range.id.length-1)).value+" <- "+val.toString());
   document.getElementById("sliderValue"+ range.id.charAt(range.id.length-1)).value = `${parseFloat(range.value).toFixed(1)}`;


    /* } */
  }





  document.addEventListener("DOMContentLoaded", setBubble);




//   document.getElementById('input').addEventListener('change',function() {
//     this.setAttribute('value',this.value);
//   });


// const
//   range = document.getElementById('slider4'),
//   allRanges = document.querySelectorAll(".range"),
//   rangeV = document.getElementById('rangeV'),
//   setValue = ()=>{
//     const
//       newValue = Number( (range.value - range.min) * 100 / (range.max - range.min) ),
//       newPosition = 10 - (newValue * 0.2);
//     rangeV.innerHTML = `<span>${range.value}</span>`;
//     rangeV.style.left = `calc(${newValue}% + (${newPosition}px))`;
//   };
//   console.log(allRanges);
// document.addEventListener("DOMContentLoaded", setValue);
// range.addEventListener('input', setValue);
