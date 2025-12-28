const BACKGROUND = "#101010"
const FOREGROUND = "#50FF50"
console.log(game)
game.width=800
game.height=800
const ctx = game.getContext("2d")
console.log(ctx)

// clear screen
function clear() {
    ctx.fillStyle = BACKGROUND
    ctx.fillRect(0, 0, game.width, game.height)
}

// draw a point
function point({x, y}) {
    const s = 20
    ctx.fillStyle = FOREGROUND
    ctx.fillRect(x - s/2, y - s/2, s, s)
}

// draw a line from p1 to p2
function line(p1, p2){
    ctx.lineWidth = 3
    ctx.strokeStyle = FOREGROUND;
    ctx.beginPath();
    ctx.moveTo(p1.x, p1.y);
    ctx.lineTo(p2.x, p2.y);
    ctx.stroke();
}

// transform to screen coordinates
function screen(p) {
    // transform x: [-1,1] -> [0,w]; y: [-1,1] -> [0,h]
    // [x,y,z] form right handed coordinate system with +y pointing up, +x pointing right
    return {
      x: (p.x + 1)/2 * game.width, 
      y: (1 - (p.y + 1)/2) * game.height, 
    }
}

// project 3D to 2D
function project({x, y, z}) {
    return {
        x: x/z,
        y: y/z,
    }
}

// translate along z-axis by dz
function translate_z({x, y, z}, dz) {
    return {
        x, y, z: z + dz
    }
}

// rotate about Y axis (xz plane) by angle
function rotate_xz({x, y, z}, angle) {
    const ct = Math.cos(angle)
    const st = Math.sin(angle)
    return {
        x: x * ct - z * st,
        y,
        z: x * st + z * ct
    }
}

const FPS = 60;
let dz = 1;
let angle = 0;

const vertices = [
    {x: 0.25, y: 0.25, z: 0.25},
    {x: -0.25, y: 0.25, z: 0.25},
    {x: -0.25, y: -0.25, z: 0.25},
    {x: 0.25, y: -0.25, z: 0.25},    

    {x: 0.25, y: 0.25, z: -0.25},
    {x: -0.25, y: 0.25, z: -0.25},
    {x: -0.25, y: -0.25, z: -0.25},
    {x: 0.25, y: -0.25, z: -0.25},   
]

const faces = [
    [0,1,2,3],
    [4,5,6,7],
    [0,4],
    [1,5],
    [2,6],
    [3,7]

]

function frame() {
    const dt = 1/FPS;
    //dz += 1 * dt
    angle += Math.PI * dt;
    clear()
    //for(const v of vertices) {
    //  point(screen(project(translate_z(rotate_xz(v, angle), dz))))
    //}
    for(const f of faces) {
        for(let i = 0; i < f.length; ++i){
            const a = vertices[f[i]];
            const b = vertices[f[(i+1)%f.length]];
            line(screen(project(translate_z(rotate_xz(a, angle), dz))),
            screen(project(translate_z(rotate_xz(b, angle), dz))))
        }
    }
    setTimeout(frame, 1000/FPS)
}
setTimeout(frame, 1000/FPS)