import * as THREE from '../three.module.js';
// import { Debug } from "../debug.js"

class ThirdPersonControls {
    constructor(camera, container) {
        this._container = container;
        this._camera = camera;
        this._time = 0;
        this._theta = 2;
        this._phi = 2;
        this._turnSpeed = 1;
        this._speed = 0.001;
        this._radius = 10;
        this._focus = new THREE.Vector3(0, 0, 0);
        this._posMove = [0, 0, 0];
        this._negMove = [0, 0, 0];
        this._posTurn = [0, 0];
        this._negTurn = [0, 0];
        this._dynamic = true;
        this._drag = false;
        this._forwardDir = new THREE.Vector3(0, 0, 1);
        this._upDir = new THREE.Vector3(0, 1, 0);
        this._rightDir = new THREE.Vector3(1, 0, 0);
        this._jump = false;
        this._jumpTime = 2;
    }

    // get target() { return this._focus; }
    get turnSpeed() { return this._turnSpeed; }
    get speed() { return this._speed; }
    get dynamic() { return this._dynamic; }
    get drag() { return this._drag; }

    target(val) { this._focus = val; }
    set turnSpeed(val) { this._turnSpeed = val; }
    set speed(val) { this._speed = val; }
    set dynamic(val) { this._dynamic = val; }
    set drag(val) { this._drag = val; }
    get focus() { return this._focus; }
    set focus(val) { this._focus = val; }

    move(axis, val) {
        (val > 0) ? this._posMove[axis] = val : this._negMove[axis] = val;
    }

    clearMove(axis, val) {
        (val > 0) ? this._posMove[axis] = 0 : this._negMove[axis] = 0;
    }

    turn(axis, val) {
        (val > 0) ? this._posTurn[axis] = val : this._negTurn[axis] = val;
    }

    clearTurn(axis, val) {
        (val > 0) ? this._posTurn[axis] = 0 : this._negTurn[axis] = 0;
    }

    jump(dt) {
        if (!this._jump) {
            this._jump = true;
        }

        if (this._jump) {
            if (this._jumpTime > 0) {
                this._jumpTime -= dt;
            }

            if (this._jumpTime <= 0) {
                this._jumpTime = 0;
                this._jump = false;
            }
        }
    }

    // this is getting called four times
    update(dt = 0.0125) {
        // Debug.log(this._posTurn, "info");
        // Debug.log("FOCUS IS: ", "info");
        // console.log(this._focus);
        // console.log(this._posMove);
        // console.log(this._negMove);
        // console.log(this._posTurn);
        // console.log(this._negTurn);
        this._theta += this._turnSpeed * (this._posTurn[0] + this._negTurn[0]) * dt;
        this._phi += this._turnSpeed * (this._posTurn[1] + this._negTurn[1]) * dt;
        let maxAng = 89 * Math.PI / 180;
        this._phi = Math.min( maxAng, Math.max( 0.1 , this._phi ));
        let depthX = (1 - Math.cos(this._theta) * this._radius);

        let sinPhi = Math.sin(this._phi);
        let cosPhi = Math.cos(this._phi);
        let cosTheta = Math.cos(this._theta);
        let sinTheta = Math.sin(this._theta);

        let camPos = new THREE.Vector3(this._focus.parent.position.x + sinTheta*sinPhi*this._radius, this._focus.parent.position.y + cosPhi*this._radius, this._focus.parent.position.z - sinPhi*cosTheta*this._radius);
        // console.log("camPos is: ");
        // console.warn(camPos);
        let forwardDir = new THREE.Vector3(sinTheta *this._radius, sinPhi *this._radius,  depthX).normalize();
        let upDir = new THREE.Vector3(0, 1, 0);
        let rightDir = new THREE.Vector3(forwardDir.x, forwardDir.y, forwardDir.z);
        rightDir.cross(upDir).negate();

        let x = this._focus.parent.position.x;
        let y = this._focus.parent.position.y;
        let z = this._focus.parent.position.z;

        // console.warn("x is: "+x);
        // console.warn("y is: "+y);
        // console.warn("z is: "+z);

            let normPosDir = new THREE.Vector3(this._posMove[0], this._posMove[1], this._posMove[2]);
            let normNegDir = new THREE.Vector3(this._negMove[0], this._negMove[1], this._negMove[2]);

            normPosDir.normalize();
            normNegDir.normalize();

            this._posMove[0] = normPosDir.x;
            this._posMove[1] = normPosDir.y;
            this._posMove[2] = normPosDir.z;

            this._negMove[0] = normNegDir.x;
            this._negMove[1] = normNegDir.y;
            this._negMove[2] = normNegDir.z;

        // if (this._dynamic) {

            x += 0.01 * (this._posMove[0] + this._negMove[0]) * rightDir.x;
            y += 0.01 * (this._posMove[0] + this._negMove[0]) * 0;
            z += 0.01 * (this._posMove[0] + this._negMove[0]) * rightDir.z;
            
            x += 0.01 * (this._posMove[1] + this._negMove[1]) * upDir.x;
            y += 0.01 * (this._posMove[1] + this._negMove[1]) * 0;
            z += 0.01 * (this._posMove[1] + this._negMove[1]) * upDir.z;

            x += 0.01 * (this._posMove[2] + this._negMove[2]) * forwardDir.x;
            y += 0.01 * (this._posMove[2] + this._negMove[2]) * 0;
            z += 0.01 * (this._posMove[2] + this._negMove[2]) * forwardDir.z;

        // }
        // x += (this._posMove[0] + this._negMove[0]) * 0.6 * dt;
        // z += (this._posMove[2] + this._negMove[2]) * 0.6 * dt;
        // y = 0;

        // console.warn("====== POST ======");
        // console.warn("x is: "+x);
        // console.warn("y is: "+y);
        // console.warn("z is: "+z);


        this._focus.parent.rotation.y = -this._theta;

        this._focus.parent.position.set(x, y, z);

        if (this._jump) {
            this._focus.parent.position.y += 0.02 * dt;
        } else {
            this._focus.parent.position.y -= 0.02 * dt;
            if (this._focus.parent.position.y <= 0) {
                this._focus.parent.position.y = 0;
            }
        }
        // console.log("FOCUS POSITION IS: ");
        // console.log(this._focus);
        // console.error(this._focus.parent.position);
        this._camera.position.set(camPos.x, camPos.y, camPos.z);
        // console.error(this._camera.position);
        this._camera.lookAt(this._focus.parent.position);
    }

} export { ThirdPersonControls }