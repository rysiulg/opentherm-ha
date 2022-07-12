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
    var sliderNumber = element.id.charAt(element.id.length-1);
    var sliderValue = document.getElementById(element.id).value;
    document.getElementById("sliderValue"+sliderNumber).innerHTML = sliderValue;
    console.log(sliderValue);
    websocket.send("sliderValue"+sliderNumber+":"+sliderValue.toString());
}

function onMessage(event) {
    console.log(event.data);
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);

    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        //var str=document.getElementById(key); str = str.replace(/^\s*|\s*$/g,""); if (str == "") { alert("I'm so brilliant"); }
        if (document.getElementById(key)!=null) {
        // document.getElementById(key).innerHTML = myObj[key];
        // if (document.getElementById(key)!=null) {
            document.getElementById(key).value = myObj[key];
            document.getElementById(key).innerText = myObj[key];
            // if (key.trim() == ("sliderValue"+ key.charAt(key.length-1)).trim()) {
            //     console.log("slider"+ key.charAt(key.length-1) + "  "+myObj[key]);
            //     document.getElementById("slider"+ key.charAt(key.length-1)).value = myObj[key];
            // }
        //}
        }
    }
}


function setOneNumberDecimal(event) {
    this.value = parseFloat(this.value).toFixed(1);
}
const allRanges = document.querySelectorAll(".range-wrap");
//console.log(allRanges);
allRanges.forEach(wrap => {
    const bubble = wrap.querySelector(".range-value");
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
    console.log("SetBubble");
    console.log(document.getElementById("sliderValue"+ range.id.charAt(range.id.length-1)).value);
  //  document.getElementById("sliderValue"+ range.id.charAt(range.id.length-1)).value = `<span>${parseFloat(range.value).toFixed(1)}</span>`;

   /* if (bubble != null) {*/
        bubble.innerHTML = `<span>${parseFloat(range.value).toFixed(1)}</span>`;

    // Sorta magic numbers based on size of the native UI thumb
    bubble.style.left = `calc(${newVal}% + (${15 - newVal * 0.30}px))`;
   /* } */
  }

  document.addEventListener("DOMContentLoaded", setBubble);



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
