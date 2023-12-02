const canvas = document.getElementById('sonarCanvas');
const ctx = canvas.getContext('2d');

// Draw semi-circle for sonar range
ctx.beginPath();
ctx.arc(250, 250, 200, Math.PI, 2 * Math.PI);
ctx.closePath();
ctx.stroke();

// Array to hold the circles
let circles = [];

// Function to add a circle
function addCircle(x, y, radius) {
    if (y > 250) return; // Ensures circles are only in the semi-circle
    circles.push({x, y, radius});
    drawCircles();
}

// Function to draw all circles
function drawCircles() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.beginPath();
    ctx.arc(250, 250, 200, Math.PI, 2 * Math.PI);
    ctx.closePath();
    ctx.stroke();
    
    circles.forEach(circle => {
        ctx.beginPath();
        ctx.arc(circle.x, circle.y, circle.radius, 0, 2 * Math.PI);
        ctx.fill();
    });
}

// Event listener for canvas click
canvas.addEventListener('click', function(event) {
    const rect = canvas.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    addCircle(x, y, 20); // Default radius 20
});

// TODO: Implement resizing and deletion of circles
