import { db } from "./firebase.js"
import { collection, addDoc, deleteDoc, updateDoc, doc } from 'https://www.gstatic.com/firebasejs/10.7.0/firebase-firestore.js';
import { arcRadius, arcX, arcY, drawRange, isInsideCircle} from './common.js'

const canvas = document.getElementById('sonarCanvas');
const ctx = canvas.getContext('2d');
const modeRadios = document.getElementsByName('mode');
const errorMessage = document.getElementById('errorMessage');
const nameInput = document.getElementById('name');
const startTimeInput = document.getElementById('startTime');
const endTimeInput = document.getElementById('endTime');

const EXPECTED_OJECT_DB = 'expected_objects';

function getFormattedTime(date) {
    let hours = date.getHours();
    let minutes = date.getMinutes();

    // Adding leading zero if the value is less than 10
    hours = hours < 10 ? '0' + hours : hours;
    minutes = minutes < 10 ? '0' + minutes : minutes;

    return hours + ':' + minutes;
}

const form = document.getElementById('circleForm');
const objectInfo = document.getElementById('objectInfo');

function toggleVisibility() {
    if (document.getElementById('addMode').checked) {
        form.style.display = 'block';
        objectInfo.style.display = 'none';
    } else {
        form.style.display = 'none';
        objectInfo.style.display = 'block';
        // Add logic to populate objectInfo fields when a circle is selected
    }
}

// Initial check to set the correct display mode
toggleVisibility();

modeRadios.forEach(radio => {
    radio.addEventListener('change', toggleVisibility);
});


function resetForm() {
    let startTime = new Date();
    let endTime = new Date();
    endTime.setMinutes(endTime.getMinutes() + 1);

    nameInput.value = ""
    startTimeInput.value = getFormattedTime(startTime);
    endTimeInput.value = getFormattedTime(endTime)
}

resetForm();
// Draw semi-circle for sonar range
drawRange(ctx)
let currentMode = 'add';

// Listen for mode changes
modeRadios.forEach(radio => {
    radio.addEventListener('change', function() {
        currentMode = this.value;
    });
});

// Check if the coordinates are within the semi-circle
function isInSemiCircle(x, y) {
    // Calculate distance from the semi-circle's center (bottom center of canvas)
    const dx = x - arcX;
    const dy = y - arcY;
    const distanceFromCenter = Math.sqrt(dx * dx + dy * dy);
    
    return y <= arcY && distanceFromCenter <= arcRadius;
}

function getAngle(x, y) {
    let theta = Math.atan2(arcY - y, x - arcX); // Arc center is (arcX, arcY)
    if (theta < 0) { // Convert from [-pi, pi] to [0, 2*pi]
        theta += 2 * Math.PI;
    }
    return Math.floor(theta * (180 / Math.PI)); // Convert radians to degrees
}

function getDistanceFromSonar(x, y) {
    const dx = x - arcX;
    const dy = arcY - y; // invert the y-axis because canvas y increases downwards
    return Math.floor(Math.sqrt(dx * dx + dy * dy));
}

function getExpectedObjects() {
    
}

// Array to hold the circles
let circles = getExpectedObjects();



// Function to add a circle
function addCircle(x, y, radius, startTime, endTime, name) {
    if (!isInSemiCircle(x,y)) return; // Ensures circles are only in the semi-circle
    circles.push({x, y, radius, startTime, endTime, name});

    const startAngle = getAngle(x - radius, y); // Left point of the circle
    const endAngle = getAngle(x + radius, y); // Right point of the circle
    console.log(db)
    addDoc(collection(db, EXPECTED_OJECT_DB), {
        name: name,
        start_time: startTime,
        end_time: endTime,
        start: startAngle,
        end: endAngle,
        distance: getDistanceFromSonar(x,y)
    }).then((docRef) => {
        // Store the document reference ID within the circle object
        // maybe find more robust way to do this
        circles[circles.length - 1].docId = docRef.id;
        console.log("added to firestore")
        console.log(circles[circles.length - 1]);
    }).catch((error) => {
        console.error("Error adding document: ", error);
    });
    console.log(name)
    resetForm();
    drawCircles();
}

function getExpectedCircleDoc(circle) {
    return doc(db, EXPECTED_OJECT_DB, circle.docId)
}

// Function to remove a circle
function removeCircle(x, y) {
    circles = circles.filter(circle => {
        const distance = Math.sqrt((x - circle.x) ** 2 + (y - circle.y) ** 2);
        const remove = distance <= circle.radius;
        if (remove) {
            deleteDoc(getExpectedCircleDoc(circle)).then(() => {
                console.log("Document successfully deleted!");
                console.log(circle.name)
            }).catch((error) => {
                console.error("Error removing document: ", error);
            });
        }
        return !remove;
    });
    drawCircles();
}

function activeCircle(circle) {
    return true
}


// Function to draw all circles
function drawCircles() {
    // Remove undesired circles
    circles = circles.filter(circle => {
        return activeCircle(circle)
    })

    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.beginPath();
    ctx.arc(arcX, arcY, arcRadius, Math.PI, 2 * Math.PI);
    ctx.closePath();
    ctx.stroke();
    
    circles.forEach(circle => {
        ctx.beginPath();
        ctx.arc(circle.x, circle.y, circle.radius, 0, 2 * Math.PI);
        ctx.fill();
    });
}

function getDateFromInput(timeString) {
    console.log(timeString)
    const [hours, minutes] = timeString.split(':').map(Number);
    const date = new Date();
    date.setHours(hours, minutes, 0, 0); // Set hours and minutes, reset seconds and milliseconds to 0
    return date;
}

function onAddCircle(x, y) {
    let goodCircle = true;
    errorMessage.innerHTML = "";
    if(nameInput.value == "") {
        errorMessage.innerText += "Please specify the name of the new object. \n";
        goodCircle = false;
    }
    let endTime = getDateFromInput(endTimeInput.value);
    if(endTime <= new Date()) {
        errorMessage.innerText += "Please make sure the end time is in the future.\n";
        goodCircle = false;
    }
    let startTime = getDateFromInput(startTimeInput.value)
    if(endTime <= startTime) {
        errorMessage.innerText += "Please make sure the end time is after the start time.\n";
        goodCircle = false;
    }

    if(goodCircle) addCircle(x, y, 20, startTime, endTime, nameInput.value); // Default radius 20
}

// Event listener for canvas click
canvas.addEventListener('click', function(event) {
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    switch(currentMode) {
        case 'add':
            onAddCircle(x, y);
            break;
        case 'remove':
            removeCircle(x, y);
            break;
        case 'info':
            updateInfo(x, y);
            break;
        case 'resize':
            // Implement resize logic
            break;
    }
});
function updateInfo(x, y) {
    const selectedCircle = circles.find(circle => isInsideCircle(x, y, circle));
    if (selectedCircle) {
      const formattedStartTime = getFormattedTime(selectedCircle.startTime);
      const formattedEndTime = getFormattedTime(selectedCircle.endTime);
      objectInfo.innerHTML = `
        <h2>Selected Object Information</h2>
        <p>Name: ${selectedCircle.name}</p>
        <p>Start Time: ${formattedStartTime}</p>
        <p>End Time: ${formattedEndTime}</p>
        <p>Start Angle: ${getAngle(selectedCircle.x - selectedCircle.radius, selectedCircle.y)}</p>
        <p>End Angle: ${getAngle(selectedCircle.x + selectedCircle.radius, selectedCircle.y)}</p>
      `;
      objectInfo.style.display = 'block';
    } else {
      objectInfo.innerHTML = '<p>No object selected</p>';
    }
}
// TODO: Implement resizing and deletion of circles
let draggingCircle = null;
let offsetX, offsetY;



// Mouse down event
canvas.addEventListener('mousedown', function(event) {
    if(currentMode != 'move') return;
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;

    for (let circle of circles) {
        if (isInsideCircle(x, y, circle)) {
            draggingCircle = circle;
            offsetX = x - circle.x;
            offsetY = y - circle.y;
            canvas.addEventListener('mousemove', onMouseMove);
            break;
        }
    }
});

// Mouse move event
function onMouseMove(event) {
    if (!draggingCircle) return;

    const rect = canvas.getBoundingClientRect();
    const newX = event.clientX - rect.left - offsetX;
    const newY = event.clientY - rect.top - offsetY;
    if (!isInSemiCircle(newX,newY)) return; // Ensures circles are only in the semi-circle

    draggingCircle.x = newX;
    draggingCircle.y = newY;
    drawCircles();
}

function updateCircleInFirestore(circle) {
    const circleRef = getExpectedCircleDoc(circle);
    const startAngle = getAngle(circle.x - circle.radius, circle.y);
    const endAngle = getAngle(circle.x + circle.radius, circle.y);

    updateDoc(circleRef, {
        start: startAngle,
        end: endAngle,
        distance: getDistanceFromSonar(circle.x, circle.y)
    }).then(() => {
        console.log("Document successfully updated!");
    }).catch((error) => {
        console.error("Error updating document: ", error);
    });
}

// Mouse up event
canvas.addEventListener('mouseup', function() {
    if(draggingCircle) updateCircleInFirestore(draggingCircle)
    draggingCircle = null;
    canvas.removeEventListener('mousemove', onMouseMove);
});
