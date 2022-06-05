import { Scene } from './scene.js';

let world;
let container = document.getElementById('scene-container');

export function main() {
    // system = new System();
    world = new Scene(container, "banjo.json", false);
    // system.scene = world;
    // await world.init().then(function(){
    //     raycast = new Raycast(world, world.camera, world.clickables);
    // });
    world.start();
}

export function add(object) {
    world.add(object);
}