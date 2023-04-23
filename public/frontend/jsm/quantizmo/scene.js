import * as Feature from './feature.js'
import {
    Matrix4,
    MeshBasicMaterial,
    Raycaster,
    SphereBufferGeometry,
    Vector3
} from './three.module.js'


let _scene, _renderer, _camera, _lights, _mixers, _script, controls, _container;
let clock = new THREE.Clock();
let _target = "newyork.json";
let _entities = [];
let _controls;
let _ui;
let _shell = null;
let _clickables = [];
let _collidables = [];
let _raycaster;
let _workingMatrix;
let _selected = false;
let _mode = "production";
let _ports = [];
let _files;
let _jarvis, _self;
let _updateReady = true;
let _updatables = [];

const setSize = (container, camera, renderer) => {
    camera.aspect = container.clientWidth / container.clientHeight;
    camera.updateProjectionMatrix();
  
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.setPixelRatio(window.devicePixelRatio);
};

class Resizer {
    constructor(container, camera, renderer) {

        setSize(container, camera, renderer);
    
        window.addEventListener('resize', () => {
          setSize(container, camera, renderer);
          this.onResize();
        });

    }
}

class Scene {
    constructor(container, target, test = false, isVr = true) {
        _camera = Feature.createCamera();
        _scene = Feature.createScene();
        _raycaster = new Raycaster();
        _workingMatrix = new Matrix4();
        _container = container;
        _lights = Feature.createLights();
        _renderer = Feature.createRenderer(container);
        _controls = Feature.createControls(_camera, _renderer.domElement);
        _ui = Feature.createUI();
        _jarvis = Feature.createJarvis();
        let o = {
            path: 'aquatic.jpg'
        };
        let skybox = Feature.createSkybox(o);
        _scene.add(skybox.model);
        _self = this;
        _target = target;
        // _handler = new Handler();

        for (const e of _lights) {
            _scene.add(e);
        }
        
        _camera.position.set(0, 0, 2000);

        container.appendChild(_renderer.domElement);

        const resizer = new Resizer(container, _camera, _renderer);
        resizer.onResize = () => {
            this.render();
        };
    }

    async init(cache = false, target = _target) {
    }

    debug(string) {}

    // different environments
    devMode() {}

    prodMode() {}

    gameMode() {}

    buffer() {}

    graph(data) {
        console.log("data making it to the scene!");
        _shell = Feature.createD3(data);
        _shell.mesh.position.set(0, 0, -5);
        _camera.add( _shell.mesh );
        _shell.scene = _scene;
        setTimeout(() => {_camera.remove(_shell.mesh);}, 3000);
    }

    save() {
        // let items = [];
        // items.push(_sceneInfo);
        // for (const e of _entities) {
        //     items.push(e.serialize());
        // }
        // Script.send("save", items);
    }

    add(object) {
        _updatables.push(object);
        _scene.add(object);
    }
    
    // NLP AI
    launchJarvis() {}

    killJarvis() {}

    // getters and setters
    get entities() { return _entities; }
    set entities(val) { _entities = val; }
    get UI() { return _ui; }
    set UI(val) { _ui = val; }
    get clickables() { return _clickables; }
    get scene() { return _scene; }
    get camera() { return _camera; }
    get mode() { return _mode; }
    set mode(val) { 
        _mode = val; 
        if (_mode == "development") {
            this.devMode();
        } else if (_mode == "production") {
            this.prodMode();
        }
    }
    get workingMatrix() { return _workingMatrix; }
    get raycaster() { return _raycaster; }
    get collidables() { return _collidables; }

    stock() {}

    vr(val) { /** this._vr = val; */ }

    start() {
        console.log("Starting!");
        _renderer.setAnimationLoop(() => {
          this.render();
          this.tick();
        });
    }

    get files() { return _files; }
    set files(val) { _files = val; }
      
    stop() {
        _renderer.setAnimationLoop(null);
    }

    cleanUp() {
        // Scaffolding.cleanDag();
        for (let e in _clickables) {
            _clickables[e].geometry.dispose();
            _clickables[e].material.dispose();
        }
        _clickables = [];
        for (let e in _grabbables) {
            _grabbables[e].geometry.dispose();
            _grabbables[e].material.dispose();
        }
        _grabbables = [];
        for (let e in _ports) {
            _ports[e].geometry.dispose();
            _ports[e].material.dispose();
        }
        _grabbables = [];
    }
      
    tick() {
        const delta = clock.getDelta();
        if (_updateReady) {

            // _handler.update(delta);
            // _user.update(delta);
            
            for(let e of _entities) {
                e.update(delta);
            }

            for(let e of _updatables) {
                e.update(delta);
            }
        }

        // this.vr.tick();
    }

    // changes scene
    changeScript(target) {}

    render() { _renderer.render(_scene, _camera); }

} export { Scene }


// ###################### RAYCAST HELPERS TBR

function intersectObjects( controller ) {

    // Do not highlight when already selected
    if ( _selected !== undefined ) return;

    // console.warn("******************");
    // console.warn(_controller);

    const line = controller.controller.getObjectByName( 'line' );
    const intersections = getIntersectionsx( controller.controller );

    if ( intersections.length > 0 ) {
        // console.log("here");
        // const intersection = intersections[ 0 ];
        // let pos = intersection.point;
        // // console.log(pos);
        // let mesh;
        // if (_controls.isTeleporting) {
        //     // line.material.color.set(0x00ff00);
        //     const geometry = new THREE.CylinderGeometry( 0.5, 0.5, 0.1, 48 );
        //     let mat = new THREE.MeshStandardMaterial({color: 0x00ff00, metalness: 0.5, roughness: 0.1, wireframe: true});
        //     mesh = new THREE.Mesh(geometry, mat);
        // } else {
        //     // line.material.color.set(0x0000ff);
        //     let sphere = new THREE.SphereBufferGeometry(0.05, 32, 32);
        //     let mat = new THREE.MeshBasicMaterial({color: 0x0000ff, opacity: 0.5});
        //     mesh = new THREE.Mesh(sphere, mat);
        // }
        // mesh.position.copy(pos);
        // _scene.add(mesh);
        // // console.log("here 2");
        // _intersector.push(mesh);
        // const object = intersection.object;
        // // object.material.emissive.r = 1;
        // _intersected.push( object );

        // line.scale.z = intersection.distance;

    } else {

        // line.scale.z = 5;

    }

}

function cleanIntersected() {

    // while ( _intersected.length ) {
    //     const object = _intersected.pop();
    //     // object.material.emissive.r = 0;
    //     if (_intersector.length > 0) {
    //         let toRemove = _intersector.pop();
    //         _scene.remove(toRemove);    
    //     }
    // }

}

function getIntersectionsx( controller ) {
    _workingMatrix.identity().extractRotation( controller.matrixWorld );
    _raycaster.ray.origin.setFromMatrixPosition( controller.matrixWorld );
    _raycaster.ray.direction.set( 0, 0, - 1 ).applyMatrix4( _workingMatrix );

    // console.warn("raycaster direction");
    // console.log(_raycaster.ray.origin);
    // console.log(_raycaster.ray.direction);
    // console.log(_collidables);
    let intersects;
    if (_controls.isTeleporting) {
        intersects = _raycaster.intersectObjects( _ports, false );
    } else {
        intersects = _raycaster.intersectObjects( _clickables, true );
    }

    return intersects;
}