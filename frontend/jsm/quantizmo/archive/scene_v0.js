import * as Feature from '../feature.js'
// import * as Script from './script.js'
import * as THREE from '../three.module.js'
// import { Entity } from './entity.js'
// import * as XRController from './xr/xrcontroller.js'
// import { XRHandler } from './xr/xrhandler.js'
// import * as Analyser from './audio/analyser.js'
// import * as Scaffolding from './scaffolding.js'
// import * as System from './system.js'
// import { Rig } from './xr/rig.js'
// import { Hand } from "./xr/hand.js";
// import * as Component from "./component.js"
// import { Handler } from "./handler/handler.js"
// import { Gamepad } from "./gamepad.js"
// import { Matrix4, MeshBasicMaterial, Raycaster, SphereBufferGeometry, Vector3 } from '../three.module.js'
// import { XRControllerModelFactory } from 'https://unpkg.com/three@0.138.0/examples/jsm/webxr/XRControllerModelFactory.js';
// import { User } from "./user.js"
// import { Debug } from './debug.js'
// import { XR } from "./xr/xr.js"


// import { WSApi } from './system/socket.js'

let _scene, _renderer, _camera, _lights, _mixers, _script, controls;
let updatables = [];
let clock = new THREE.Clock();
let _scriptsDir = "./js/scenes/";
let _assetsDir = "../assets/models/";
let _song = "../assets/audio/heist.wav";
let _target = "newyork.json";
let _updateReady = false;
let _entities = [];
let _controller;
let _dolly;
let _dummyCam;
let _container;
let _controls;
let _ui;
let _uniforms;
let _socket;
let _shell = null;
let _rig;
let _jarvis;
let _jarvisActive = false;
let _skybox;
let _clickables = [];
let _collidables = [];
let _sceneInfo;
let _menuController;
let _vr = false;
let _raycaster;
let _workingMatrix;
let _controllers;
let _intersected = [];
let _clean;
let _selected = false;
let _collections = [];
let _intersector = [];
let _mode = "production";
let _tronDiscs = [];
let _shield;
let _ironManHands = [];
let _grabbables = [];
let _devObjs = [];
let _regModelLeft;
let _regModelRight;
let _loaders = [];
let _ports = [];
let _dofMenu;
let _files;
let _fileSystem;
let _self;
let _gamepad;
let _user;
let _handler;

const setSize = (container, camera, renderer) => {
    camera.aspect = container.clientWidth / container.clientHeight;
    camera.updateProjectionMatrix();
  
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.setPixelRatio(window.devicePixelRatio);
};

class Cache {
    constructor() {
        this._session = undefined;
        this._cache;
    }

    store(target, scene) {
        let models = [];
        for (let e of scene.children) {
            models.push(e);
        }
        this._cache[target] = models;
    }

    isCached(target) {
        return (this._cache[target] != undefined);
    }

    getCache(target) {
        if(this.isCached(target)) {
            return this._cache[target];
        }
    }

    loadCache(target, scene) {
        for (let e in this._cache[target]) {
            scene.add(e);
        }
    }
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
        _self = this;
        _target = target;
        _handler = new Handler();

        for (const e of _lights) {
            _scene.add(e);
        }
        
        // loop = new Loop(camera, scene, renderer);
        container.appendChild(_renderer.domElement);

        // if (_mode != "production") {
        //     this.devMode();

        //     const cube = Feature.createCube();
        //     // controls.controls.target.copy(cube.position);
        //     // updatables.push(cube);
        //     updatables.push(controls);
        //     _grabbables.push(cube);
        //     _scene.add(cube);
        // }

        const resizer = new Resizer(container, _camera, _renderer);
        resizer.onResize = () => {
            this.render();
        };
    }

    async init(cache = false, target = _target) {
        // await Script.run(`${_scriptsDir}${target}`, _assetsDir).then(function(data) {  
        //     Promise.all(data).then(function(entities) {
        //         $("#loading-background").hide();
        //         _entities = [];
        //         let arr = target.split(".");
        //         for (let e of entities[0]) {
        //             let d = new Entity(e);
        //             console.log(d);
        //             _entities.push(d);
        //             // _cache.push(d);
        //             if (d.type !== "skybox" && d.type != "undefined") {
        //                 // _clickables.push(d.model);
        //                 let id = 0;
        //                 for (let mesh of d.meshes) {
        //                     if (d.interact === 1 || d.interact === 3) {
        //                         _clickables.push(mesh);
        //                     }

        //                     if (d.interact === 2 || d.interact === 3) {
        //                         _grabbables.push(mesh);
        //                     }

        //                     if (d.interact === 0) {
        //                         _ports.push(mesh);
        //                     }

        //                     id = mesh.entityId;
        //                 }
        //                 _collections[id] = d.meshes;

        //                 if (d.type == "user") {
        //                     Debug.log("user being added", "success");
        //                     console.log(d.model);
        //                     console.log(d.model.children);
        //                     for (let mesh of d.model.children) {
        //                         console.error(mesh);
        //                         if (mesh.name != "Camera" && mesh.name != "Light") {
        //                             mesh.position.copy(new Vector3(0, 0, 0));
        //                             _user = new User(mesh, _scene, _renderer, _camera, _controls, _handler, d);
        //                             break;
        //                         }
        //                     }
        //                     _handler.user = _user;
        //                     // _huser = _user;
        //                 }
        //                 // walkDown(d.model);
        //             }
        //             if (d.name.includes("tron")) {
        //                 let i;
        //                 (d.hand != "left") ? i = 0 : i = 1;
        //                 _tronDiscs.push(d.model);
        //             }

        //             if (d.name.includes("iron")) {
        //                 let i = 0;
        //                 (d.hand != "left") ? i = 0 : i = 1;
        //                 _ironManHands.push(d.model);
        //             }

        //             if (d.name.includes("shield")) {
        //                 _shield = d.model;
        //             }

        //             if (d.type == "collider") {
        //                 d.model.visible = false;
        //             }

        //             _scene.add(d.model);
        //             if(d.name == "vr-menu") {
        //                 _dofMenu = d.model;
        //                 d.model.visible = false;
        //                 _menuController.controller.add(d.model);
        //                 // d.model.position.copy(_menuController.grip.position);
        //                 // console.log("DOF-MENU");
        //                 // console.warn(d.model);
        //                 d.update = function() {
        //                     // let c = _menuController.grip.position;
        //                     // d.model.position.set(0, 0, 0);
        //                 };
        //             }
        //         }
        //         _sceneInfo = entities[1];
        //         _controls.scene = _scene;
        //         _updateReady = true;
        //         while (_loaders.length > 0) {
        //             let el = _loaders.pop();
        //             _scene.remove(el);
        //             el.geometry.dispose();
        //             el.material.dispose();
        //         }
        //         _controls.collidables = _clickables;
        //         // _controls.grabbables = _grabbables;
        //         _collidables = _clickables;
        //         let params = {name: "rando"};
        //         Script.send("setup", params, _self).then(function(response){
        //             Scaffolding.fileTree(_files, _self);
        //         });
        //         Scaffolding.dag(_entities);
        //         Scaffolding.lighting(_lights);
        //         Scaffolding.analytics(_entities, _collections, _collidables, _clickables, _grabbables);
        //         Scaffolding.entities(_self);
        //     });
        // });
    }

    // VR Controller changing code, needs to be refactored

    // suitUp() {
    //     _regModelLeft = _controllers[0].grip.children.pop();
    //     _regModelRight = _controllers[1].grip.children.pop();
    //     // _regModelRight = _controllers[1].grip.children.pop();
    //     // _scene.remove(_dofMenu);
    //     // _lineLeft = _controllers[0].controller.line.
    //     _ironManHands[0].position.set(0, 0, 0);
    //     _ironManHands[1].position.set(0, 0, 0);
    //     _ironManHands[0].quaternion.copy(_controllers[0].controller.quaternion);
    //     _ironManHands[1].quaternion.copy(_controllers[1].controller.quaternion);
    //     _controllers[0].controller.add(_ironManHands[0]);
    //     _controllers[1].controller.add(_ironManHands[1]);
    //     _controls.mode = "iron-man";
    // }

    // shutDown() {
    //     _scene.add(_dofMenu);
    //     _regModelLeft.position.set(0, 0, 0);
    //     _regModelRight.position.set(0, 0, 0);
    //     for (let e in _controllers[0].grip.children) {
    //         _controllers[0].grip.children[e].remove();
    //     }
    //     for (let e in _controllers[1].grip.children) {
    //         _controllers[1].grip.children[e].remove();
    //     }
    //     _ironManHands[0] = _controllers[0].grip.remove(_ironManHands[0]);
    //     _ironManHands[1] = _controllers[1].grip.remove(_ironManHands[1]);
    //     _regModelLeft.quaternion.copy(_controllers[0].controller.quaternion);
    //     _regModelRight.quaternion.copy(_controllers[1].controller.quaternion);
    //     _controllers[0].controller.add(_regModelLeft);
    //     _controllers[1].controller.add(_regModelRight);
    //     _controls.mode = "standard";
    // }

    // tron() {
    //     _tronDiscs[0].position.set(0, 0, 0);
    //     _tronDiscs[1].position.set(0, 0, 0);
    //     _tronDiscs[0].quaternion.copy(_controllers[0].controller.quaternion);
    //     _tronDiscs[1].quaternion.copy(_controllers[1].controller.quaternion);
    //     let _leftShield = _shield.clone();
    //     _shield.position.set(0, 0, 0);
    //     _leftShield.position.set(0, 0, 0);
    //     _shield.quaternion.copy(_controllers[0].controller.quaternion);
    //     _leftShield.quaternion.copy(_controllers[1].controller.quaternion);
    //     _controllers[0].controller.add(_leftShield);
    //     _controllers[1].controller.add(_shield);
    //     _controllers[0].controller.add(_regModelLeft);
    //     _controllers[1].controller.add(_regModelRight);
    //     _controls.mode = "tron";
    // }

    debug(string) {
        // let content = {
        //     header: "Debugger",
        //     main: string,
        //     footer: "Footer"
        // }
        // let ui = Feature.createShell(content);
        // ui.mesh.position.set( 0, 0, -2 );
        // _camera.attach( ui.mesh );
        // ui.scene = _scene;
    }

    // different environments

    devMode() {
        // if (_devObjs.length == 0) {
        //     let els = Feature.createDevGrid();
        //     for (let e in els) {
        //         _devObjs.push(els[e]);
        //         _scene.add(els[e]);
        //     }
        // }
        // _handler.setType("development");
    }

    prodMode() {
        // if (_devObjs.length > 0) {
        //     let els = Feature.createDevGrid();
        //     for (let e in els) {
        //         let toRemove = _devObjs.pop();
        //         toRemove.geometry.dispose();
        //         toRemove.material.dispose();
        //         _scene.remove(toRemove);
        //     }
        // }
        // _user.third();
        // _handler.user = _user;
        // _handler.controls = _user.controls;
        // _handler.focus();
        // _handler.setType("gamepad");
    }

    gameMode() {
        // if (_devObjs.length > 0) {
        //     let els = Feature.createDevGrid();
        //     for (let e in els) {
        //         let toRemove = _devObjs.pop();
        //         toRemove.geometry.dispose();
        //         toRemove.material.dispose();
        //         _scene.remove(toRemove);
        //     }
        // }
        // _user.third();
        // _handler.setType("gamepad");
    }

    buffer() {

    }

    graph(data) {
        console.log("data making it to the scene!");
        _shell = Feature.createD3(data);
        _shell.mesh.position.set(0, 0, -5);
        _camera.add( _shell.mesh );
        // _collections["stock-chart"] = _shell.mesh;
        _shell.scene = _scene;
        setTimeout(() => {_camera.remove(_shell.mesh);}, 3000);
    }

    save() {
        // let items = [];
        // items.push(_sceneInfo);
        // for (const e of _entities) {
        //     items.push(e.serialize());
        // }
        // console.log("#############");
        // console.log(items);
        // Script.send("save", items);
    }

    
    launchJarvis() {
        // if (!_jarvisActive) {
        //     if (this._vr) {
        //         _jarvis.position.set(0, 1, -7);
        //         _camera.add(_jarvis);
        //     } else {
        //         _scene.add(_jarvis);
        //     }
        //     updatables.push(_jarvis);
        //     _jarvisActive = true;
        // }
    }

    killJarvis() {
        // updatables = updatables.filter(item => item !== _jarvis);
        // if (this._vr) {
        //     _camera.remove(_jarvis);
        // } else {
        //     _scene.remove(_jarvis);
        // }
        // _jarvisActive = false;
    }

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
        // _mode = val; 
        // if (_mode == "development") {
        //     this.devMode();
        // } else if (_mode == "production") {
        //     this.prodMode();
        // }
    }
    get workingMatrix() { return _workingMatrix; }
    get raycaster() { return _raycaster; }
    get collidables() { return _collidables; }

    stock() {
        // let o = {
        //     ticker: "AAPL",
        //     data: "20211201"
        // };
        // Script.send("stock", o);
    }

    vr(val) {
        // this._vr = val;
    }

    start() {
        // _renderer.setAnimationLoop(() => {
        //   this.render();
        //   this.tick();
        // });
    }

    get files() { return _files; }
    set files(val) { _files = val; }
      
    stop() {
        _renderer.setAnimationLoop(null);
    }

    cleanUp() {
        // Scaffolding.cleanDag();
        // for (let e in _clickables) {
        //     _clickables[e].geometry.dispose();
        //     _clickables[e].material.dispose();
        // }
        // _clickables = [];
        // for (let e in _grabbables) {
        //     _grabbables[e].geometry.dispose();
        //     _grabbables[e].material.dispose();
        // }
        // _grabbables = [];
        // for (let e in _ports) {
        //     _ports[e].geometry.dispose();
        //     _ports[e].material.dispose();
        // }
        // _grabbables = [];
    }
      
    tick() {
        // const delta = clock.getDelta();
        // if (_updateReady) {

        //     _handler.update(delta);
        //     _user.update(delta);
            
        //     for(let e of _entities) {
        //         e.update(delta);
        //     }

        //     for(let e of updatables) {
        //         e.update(delta);
        //     }
        // }


        // if (_vr) {
        //     if (_controls) {
        //         // console.log("&&&&&&&&&&&&&");
        //         // console.log(_controllers[0]);
        //         _controls.update(delta);
        //         cleanIntersected();
        //         intersectObjects(_controllers[0]);
        //         intersectObjects(_controllers[1]);
        //         _ui.update();
        //     }

        //     _rig.update();
        // }
    }

    changeScript(target) {
        // this.cleanUp();
        // _target = target;
        // for (let e of _entities) {
        //     _scene.remove(e.model);
        // }

        // loading screen
        // ------------------------------
        // let params = {path: "glitch.jpeg"};
        // let loader = Feature.createSkybox(params, false, 1000);
        // loader.position.copy(_dolly.position);
        // loader.update = (dt) => {
        //     console.log(this);
        //     loader.rotation.y += 0.1 * dt;
        // }
        // _loaders.push(loader);
        // _scene.add(loader);
        // updatables.push(loader);

        // this.init();
    }

    render() {
        _renderer.render(_scene, _camera);
    }

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