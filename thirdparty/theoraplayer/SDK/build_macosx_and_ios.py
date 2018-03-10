import os, sys, shutil, subprocess
# -------------------------
if len(sys.argv) < 2:
	print "ERROR: script must be called with a version string as argument."
	sys.exit(1)
# -------------------------
root         = "../"
theoraplayer = root + "theoraplayer/theoraplayer.xcodeproj"
workspace    = root + "theoraplayer_workspace.xcodeproj"
ogg          = root + "ogg/macosx/Ogg.xcodeproj"
vorbis       = root + "vorbis/macosx/Vorbis.xcodeproj"
theora       = root + "theora/macosx/Theora.xcodeproj"
tremor       = root + "tremor/Tremor.xcodeproj"
clang_version = subprocess.check_output("clang --version | grep version | awk '{print $4}'", shell=True).strip()
llvm_path    = 'llvm'+clang_version
iphoneos_path        = '/device_armv7_armv7s'
iphonesimulator_path = '/simulator_i386'

version      = sys.argv[1]
path         = "libtheoraplayer_sdk_" + version;

iOS_simulator = subprocess.check_output("xcodebuild -showsdks | grep simulator | awk '{print $6}'", shell=True).strip()
# -------------------------
def clean():
	if os.path.exists(path): shutil.rmtree(path)
	if os.path.exists('build'): shutil.rmtree('build')
	os.system("svn export libtheoraplayer_sdk/ " + path)
	
def build(project, target, configuration, sdk = ""):
	cwd = os.getcwd()
	objpath = cwd + "/build"
	productpath = cwd + "/build/products"
	if sdk != "": sdk = "-sdk " + sdk
	ret = os.system("xcodebuild -project %s -target \"%s\" -configuration %s OBJROOT=\"%s\" SYMROOT=\"%s\" %s DEPLOYMENT_LOCATION=NO" % (project, target, configuration, objpath, productpath, sdk))
	if ret != 0:
		print "ERROR while building target: " + target
		sys.exit(ret)

def buildMac(project, target_base):
	build(project, target_base,            "Release")

def buildMacLib(project, target_base):
	build(project, target_base + " (Mac)", "Release")

def buildiOS(project, target_base, suffix = " (iOS)"):
	build(project, target_base + suffix, "Release")
	build(project, target_base + suffix, "Release", iOS_simulator)

def buildAll(project, target_base):
	buildMac(project, target_base)
	buildMacLib(project, target_base)
	buildiOS(project, target_base)

def buildDeps():
	buildAll(ogg, "ogg")
	buildAll(vorbis, "vorbis")
	buildAll(theora, "theora")
	buildiOS(tremor, "tremor")

def copyDirIfExists(src, dest):
	if os.path.exists(src):
		shutil.copytree(src, dest)

def copyIfExists(src, dest):
	if os.path.exists(src):
		shutil.copyfile(src, dest)

def copyDepsDir(src, dest, ext):
	os.mkdir(dest)
	if ext == ".framework":
		copyDirIfExists(src + "/Ogg.framework", dest + "/Ogg.framework")
		copyDirIfExists(src + "/Vorbis.framework", dest + "/Vorbis.framework")
		copyDirIfExists(src + "/Theora.framework", dest + "/Theora.framework")
	else:
		copyIfExists(src + "/libogg.a", dest + "/libogg.a")
		copyIfExists(src + "/libvorbis.a", dest + "/libvorbis.a")
		copyIfExists(src + "/libtremor.a", dest + "/libtremor.a")
		copyIfExists(src + "/libtheora.a", dest + "/libtheora.a")

def copyDeps():
	deps_path = '/Dependencies'
	copyDepsDir('build/products/Release',                  path + '/lib/macosx_framework-' + llvm_path + deps_path, ".framework")
	copyDepsDir('build/products/Release',                  path + '/lib/macosx_lib-'       + llvm_path + deps_path, ".a")
	copyDepsDir('build/products/Release-iphonesimulator',  path + '/lib/ios-'              + llvm_path + iphonesimulator_path + deps_path, ".a")
	copyDepsDir('build/products/Release-iphoneos',         path + '/lib/ios-'              + llvm_path + iphoneos_path + deps_path, ".a")

def copyHeaders():
	shutil.copytree('../ogg/include/ogg',                   path + '/include/ogg')
	shutil.copytree('../vorbis/include/vorbis',             path + '/include/vorbis')
	shutil.copytree('../theora/include/theora',             path + '/include/theora')
	shutil.copytree('../theoraplayer/include/theoraplayer', path + '/include/theoraplayer')

def rm(path):
	if os.path.exists(path):
		if os.path.isdir(path):
			shutil.rmtree(path)
		else:
			os.remove(path)

def makeTheoraplayerFramework(version):
	framework_path = '/theoraplayer.framework'
	build_path = 'build/products/Release'
	rm(build_path + framework_path)
	
	buildMac(theoraplayer, "theoraplayer (" + version + ")")
	shutil.copytree('build/products/Release' + framework_path, path + '/lib/macosx_framework-' + llvm_path + "/" + version.replace(" ", "_") + framework_path)

def createDirectory(path):
	if not os.path.exists(path):
		os.mkdir(path)

def makeTheoraplayerMacLib(version, lib_path):
	lib_path = '/' + lib_path
	build_path_llvm = 'build/products/Release'
	rm(build_path_llvm + lib_path)
	
	buildMac(theoraplayer, "theoraplayer (Mac " + version + ")")

	createDirectory(path + '/lib/macosx_lib-' + llvm_path   + "/" + version.replace(" ", "_"))

	shutil.copyfile(build_path_llvm + lib_path, path + '/lib/macosx_lib-' + llvm_path + "/" + version.replace(" ", "_") + "/libtheoraplayer.a")

def makeTheoraplayeriOSLib(version):
	lib_path = '/libtheoraplayer.a'
	build_path_sim_llvm = 'build/products/Release-iphonesimulator'
	build_path_dev_llvm = 'build/products/Release-iphoneos'
	rm(build_path_sim_llvm + lib_path)
	rm(build_path_dev_llvm + lib_path)
	
	buildiOS(theoraplayer, "theoraplayer", " (iOS " + version + ")")

	createDirectory(path + '/lib/ios-' + llvm_path   + "/" + iphoneos_path        + "/" + version.replace(" ", "_"))
	createDirectory(path + '/lib/ios-' + llvm_path   + "/" + iphonesimulator_path        + "/" + version.replace(" ", "_"))

	shutil.copyfile(build_path_dev_llvm + lib_path, path + '/lib/ios-' + llvm_path  + "/" + iphoneos_path        + "/" + version.replace(" ", "_") + lib_path)
	shutil.copyfile(build_path_sim_llvm + lib_path, path + '/lib/ios-' + llvm_path  + "/" + iphonesimulator_path + "/" + version.replace(" ", "_") + lib_path)

def makeTheoraPlayerLibs():
	makeTheoraplayerFramework("Theora")
	makeTheoraplayerFramework("AVFoundation")
	makeTheoraplayerFramework("Theora AVFoundation")

	makeTheoraplayerMacLib("Theora", "libtheoraplayer.a")
	makeTheoraplayerMacLib("AVFoundation", "libtheoraplayer_avfoundation.a")
	makeTheoraplayerMacLib("Theora AVFoundation", "libtheoraplayer_theora_avfoundation.a")

	makeTheoraplayeriOSLib("Theora")
	makeTheoraplayeriOSLib("AVFoundation")
	makeTheoraplayeriOSLib("Theora AVFoundation")
# -------------------------
clean()
copyHeaders()
buildDeps()
copyDeps()
makeTheoraPlayerLibs()

print "---------------------------"
print "Successfully compiled Theora Playback Library SDK: version " + version + " !"
