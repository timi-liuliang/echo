//https://stackoverflow.com/questions/28068038/how-do-i-import-additional-plugins-for-an-already-imported-library-using-jspm/28074932?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
import * as THREE from "./three";
window.THREE = THREE;
// require("./js/controls/OrbitControls");
export default THREE;