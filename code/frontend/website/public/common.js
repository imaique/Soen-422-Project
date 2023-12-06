const arcX = 250;
const arcY = 250;
const arcRadius = 200;
const MAX_RANGE = 50;

function drawRange(ctx) {
    ctx.beginPath();
    ctx.arc(arcX, arcY, arcRadius, 0, Math.PI, true);
    ctx.closePath();
    ctx.stroke();
}

// Function to check if a point is inside a circle
function isInsideCircle(x, y, circle) {
    return Math.sqrt((x - circle.x) ** 2 + (y - circle.y) ** 2) < circle.radius;
}

export { arcX, arcY, arcRadius, drawRange, MAX_RANGE, isInsideCircle }