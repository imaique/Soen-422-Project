import { db } from "./firebase.js";
import { collection, query, where, onSnapshot } from 'https://www.gstatic.com/firebasejs/10.7.0/firebase-firestore.js';
import { arcX, arcY, arcRadius, drawRange, MAX_RANGE, isInsideCircle } from './common.js';

const canvas = document.getElementById('sonarCanvas');
const ctx = canvas.getContext('2d');
const logContainer = document.getElementById('logList');
const objectInfo = document.getElementById('objectInfo');

function angleToCanvasCoords(angle, distance) {
    // Convert the angle range from [0, 180] to [-90, 90] degrees
    // Then, convert from degrees to radians
    const adjustedAngle = angle;
    const angleRadians = adjustedAngle * (Math.PI / 180);

    // Calculate canvas coordinates
    const x = arcX + distance * Math.cos(angleRadians);
    const y = arcY - distance * Math.sin(angleRadians); // Subtract because canvas y-axis is inverted
    return { x, y };
}


function drawDetectedObject(data) {
    // Assuming average of start and end angles for the object's angle
    const avgAngle = (data.start + data.end) / 2;
    const canvasDistance = data.distance * arcRadius / MAX_RANGE
    const canvasCoords = angleToCanvasCoords(avgAngle, canvasDistance);

    data.x = canvasCoords.x 
    data.y = canvasCoords.y
    const startCoords = angleToCanvasCoords(data.start, canvasDistance);
    const dx = canvasCoords.x - startCoords.x;
    const dy = canvasCoords.y - startCoords.y;
    const radius = Math.sqrt(dx * dx, dy * dy)
    data.radius = radius

    const color = data.expected ? 'green' : 'red';
    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.arc(canvasCoords.x, canvasCoords.y, radius, 0, 2 * Math.PI);
    ctx.fill();
}
let objects = []

const q = query(collection(db, 'detected_objects'));
onSnapshot(q, (snapshot) => {
    drawRange(ctx); // Redraw range for each update
    logContainer.innerHTML = ''; // Clear the logs
    
    objects = []
    snapshot.forEach((doc) => {
        const data = doc.data();
        drawDetectedObject(data);
        data.timestamp = data.timestamp.toDate();
        data.docId = doc.id; // Store for reference on click
        objects.push(data)
        console.log(data)
    });

    objects.sort((a,b) => b.timestamp.getTime() - a.timestamp.getTime())

    objects.forEach((log) => {
        const logEntry = document.createElement('div');
        logEntry.textContent = `Object: ${log.name}, Time: ${log.timestamp.toLocaleString()}`;
        logContainer.appendChild(logEntry);
    })
});

canvas.addEventListener('click', (event) => {
    // Click handling logic to display object information
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    console.log(`click (${x},${y})`)
    const selectedCircle = objects.find(circle => isInsideCircle(x, y, circle));
    if (selectedCircle) {
      objectInfo.innerHTML = `
        <p>Name: ${selectedCircle.name}</p>
        <p>Timestamp: ${selectedCircle.timestamp.toLocaleString()}</p>
        <p>Start Angle: ${selectedCircle.start}</p>
        <p>End Angle: ${selectedCircle.end}</p>
      `;
      objectInfo.style.display = 'block';
    } else {
      objectInfo.innerHTML = '<p>No object selected</p>';
    }
});
