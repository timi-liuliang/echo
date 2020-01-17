import THREE from "../libs/three/index"

const vertexShader = `
    attribute vec3 center;
    varying vec3 vCenter;

    void main() {
        vCenter = center;
        gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
    }
`;

const fragmentShader = `
    varying vec3 vCenter;

    float edgeFactorTri() {
        vec3 d = fwidth( vCenter.xyz );
        vec3 a3 = smoothstep( vec3( 0.0 ), d * 1.5, vCenter.xyz );
        return min( min( a3.x, a3.y ), a3.z );
    }

    void main() {
        gl_FragColor.rgb = mix( vec3( 1.0 ), vec3( 0.2 ), edgeFactorTri() );
        gl_FragColor.a = 1.0;
    }
`;

export class app {
    constructor()
    {
        this.init();
        this.animate();
    }

    init() 
    {
        const size = 150;

        this.camera = new THREE.PerspectiveCamera( 40, window.innerWidth / window.innerHeight, 1, 2000 );
        this.camera.position.z = 800;

        this.scene = new THREE.Scene();

        //left box
        let geometry = new THREE.BoxBufferGeometry( size, size, size );
        let material = new THREE.MeshBasicMaterial( { wireframe: true } );

        let mesh = new THREE.Mesh( geometry, material );
        mesh.position.x = 0;
        mesh.position.y = -150;
        this.scene.add( mesh );

        //left box ball
        geometry = new THREE.SphereBufferGeometry( size / 2, 32, 16 );
        geometry = geometry.toNonIndexed();

        this.setupAttributes( geometry );

        material = new THREE.ShaderMaterial( {
            uniforms: {},
            vertexShader,
            fragmentShader,
        } );

        material.extensions.derivatives = true;

        mesh = new THREE.Mesh( geometry, material );
        mesh.position.x = 0;
        mesh.position.y = -150;
        this.scene.add( mesh );

        //right box
        geometry = new THREE.BoxBufferGeometry( size, size, size );
        geometry = geometry.toNonIndexed();

        this.setupAttributes( geometry );

        material = new THREE.ShaderMaterial( {
            uniforms: {},
            vertexShader,
            fragmentShader,
        } );

        material.extensions.derivatives = true;

        mesh = new THREE.Mesh( geometry, material );
        mesh.position.x = 0;
        mesh.position.y = 100;
        this.scene.add( mesh );

        // renderer
        this.renderer = new THREE.WebGLRenderer( { antialias: true, canvas: canvas } );
        this.renderer.setPixelRatio( window.devicePixelRatio );
        this.renderer.setSize( window.innerWidth, window.innerHeight );
    }

    setupAttributes( geometry ) 
    {
        // TODO: Bring back quads
        const vectors = [
            new THREE.Vector3( 1, 0, 0 ),
            new THREE.Vector3( 0, 1, 0 ),
            new THREE.Vector3( 0, 0, 1 )
        ];
        const position = geometry.attributes.position;
        const centers = new Float32Array( position.count * 3 );

        for ( let i = 0, l = position.count; i < l; i ++ ) {

            vectors[ i % 3 ].toArray( centers, i * 3 );

        }
        geometry.addAttribute( 'center', new THREE.BufferAttribute( centers, 3 ) );
    }

    animate = () => {
        requestAnimationFrame( this.animate );

        for ( let i = 0; i < this.scene.children.length; i ++ ) {

            const object = this.scene.children[ i ];
            object.rotation.x += 0.005;
            object.rotation.y += 0.01;

        }
        this.renderer.render( this.scene, this.camera );
    }
}