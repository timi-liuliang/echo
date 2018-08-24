import os
import subprocess
import argparse
import shutil

# root directory
root_dir = os.getcwd()

# config res
def copy_res() :
    # dirs
    src_dir = root_dir + '/../../Resource/'
    des_debug_dir = root_dir + '/../../Bin/Windows/Debug/res/'
    des_release_dir = root_dir + '/../../Bin/Windows/Release/res/'

    # copy res
    shutil.rmtree( des_debug_dir, True)
    shutil.copytree( src_dir, des_debug_dir)
    print('copy resource from [' + src_dir + '] to [' + des_debug_dir + ']')

    shutil.rmtree( des_release_dir, True)
    shutil.copytree( src_dir, des_release_dir)
    print('copy resource from [' + src_dir + '] to [' + des_release_dir + ']')

    return;

# cmake vs project
def cmake_project(version, platform) :
    vsconfig = " -G\"Visual Studio 15 2017 Win64\" ../"
    solution_dir = root_dir + "/../../../solution/"

	# create dir
    shutil.rmtree( solution_dir, True)
    os.makedirs( solution_dir)

    os.chdir(solution_dir)

    # cmake the project
    subprocess.call("cmake" + vsconfig, shell=True)

    # copy resource
    # copy_res()

    # open direcotry
    os.system( "start " + solution_dir)

    return

# compile vs project debug
def compile_debug() :
    # change working directory
    os.chdir( root_dir + "/../../Build/Win32")

    # Compile
    vs_env = os.environ.get('VS140COMNTOOLS') + "../IDE/devenv.com"
    subprocess.call( vs_env + " LordEngine.sln /Build \"Debug|Win32\"");
    return

# compile vs project release
def compile_release() :
    # change working directory
    os.chdir( root_dir + "/../../build/editor/echo")

    # Compile
    vs_env = os.environ.get('VS140COMNTOOLS') + "../IDE/devenv.com"
    subprocess.call( vs_env + " echo.vcxproj /Build \"Release|Win32\"");
	#subprocess.call( vs_env + " echo.vcxproj /Build \"Release|Win32\"");

    return

# function parse args
def run_parse_args() :
    # create an ArgumentParser object
    parser = argparse.ArgumentParser(description='build LordEngine project on Windows...')

    # add arguments
    #parser.add_argument('-build', help='[debug|release]', choices=['debug', 'release'])
    parser.add_argument('-make', help='build make', default='')
    parser.add_argument('-platform', help='build platform', default='x86', choices=['x86', 'x64'])
    parser.add_argument('-build', help='build type', default='', choices=['debug', 'release'])
    parser.add_argument('-version', help='build version', default='#1')
    parser.add_argument('-nsis', help='nsis build', choices=['echo'])
    parser.add_argument('-lightmass', help='build lightmass', default='win32', choices=['win32', 'x64'])

    # parse instruction
    args = parser.parse_args()

    if args.make=='cmake' :
        cmake_project(args.version, args.platform)

    if args.nsis=='echo':
        release_echo(args.version)

    if args.build=='debug' :
        compile_debug()
    elif args.build=='release' :
        compile_release()

    # end of the function parse_args
    return

def release_echo(version):
	 # dirs
    src_dir = root_dir + '/../../'
    des_dir = root_dir + '/../nsis/echo/'
    nsis_dir= root_dir + '/../nsis/'

    # remove des dir
    shutil.rmtree( des_dir, True)

    # copy bin release
    shutil.copytree( src_dir + 'bin/Release', des_dir)
    print('copy resource from [' + src_dir + 'Bin/Release' + '] to [' + des_dir + ']')

    # copy Dependencies
    #shutil.copytree( src_dir + 'Dependencies', des_dir+'Dependencies', ignore=shutil.ignore_patterns('*Qt*', "*QLibrary*"))
    #print('copy resource from [' + src_dir + 'Dependencies' + '] to [' + des_dir+'Dependencies' + ']')

    # copy lib
    #shutil.copytree( src_dir + 'Lib', des_dir+'Lib')
    #print('copy resource from [' + src_dir + 'Lib' + '] to [' + des_dir+'Lib' + ']')

    # copy Src
    #shutil.copytree( src_dir + 'Src', des_dir+'Src', ignore=shutil.ignore_patterns('*.cpp', '*.txt', '*.7z', '*.rar', '*Tools*', '*.pkg'))
    #print('copy resource from [' + src_dir + 'Src' + '] to [' + des_dir+'Src' + ']')

	# generate installer
    astudio_version_name = 'echo-setup-' + version + '.exe'
    os.chdir( nsis_dir)
    subprocess.call('makensis.exe echo.nsi')
    os.rename('echo-setup.exe', astudio_version_name)
    #shutil.move(astudio_version_name, astudio_des_dir+astudio_version_name)

    return

if __name__ == '__main__' :
    args = run_parse_args()
