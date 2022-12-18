import { Scene,
    Color,
    PerspectiveCamera,
    WebGLRenderer,
    AmbientLight,
    DirectionalLight,
    BoxBufferGeometry,
    MeshBasicMaterial,
    MeshLambertMaterial,
    MeshStandardMaterial,
    Mesh,
    Clock,
    Float32BufferAttribute,
    BufferGeometry,
    Object3D,
    DoubleSide,
    VertexColors,
    sRGBEncoding,
    MeshPhongMaterial,
    WireframeGeometry,
    SphereGeometry,
    TextureLoader,
    Vector3,
    EdgesGeometry,
    LineSegments,
    LineBasicMaterial,
    Line,
    AdditiveBlending,
    GridHelper
        } from '../three.module.js';
import * as Loader from './loader.js';
import { Controls } from './controls/controls.js';
import { VRButton } from 'https://unpkg.com/three@0.138.0/examples/jsm/webxr/VRButton.js';
import { CanvasUI } from './ui/canvas.js';
import { TetrahedronGeometry } from '../three.module.js';

export function createCamera() {
  // const camera = new PerspectiveCamera( 55, window.innerWidth / window.innerHeight, 1, 1000 );
  // camera.position.set( -10, 10, 10 );//30, 30, 100 );
  const camera = new PerspectiveCamera(
    35, // fov = Field Of View
    1, // aspect ratio (dummy value)
    0.1, // near clipping plane
    50000, // far clipping plane
  );

  // move the camera back so we can view the scene
  camera.position.set(0, 0, 10);
  return camera;
}

export async function createModelAsync(path) {
  let m = await Loader.load(path);
  return m;
}

export function createModel(path) {
  let m = Loader.load(path);
  return m;
}

export function createUI(content = "") {
    const config = {
      header:{
          type: "text",
          position:{ top:0 },
          paddingTop: 30,
          height: 70
      },
      main:{
          type: "text",
          position:{ top:70 },
          height: 372, // default height is 512 so this is 512 - header height:70 - footer height:70
          backgroundColor: "#bbb",
          fontColor: "#000"
      },
      footer:{
          type: "text",
          position:{ bottom:0 },
          paddingTop: 30,
          height: 70
      },
      panelSize: { width: 3, height: 3 } 
  }
  if (content == "") {
    content = {
        header: "Uncharted Territory Research Project",
        main: "Welcome to stocket!",
        footer: "Footer"
    }
  }
  return new CanvasUI( content, config );
}

export function createShell(content = "") {
  const config = {
    header:{
        type: "text",
        position:{ top:0 },
        paddingTop: 30,
        height: 70
    },
    main:{
        type: "text",
        position:{ top:70 },
        height: 372, // default height is 512 so this is 512 - header height:70 - footer height:70
        backgroundColor: "#bbb",
        fontColor: "#000"
    },
    footer:{
        type: "text",
        position:{ bottom:0 },
        paddingTop: 30,
        height: 70
    },
    panelSize: { width: 3, height: 3 } 
}
if (content == "") {
  content = {
      header: "Uncharted Territory Research Project",
      main: "Welcome to stocket!",
      footer: "Footer"
  }
}
return new CanvasUI( content, config );
}

export function createD3(data, renderer) {
    const config = {
      header:{
          type: "text",
          position:{ top:0 },
          paddingTop: 30,
          height: 70
      },
      main:{
          type: "text",
          position:{ top:70 },
          height: 372, // default height is 512 so this is 512 - header height:70 - footer height:70
          backgroundColor: "#bbb",
          fontColor: "#000"
      },
      footer:{
          type: "text",
          position:{ bottom:0 },
          paddingTop: 30,
          height: 70
      },
      panelSize: { width: 3, height: 3 } 
  }
  const content = {
      header: data.companyName + "",
      main: data.iexOpen + "",
      footer: data.high + ""
  }

  // let uiElement = document.getElementById('dag');
  // navigator.xr.requestSession('immersive-ar', {
  //     optionalFeatures: ['dom-overlay'],
  //     domOverlay: { root: uiElement } }).then((session) => {
  //     // session.domOverlayState.type is now set if supported,
  //     // or is null if the feature is not supported.
  //   }
  // }
  let ui = new CanvasUI( content, config );
  // ui.initControllers(renderer);

  return ui;
}

export function createSocket() {
  return new WSApi();
}

export function createLights() {
  const lights = [];
  const ambient = new AmbientLight( 0xffffff, 1.0);
  // scene.add( ambientLight );
  const dir1 = new DirectionalLight( 0xffffff, 1.0 );
  dir1.position.set( 10, 10, 10 );
  // scene.add( ambientLight, light );
  const dir2 = new DirectionalLight( 0xffffff, 1.0 );
  dir2.position.set( 0, 10, -10 );
  lights.push(ambient);
  lights.push(dir1);
  lights.push(dir2);
  return lights;
  // scene.add( ambientLight, light2 );
}

export function createScene() {
  let scene = new Scene();
  scene.background = new Color('black');
  return scene;
}

export function createCube() {
  // create a geometry
  const geometry = new BoxBufferGeometry(1, 1, 1);

  // create a default (white) Basic material
  const material = new MeshBasicMaterial();

  // create a Mesh containing the geometry and material
  const cube = new Mesh(geometry, material);

  cube.position.x = 0;

  cube.update = (dt) => {
    cube.rotation.x += 2 * dt;
  }

  return cube;
}

export function createDevGrid() {
  let elements = [];
  const size = 50000;
  const divisions = 50000;

  const gridHelper = new GridHelper( size, divisions );
  // scene.add( gridHelper );
  elements.push(gridHelper);

  const xGeo = new BufferGeometry();
  xGeo.setAttribute( 'position', new Float32BufferAttribute( [ 50000, 0, 0, -50000, 0, 0 ], 3 ) );
  let xMat = new LineBasicMaterial( { color: 0xff0000, blending: AdditiveBlending, lineWidth: 5 } );
  const xAxis = new Line( xGeo, xMat );
  elements.push(xAxis);

  const yGeo = new BufferGeometry();
  yGeo.setAttribute( 'position', new Float32BufferAttribute( [ 0, 50000, 0, 0, -500000, 0 ], 3 ) );
  let yMat = new LineBasicMaterial( { color: 0x0000ff, blending: AdditiveBlending, lineWidth: 5 } );
  const yAxis = new Line( yGeo, yMat );
  elements.push(yAxis);

  const zGeo = new BufferGeometry();
  zGeo.setAttribute( 'position', new Float32BufferAttribute( [ 0, 0, 50000, 0, 0, -50000 ], 3 ) );
  let zMat = new LineBasicMaterial( { color: 0x00ff00, blending: AdditiveBlending, lineWidth: 5 } );
  const zAxis = new Line( zGeo, zMat );
  elements.push(zAxis);

  return elements;
}

export function createJarvis() {
  let group = new Object3D();
  let num = 4;
  for(let i = -1 * num; i < num + 1; i++) {
    for(let j = -1 * num; j < num + 1; j++) {
      for(let k = -1 * num; k < num + 1; k++) {
        let geometry = new BoxBufferGeometry(0.1, 0.1, 0.1);
        let material = new MeshStandardMaterial({color: 0x007777, roughness: 0.7});
        let cube = new Mesh(geometry, material);
        cube.position.x = j * 0.3 + 0.1;
        cube.position.y = i * 0.3 + 0.1;
        cube.position.z = k * 0.3 + 0.1;
        group.add(cube);
      }
    }
  }

  let counter = 0;

  group.update = (dt) => {
    group.rotation.x += 2 * dt;
    group.rotation.y += 1 * dt;
    let ct = 1;
    let nums = num * num * num;
    let nums2 = num * num;
    for (let e of group.children) {
      e.rotation.z += group.rotation.x * (ct / nums) * dt;
      e.rotation.y += 1 * (ct / nums) * dt;
      ct++;
      e.position.x += 0.05 * Math.cos(0.5 * counter);
      e.position.z += 0.05 * Math.sin(0.5 * counter);
      counter++;
    }
  }

  return group;
}

export function createGrid() {

  // Initialise threejs geometry
  var geometry = new BufferGeometry(); 

  const indices = [];
  const vertices = [];
  const normals = [];
  const colors = [];

  let cells = 20;
  let width = cells + 1;
  let height = cells + 1;
      
  let scale = 10;
  for (let i = 0; i < width; i++) {
    for (let j = 0; j < height; j++) {
      let f1 = new Vector3(-cells*10/2, 0, -cells*10/2);
      vertices.push(f1.x + (scale * j), f1.y, f1.z + (scale * i));
    }
  }

  // Add cell faces (2 traingles per cell) to geometry
  for (var i = 0; i < cells; i++){
      for (var j = 0; j < cells; j++){ 
        const n0 = j + (i * width);
        const n1 = j + (i * width) + 1;
        const n2 = j + (i * width) + width + 1;
        const n3 = j + (i * width) + width;

        let c1 = 0.2;
        let c2 = 0.2;
        let c3 = 0.2;

        let c4 = 0.2;
        let c5 = 0.2;
        let c6 = 0.2;

        normals.push( 0, 1, 0 );
        normals.push( 0, 1, 0 );

        colors.push(c1, c2, c3);
        colors.push(c4, c5, c6);

        indices.push(n0,n1,n2);
        indices.push(n2,n3,n0);
      }
    }

    for (let j = 7; j < vertices.length; j += (cells * 3 + 3)) {
      // left mountain
      vertices[j - 3] += 6;
      vertices[j] += 10;
      vertices[j + 3] += 6;

      // right mountain
      vertices[j + cells * 3 - 15] += 6;
      vertices[j + cells * 3 - 12] += 10;
      vertices[j + cells * 3 - 9] += 6;
    }


  geometry.setIndex( indices );
  geometry.setAttribute( 'position', new Float32BufferAttribute( vertices, 3 ) );
  // geometry.setAttribute( 'normal', new Float32BufferAttribute( normals, 3 ) );
  // geometry.setAttribute( 'color', new Float32BufferAttribute( colors, 3 ) );

  // Use MeshPhongMaterial for a reflective surface
  var material = new MeshPhongMaterial( {
      side: DoubleSide,
      color: 0x000,
      // vertexColors: true,
      specular: 0x050505,
      shininess: 100.,
      // emissive: 0x00bbbb,
  });

  let mesh = new Mesh( geometry, material );

  // testing geometry shader
  // mesh.update = (dt) => {
  //   for (let j = 7; j < vertices.length; j += (cells * 3 + 3)) {
  //     decay(_amp, dt);
  //     // left mountain
  //     vertices[j - 3] -= _amp / 2;
  //     vertices[j] -= _amp;
  //     vertices[j + 3] -= _amp / 2;
  
  //     // right mountain
  //     vertices[j + cells * 3 - 15] -= _amp / 2;
  //     vertices[j + cells * 3 - 12] -= _amp;
  //     vertices[j + cells * 3 - 9] -= _amp / 2;
  //   }

  //   geometry.setIndex( indices );
  //   geometry.setAttribute( 'position', new Float32BufferAttribute( vertices, 3 ) );

  //   this.scene = new Mesh(geometry, material);
  // }

  console.warn(mesh);
  const edges = new WireframeGeometry( geometry );
  const line = new LineSegments( edges, new LineBasicMaterial( { color: 0x00ffff, lineWidth: 10 } ) );

  return [mesh, line];
}

let _amp = 10;

function decay(amp, dt) {
  amp -= 0.5 * dt;
}

export function createStars() {
  // const geometry = new THREE.SphereGeometry( 1000, 60, 40 );
  // geometry.scale( - 1, 1, 1 );

  // const texture = new THREE.TextureLoader().load( `../assets/texture/hdr/${bg}` );
  // const material = new THREE.MeshBasicMaterial( { map: texture } );

  // const mesh = new THREE.Mesh( geometry, material );
  // scene.add( mesh );
}

export function createSkybox(params, entity = true, radius = 35000) {



  const geometry = new SphereGeometry( radius, 60, 40 );
  geometry.scale( - 1, 1, 1 );

  const texture = new TextureLoader().load( `../assets/texture/hdr/${params.path}` );
  const material = new MeshBasicMaterial( { map: texture } );

  const mesh = new Mesh( geometry, material );
  // scene.add( mesh );
  let o;
  if (entity) {
    o = {
      entityId: params.entityId,
      name: params.name,
      model: mesh,
      type: params.type,
      dynamic: false,
      path: params.path
    };
  } else {
    o = mesh;
  }

  return o;
}

export function createRenderer(container, vr = false, start = undefined, end = undefined) {
  if (!vr) {
    const renderer = new WebGLRenderer( {container, alpha: true});
    return renderer;
  } else {
    const renderer = new WebGLRenderer( { alpha: true, antialias: true } );
    renderer.setPixelRatio( window.devicePixelRatio );
    renderer.setSize( window.innerWidth, window.innerHeight );
    renderer.outputEncoding = sRGBEncoding;
    renderer.xr.enabled = true;
    renderer.autoClear = true;
    document.body.appendChild( VRButton.createButton( renderer ) );
    return renderer;
  }
}

export function createControls(camera, canvas) {
  const controls = new Controls(camera, canvas);

  // damping and auto rotation require
  // the controls to be updated each frame

  // this.controls.autoRotate = true;
  // controls.enableDamping = true;

  // controls.tick = () => controls.update();

  return controls;
}