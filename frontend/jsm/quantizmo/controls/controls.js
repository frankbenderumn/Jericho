import { OrbitControls } from 'https://unpkg.com/three@0.137.0/examples/jsm/controls/OrbitControls.js';
import { ThirdPersonControls } from './ThirdPersonControls.js';
import { TrackballControls } from 'https://unpkg.com/three@0.137.0/examples/jsm/controls/TrackballControls.js';

let _camera;
let _container;
let _controls;
let _type;


// clean memory with strategy pattern
class Controls {
    
    constructor(camera, container, type = "orbit") {
        _camera = camera;
        _container = container;
        if (type == "orbit") {
            _controls = new OrbitControls(camera, container);
        }
        _type = type;
    }

    get controls() { return _controls; }
    set controls(val) { _controls = val; }

    update(dt) {
        if (_type != "orbit") {
            _controls.update(dt);            
        }
        _controls.update();
    }

    focus(target) {
        if (_type == "third") {
            _controls.focus = target;
        }
    }

    setType(name) {
        switch(name) {
            case "orbit":
                // _controls = new OrbitControls(_camera, _container);
                break;
            // case "first":
            //     _controls = new FirstPersonControls(camera, container);
            //     break;
            case "third":
                _controls = new ThirdPersonControls(_camera, _container);
                break;
            case "trackball":
                // _controls = new TrackballControls(camera, container);
                break;
            case "first":
                // _controls = new FirstPersonControls(camera, container);
                break;
            default:
                alert("ERROR: No controls found!");
                break;
        }
        _type = name;
    }

} export { Controls }